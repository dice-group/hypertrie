#ifndef HYPERTRIE_BULKINSERTER_HPP
#define HYPERTRIE_BULKINSERTER_HPP

#include "dice/hypertrie/BulkInserter_predeclare.hpp"
#include "dice/hypertrie/Hypertrie.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawBulkInserter.hpp"
#include "dice/hypertrie/internal/raw/node_context/SynchronousRawBulkInserter.hpp"

namespace dice::hypertrie {
	namespace bulk_inserter_detail {
		template<HypertrieTrait_bool_valued htt_t, ByteAllocator allocator_type, size_t depth, bool async>
		using RawBulkInserter_tt = std::conditional_t<async,
													  internal::raw::RawHypertrieBulkInserter<depth, htt_t, allocator_type, hypertrie_max_depth>,
													  internal::raw::SynchronousRawHypertrieBulkInserter<depth, htt_t, allocator_type, hypertrie_max_depth>>;
	}

	template<HypertrieTrait_bool_valued htt_t, ByteAllocator allocator_type, bool async>
	class alignas(bulk_inserter_detail::RawBulkInserter_tt<htt_t, allocator_type, hypertrie_max_depth, async>) BulkInserter {
	public:
		using Entry = NonZeroEntry<htt_t>;
		template<size_t depth>
		using RawEntry = internal::raw::SingleEntry<depth, htt_t>;
		using value_type = typename htt_t::value_type;

		using BulkInserted_callback = ::dice::hypertrie::internal::raw::BulkInserter_bulk_loaded_callback;

	protected:
		template<size_t depth>
		using RawBulkInserter_t = bulk_inserter_detail::RawBulkInserter_tt<htt_t, allocator_type, depth, async>;

		using max_sized_RawBulkInserter_t = RawBulkInserter_t<hypertrie_max_depth>;

		/**
		 * Struct with functions of a RawHypertrieBulkInserter with fixed depth. This translates the parameter "depth of a hypertrie" from runtime (BulkInserter) to compile time (RawHypertrieBulkInserter).
		 */
		struct RawMethods {
			/**
			  * Constructs an RawHypertrieBulkInserter for hypertrie at the memory address voided_bulk_inserter. Parameters bulk_size and bulk_inserted_callback are passed to the constructor.
			  * @param hypertrie
			  * @param voided_bulk_inserter
			  * @param bulk_size
			  * @param bulk_inserted_callback
			  */
			void (*const construct)(Hypertrie<htt_t, allocator_type> &hypertrie, void *voided_bulk_inserter, uint32_t bulk_size, BulkInserted_callback bulk_inserted_callback);
			/**
			 * Calls the destructor of a RawHypertrieBulkInserter located at voided_bulk_inserter.
			 * @param voided_bulk_inserter
			 */
			void (*const destroy)(void *voided_bulk_inserter);
			 /**
			  * Adds the RawEntry located at raw_entry to the RawHypertrieBulkInserter located at voided_bulk_inserter.
			  * @param voided_bulk_inserter
			  * @param raw_entry
			  */
			void (*const add_raw)(void *voided_bulk_inserter, void const *raw_entry);
			/**
			 * Adds entry to the RawHypertrieBulkInserter located at voided_bulk_inserter.
			 * @param voided_bulk_inserter
			 * @param entry
			 */
			void (*const add)(void *voided_bulk_inserter, Entry const&entry);
			/**
			 * Returns the number of entries which are currently queued to be inserted into the hypertrie at the RawHypertrieBulkInserter located at voided_bulk_inserter.
			 * @param voided_bulk_inserter
			 * @return
			 */
			size_t (*const size)(void const *voided_bulk_inserter) noexcept;
			/**
			 * Flushes the queued entries at the RawHypertrieBulkInserter located at voided_bulk_inserter.
			 * @param voided_bulk_inserter
			 */
			void (*const flush)(void *voided_bulk_inserter);

