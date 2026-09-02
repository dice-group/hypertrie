#ifndef HYPERTRIE_BULKUPDATER_HPP
#define HYPERTRIE_BULKUPDATER_HPP

#include "dice/hypertrie/BulkUpdater_predeclare.hpp"
#include "dice/hypertrie/Hypertrie.hpp"
#include "dice/hypertrie/internal/raw/node_context/BulkUpdaterSettings.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawBulkUpdater.hpp"
#include "dice/hypertrie/internal/raw/node_context/SynchronousRawBulkUpdater.hpp"

namespace dice::hypertrie {
	namespace bulk_updater_detail {
		template<BulkUpdaterMode mode, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t depth, BulkUpdaterSyncness syncness>
		using RawBulkUpdater_tt = std::conditional_t<syncness == BulkUpdaterSyncness::Async,
													 internal::raw::RawHypertrieBulkUpdater<mode, depth, htt_t, allocator_type, hypertrie_max_depth>,
													 internal::raw::SynchronousRawHypertrieBulkUpdater<mode, depth, htt_t, allocator_type, hypertrie_max_depth>>;
	}// namespace bulk_updater_detail

	/**
	 * @note This does not support changing values yet.
	 * 		- On insert will ignore entries if their key already exists in the hypertrie
	 *		- On removal will ignore entries if the key's associated value does not exactly match the value in the hypertrie
	 */
	template<BulkUpdaterMode mode, HypertrieTrait htt_t, ByteAllocator allocator_type, BulkUpdaterSyncness syncness>
	class BulkUpdater {
	public:
		template<size_t depth>
		using RawEntry = std::conditional_t<mode == BulkUpdaterMode::Insert, internal::raw::SingleEntry<depth, htt_t>, internal::raw::RawKey<depth, htt_t>>;

		using Entry = std::conditional_t<mode == BulkUpdaterMode::Insert, NonZeroEntry<htt_t>, Key<htt_t>>;

		using BulkProcessed_callback = ::dice::hypertrie::internal::raw::BulkUpdater_bulk_processed_callback;

	protected:
		template<size_t depth>
		using RawBulkUpdater_t = bulk_updater_detail::RawBulkUpdater_tt<mode, htt_t, allocator_type, depth, syncness>;

		using max_sized_RawBulkUpdater_t = RawBulkUpdater_t<hypertrie_max_depth>;

		/**
		 * Struct with functions of a RawHypertrieBulkUpdater with fixed depth. This translates the parameter "depth of a hypertrie" from runtime (BulkUpdater) to compile time (RawHypertrieBulkUpdater).
		 */
		struct VTable {
			/**
			  * Constructs an RawHypertrieBulkUpdater for hypertrie at the memory address voided_bulk_updater. Parameters bulk_size and bulk_processed_callback are passed to the constructor.
			  * @param hypertrie
			  * @param voided_bulk_updater
			  * @param bulk_size
			  * @param bulk_processed_callback
			  */
			void (*construct)(void *voided_bulk_updater, Hypertrie<htt_t, allocator_type> &hypertrie, uint32_t bulk_size, BulkProcessed_callback bulk_processed_callback);
			/**
			 * Calls the destructor of a RawHypertrieBulkUpdater located at voided_bulk_updater.
			 * @param voided_bulk_updater
			 */
			void (*destroy)(void *voided_bulk_updater);
			 /**
			  * Adds the RawEntry located at raw_entry to the RawHypertrieBulkUpdater located at voided_bulk_updater.
			  * @param voided_bulk_updater
			  * @param raw_entry
			  */
			void (*add_raw)(void *voided_bulk_updater, void const *raw_entry);
			/**
			 * Adds entry to the RawHypertrieBulkUpdater located at voided_bulk_updater.
			 * @param voided_bulk_updater
			 * @param entry
			 */
			void (*add)(void *voided_bulk_updater, Entry const& entry);
			/**
			 * Returns the number of entries which are currently queued to be inserted into/removed from the hypertrie at the RawHypertrieBulkUpdater located at voided_bulk_updater.
			 * @param voided_bulk_updater
			 * @return
			 */
			size_t (*size)(void const *voided_bulk_updater) noexcept;
			/**
			 * Flushes the queued entries at the RawHypertrieBulkUpdater located at voided_bulk_updater.
			 * @param voided_bulk_updater
			 */
			void (*flush)(void *voided_bulk_updater);

