#ifndef HYPERTRIE_BULKUPDATER_HPP
#define HYPERTRIE_BULKUPDATER_HPP

#include "dice/hypertrie/BulkUpdater_predeclare.hpp"
#include "dice/hypertrie/Hypertrie.hpp"
#include "dice/hypertrie/internal/raw/node_context/BulkUpdaterSettings.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawBulkUpdater.hpp"
#include "dice/hypertrie/internal/raw/node_context/SynchronousRawBulkUpdater.hpp"

namespace dice::hypertrie {
	namespace bulk_updater_detail {
		template<BulkUpdaterMode mode, HypertrieTrait_bool_valued htt_t, ByteAllocator allocator_type, size_t depth, BulkUpdaterSyncness syncness>
		using RawBulkUpdater_tt = std::conditional_t<syncness == BulkUpdaterSyncness::Async,
													 internal::raw::RawHypertrieBulkUpdater<mode, depth, htt_t, allocator_type, hypertrie_max_depth>,
													 internal::raw::SynchronousRawHypertrieBulkUpdater<mode, depth, htt_t, allocator_type, hypertrie_max_depth>>;
	}

	template<BulkUpdaterMode mode, HypertrieTrait_bool_valued htt_t, ByteAllocator allocator_type, BulkUpdaterSyncness syncness>
	class alignas(bulk_updater_detail::RawBulkUpdater_tt<mode, htt_t, allocator_type, hypertrie_max_depth, syncness>) BulkUpdater {
	public:
		using Entry = NonZeroEntry<htt_t>;
		template<size_t depth>
		using RawEntry = internal::raw::SingleEntry<depth, htt_t>;
		using value_type = typename htt_t::value_type;

		using BulkProcessed_callback = ::dice::hypertrie::internal::raw::BulkUpdater_bulk_processed_callback;

	protected:
		template<size_t depth>
		using RawBulkUpdater_t = bulk_updater_detail::RawBulkUpdater_tt<mode, htt_t, allocator_type, depth, syncness>;

		using max_sized_RawBulkUpdater_t = RawBulkUpdater_t<hypertrie_max_depth>;

		/**
		 * Struct with functions of a RawHypertrieBulkUpdater with fixed depth. This translates the parameter "depth of a hypertrie" from runtime (BulkUpdater) to compile time (RawHypertrieBulkUpdater).
		 */
		struct RawMethods {
			/**
			  * Constructs an RawHypertrieBulkUpdater for hypertrie at the memory address voided_bulk_updater. Parameters bulk_size and bulk_processed_callback are passed to the constructor.
			  * @param hypertrie
			  * @param voided_bulk_updater
			  * @param bulk_size
			  * @param bulk_processed_callback
			  */
			void (*const construct)(Hypertrie<htt_t, allocator_type> &hypertrie, void *voided_bulk_updater, uint32_t bulk_size, BulkProcessed_callback bulk_processed_callback);
			/**
			 * Calls the destructor of a RawHypertrieBulkUpdater located at voided_bulk_updater.
			 * @param voided_bulk_updater
			 */
			void (*const destroy)(void *voided_bulk_updater);
			 /**
			  * Adds the RawEntry located at raw_entry to the RawHypertrieBulkUpdater located at voided_bulk_updater.
			  * @param voided_bulk_updater
			  * @param raw_entry
			  */
			void (*const add_raw)(void *voided_bulk_updater, void const *raw_entry);
			/**
			 * Adds entry to the RawHypertrieBulkUpdater located at voided_bulk_updater.
			 * @param voided_bulk_updater
			 * @param entry
			 */
			void (*const add)(void *voided_bulk_updater, Entry const& entry);
			/**
			 * Returns the number of entries which are currently queued to be inserted into/removed from the hypertrie at the RawHypertrieBulkUpdater located at voided_bulk_updater.
			 * @param voided_bulk_updater
			 * @return
			 */
			size_t (*const size)(void const *voided_bulk_updater) noexcept;
			/**
			 * Flushes the queued entries at the RawHypertrieBulkUpdater located at voided_bulk_updater.
			 * @param voided_bulk_updater
			 */
			void (*const flush)(void *voided_bulk_updater);