		private:
			template<size_t depth>
			static RawMethods instance() {
				using RawBulkInserter_tt = RawBulkInserter_t<depth>;
				using RawEntry_t = RawEntry<depth>;
				return {
						.construct = [](Hypertrie<htt_t, allocator_type> &hypertrie, void *voided_bulk_inserter, uint32_t bulk_size, BulkInserted_callback bulk_inserted_callback) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						std::construct_at(reinterpret_cast<RawBulkInserter_tt *>(voided_bulk_inserter),
										  hypertrie.node_container_,
										  hypertrie.context()->raw_context(),
										  bulk_size,
										  bulk_inserted_callback); },
						.destroy = [](void *voided_bulk_inserter) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						std::destroy_at(reinterpret_cast<RawBulkInserter_tt *>(voided_bulk_inserter)); },
						.add_raw = [](void *voided_bulk_inserter, void const *raw_entry) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						reinterpret_cast<RawBulkInserter_tt *>(voided_bulk_inserter)->add(*reinterpret_cast<RawEntry_t const *>(raw_entry)); },
						.add = [](void *voided_bulk_inserter, Entry const &entry) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						reinterpret_cast<RawBulkInserter_tt *>(voided_bulk_inserter)->add(entry); },
						.size = [](void const *voided_bulk_inserter) noexcept {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						return reinterpret_cast<RawBulkInserter_tt const *>(voided_bulk_inserter)->size(); },
						.flush = [](void *voided_bulk_inserter) {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						reinterpret_cast<RawBulkInserter_tt *>(voided_bulk_inserter)->flush(); }};
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
			 * @param depth depth of the hypertrie into which entries are inserted
			 * @return struct with functionpointers to RawHypertrieBulkInserter<depth, htt_t, hypertrie_max_depth> member functions.
			 */
			static RawMethods const &instance(size_t depth) {
				return get_raw_methods()[depth - 1];
			}
		};


		std::array<std::byte, sizeof(max_sized_RawBulkInserter_t)> raw_bulk_inserter;
		RawMethods const *raw_methods = nullptr;
		internal::pos_type const depth_;

	public:
		BulkInserter() = delete;
		BulkInserter(BulkInserter const&) = delete;
		BulkInserter(BulkInserter &&) = delete;
		BulkInserter& operator=(BulkInserter const&) = delete;
		BulkInserter& operator=(BulkInserter &&) = delete;

		explicit BulkInserter(
				Hypertrie<htt_t, allocator_type> &hypertrie,
				uint32_t bulk_size = 1'000'000U,
				BulkInserted_callback bulk_inserted_callback = []([[maybe_unused]] size_t processed_entries,
																  [[maybe_unused]] size_t inserted_entries,
																  [[maybe_unused]] size_t hypertrie_size_after) noexcept {})
			: raw_methods(&RawMethods::instance(hypertrie.depth())),
			  depth_(hypertrie.depth()) {
			raw_methods->construct(hypertrie, &raw_bulk_inserter, bulk_size, bulk_inserted_callback);
		}

		BulkInserter(Hypertrie<htt_t, allocator_type> const &) = delete;

		~BulkInserter() {
			if (raw_methods != nullptr)
				raw_methods->destroy(&raw_bulk_inserter);
			raw_methods = nullptr;
		}

		template<size_t depth>
		void add(RawEntry<depth> const &entry) {
			if (depth_ == depth) [[likely]]
				raw_methods->add_raw(&raw_bulk_inserter, &entry);
			else [[unlikely]]
				throw std::logic_error{"Entry has wrong depth."};
		}

		void add(Entry const &entry) {
			raw_methods->add(&raw_bulk_inserter, entry);
		}

		[[nodiscard]] size_t size() const noexcept {
			return raw_methods->size(&raw_bulk_inserter);
		}

		void flush() {
			raw_methods->flush(&raw_bulk_inserter);
		}
	};
}// namespace dice::hypertrie


#endif//HYPERTRIE_BULKINSERTER_HPP