			template<size_t depth>
			static consteval VTable make() {
				using RawBulkUpdater_tt = RawBulkUpdater_t<depth>;
				return VTable{
						.construct = [](void *location, Hypertrie<htt_t, allocator_type> &hypertrie, uint32_t bulk_size, BulkProcessed_callback bulk_processed_callback) {
							new (location) RawBulkUpdater_tt{static_cast<internal::raw::NodePtr<depth, htt_t, allocator_type> &>(hypertrie.node_ptr_),
															 hypertrie.context_->raw_context(),
															 bulk_size,
															 bulk_processed_callback};
						},
						.destroy = [](void *voided_bulk_updater) {
							reinterpret_cast<RawBulkUpdater_tt *>(voided_bulk_updater)->~RawBulkUpdater_tt();
						},
						.add_raw = [](void *voided_bulk_updater, void const *raw_entry) {
							reinterpret_cast<RawBulkUpdater_tt *>(voided_bulk_updater)->add(*reinterpret_cast<RawEntry<depth> const *>(raw_entry));
						},
						.add = [](void *voided_bulk_updater, Entry const &entry) {
							reinterpret_cast<RawBulkUpdater_tt *>(voided_bulk_updater)->add(entry);
						},
						.size = [](void const *voided_bulk_updater) noexcept {
							return reinterpret_cast<RawBulkUpdater_tt const *>(voided_bulk_updater)->size();
						},
						.flush = [](void *voided_bulk_updater) {
							reinterpret_cast<RawBulkUpdater_tt *>(voided_bulk_updater)->flush();
						}};
			}
		};

		template<size_t ...ixs>
		static consteval std::array<VTable, hypertrie_max_depth> make_vtables(std::index_sequence<ixs...>) {
			return {VTable::template make<ixs + 1>()...};
		}

		static constexpr std::array<VTable, hypertrie_max_depth> vtables_ = make_vtables(std::make_index_sequence<hypertrie_max_depth>{});

		alignas(max_sized_RawBulkUpdater_t) std::byte inner_[sizeof(max_sized_RawBulkUpdater_t)];
		VTable const *vtable_ = nullptr;
		internal::pos_type const depth_;

	public:
		BulkUpdater() = delete;
		BulkUpdater(BulkUpdater const&) = delete;
		BulkUpdater(BulkUpdater &&) = delete;
		BulkUpdater &operator=(BulkUpdater const&) = delete;
		BulkUpdater &operator=(BulkUpdater &&) = delete;

		explicit BulkUpdater(Hypertrie<htt_t, allocator_type> &hypertrie,
				uint32_t bulk_size = 1'000'000U,
				BulkProcessed_callback bulk_processed_callback = []([[maybe_unused]] size_t processed_entries,
																	[[maybe_unused]] size_t committed_entries,
																	[[maybe_unused]] size_t hypertrie_size_after) noexcept {})
			: vtable_{&vtables_[hypertrie.depth() - 1]},
			  depth_{static_cast<internal::pos_type>(hypertrie.depth())} {

			vtable_->construct(inner_, hypertrie, bulk_size, bulk_processed_callback);
		}

		~BulkUpdater() {
			if (vtable_ != nullptr) {
				vtable_->destroy(inner_);
			}
		}

		template<size_t depth> requires (mode == BulkUpdaterMode::Insert)
		void add(internal::raw::SingleEntry<depth, htt_t> const &entry) {
			if (depth_ != depth) [[unlikely]] {
				throw std::logic_error{"RawEntry has wrong depth."};
			}

			vtable_->add_raw(inner_, &entry);
		}

		template<size_t depth> requires (mode == BulkUpdaterMode::Remove)
		void add(internal::raw::RawKey<depth, htt_t> const &entry) {
			if (depth_ != depth) [[unlikely]] {
				throw std::logic_error{"RawEntry has wrong depth."};
			}

			vtable_->add_raw(inner_, &entry);
		}

		void add(Entry const &entry) {
			vtable_->add(inner_, entry);
		}

		[[nodiscard]] size_t size() const noexcept {
			return vtable_->size(inner_);
		}

		void flush() {
			vtable_->flush(inner_);
		}
	};
}// namespace dice::hypertrie

#endif//HYPERTRIE_BULKUPDATER_HPP