		private:
			template<size_t depth>
			static RawMethods instance() {
				using RawBulkUpdater_tt = RawBulkUpdater_t<depth>;
				using RawEntry_t = RawEntry<depth>;
				return {
						.construct = [](Hypertrie<htt_t, allocator_type> &hypertrie, void *voided_bulk_updater, uint32_t bulk_size, BulkProcessed_callback bulk_processed_callback) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						std::construct_at(reinterpret_cast<RawBulkUpdater_tt *>(voided_bulk_updater),
										  hypertrie.node_container_,
										  hypertrie.context()->raw_context(),
										  bulk_size,
										  bulk_processed_callback); },
						.destroy = [](void *voided_bulk_updater) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						std::destroy_at(reinterpret_cast<RawBulkUpdater_tt *>(voided_bulk_updater)); },
						.add_raw = [](void *voided_bulk_updater, void const *raw_entry) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						reinterpret_cast<RawBulkUpdater_tt *>(voided_bulk_updater)->add(*reinterpret_cast<RawEntry_t const *>(raw_entry)); },
						.add = [](void *voided_bulk_updater, Entry const &entry) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						reinterpret_cast<RawBulkUpdater_tt *>(voided_bulk_updater)->add(entry); },
						.size = [](void const *voided_bulk_updater) noexcept {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						return reinterpret_cast<RawBulkUpdater_tt const *>(voided_bulk_updater)->size(); },
						.flush = [](void *voided_bulk_updater) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						reinterpret_cast<RawBulkUpdater_tt *>(voided_bulk_updater)->flush(); }};
			};

			using RawMethodsCache = std::array<RawMethods const, hypertrie_max_depth>;

			template<typename std::size_t... IDs>
			static RawMethodsCache get_raw_methods(std::index_sequence<IDs...>) {
				return {RawMethods::instance<IDs + 1>()...};
			}
			static RawMethodsCache const &get_raw_methods() {
				static RawMethodsCache raw_methods{get_raw_methods(std::make_index_sequence<hypertrie_max_depth>{})};
				return raw_methods;
			}

		public:
			/**
			 * Get a populated instance for the given depth.
			 * @param depth depth of the hypertrie into which entries are inserted/removed
			 * @return struct with functionpointers to RawHypertrieBulkUpdater<depth, htt_t, hypertrie_max_depth> member functions.
			 */
			static RawMethods const &instance(size_t depth) {
				return get_raw_methods()[depth - 1];
			}
		};


		std::array<std::byte, sizeof(max_sized_RawBulkUpdater_t)> raw_bulk_updater;
		RawMethods const *raw_methods = nullptr;
		internal::pos_type const depth_;

	public:
		BulkUpdater() = delete;
		BulkUpdater(BulkUpdater const&) = delete;
		BulkUpdater(BulkUpdater &&) = delete;
		BulkUpdater & operator=(BulkUpdater const&) = delete;
		BulkUpdater & operator=(BulkUpdater &&) = delete;

		explicit BulkUpdater(
				Hypertrie<htt_t, allocator_type> &hypertrie,
				uint32_t bulk_size = 1'000'000U,
				BulkProcessed_callback bulk_processed_callback = []([[maybe_unused]] size_t processed_entries,
																  [[maybe_unused]] size_t committed_entries,
																  [[maybe_unused]] size_t hypertrie_size_after) noexcept {})
			: raw_methods(&RawMethods::instance(hypertrie.depth())),
			  depth_(hypertrie.depth()) {
			raw_methods->construct(hypertrie, &raw_bulk_updater, bulk_size, bulk_processed_callback);
		}

		BulkUpdater(Hypertrie<htt_t, allocator_type> const &) = delete;

		~BulkUpdater() {
			if (raw_methods != nullptr)
				raw_methods->destroy(&raw_bulk_updater);
			raw_methods = nullptr;
		}

		template<size_t depth>
		void add(RawEntry<depth> const &entry) {
			if (depth_ == depth) [[likely]]
				raw_methods->add_raw(&raw_bulk_updater, &entry);
			else [[unlikely]]
				throw std::logic_error{"Entry has wrong depth."};
		}

		void add(Entry const &entry) {
			raw_methods->add(&raw_bulk_updater, entry);
		}

		[[nodiscard]] size_t size() const noexcept {
			return raw_methods->size(&raw_bulk_updater);
		}

		void flush() {
			raw_methods->flush(&raw_bulk_updater);
		}
	};
}// namespace dice::hypertrie

#endif//HYPERTRIE_BULKUPDATER_HPP
