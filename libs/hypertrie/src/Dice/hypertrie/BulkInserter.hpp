#ifndef HYPERTRIE_BULKINSERTER_HPP
#define HYPERTRIE_BULKINSERTER_HPP

#include "Dice/hypertrie/BulkInserter_predeclare.hpp"
#include "Dice/hypertrie/Hypertrie.hpp"
#include "Dice/hypertrie/internal/raw/node_context/RawBulkInserter.hpp"

namespace Dice::hypertrie {

	template<HypertrieTrait tr_t>
	class BulkInserter {
	public:
		using tr = tr_t;
		using tri = internal::raw::Hypertrie_core_t<tr>;
		using Entry = NonZeroEntry<tr>;
		template<size_t depth>
		using RawEntry = internal::raw::SingleEntry<depth, tri>;
		using value_type = typename tr::value_type;

		using BulkInserted_callback = ::Dice::hypertrie::internal::raw::BulkInserter_bulk_loaded_callback;

	protected:
		using max_sized_RawBulkInserter_t = internal::raw::RawHypertrieBulkInserter<hypertrie_max_depth, tri, hypertrie_max_depth>;

		template<size_t depth>
		using RawBulkInserter_t = internal::raw::RawHypertrieBulkInserter<depth, tri, hypertrie_max_depth>;

		struct RawMethods {
			void (*construct)(Hypertrie<tr> &, void *, size_t, BulkInserted_callback) = nullptr;
			void (*destroy)(void *) = nullptr;
			void (*add_raw)(void *, void const *) = nullptr;
			void (*add)(void *, Entry const &) = nullptr;
			size_t (*size)(void const *) noexcept = nullptr;
		};

		template<size_t depth>
		inline static RawMethods generate_raw_methods() noexcept {
			using namespace ::Dice::hypertrie::internal::raw;

			using RawBulkInserter_tt = RawBulkInserter_t<depth>;
			using RawEntry_t = RawEntry<depth>;
			return RawMethods{
					.construct =
							[](Hypertrie<tr> &hypertrie, void *raw_bulk_inserter, size_t bulk_size, BulkInserted_callback bulk_inserted_callback) {
								std::construct_at(reinterpret_cast<RawBulkInserter_tt *>(raw_bulk_inserter),
												  hypertrie.template node_container<depth>(),
												  hypertrie.context()->raw_context(),
												  bulk_size,
												  bulk_inserted_callback);
							},
					.destroy =
							[](void *raw_bulk_inserter) {
								std::destroy_at(reinterpret_cast<RawBulkInserter_tt *>(raw_bulk_inserter));
							},
					.add_raw =
							[](void *raw_bulk_inserter, void const *raw_entry) {
								reinterpret_cast<RawBulkInserter_tt *>(raw_bulk_inserter)->add(*reinterpret_cast<RawEntry_t const *>(raw_entry));
							},
					.add =
							[](void *raw_bulk_inserter, Entry const &entry) {
								reinterpret_cast<RawBulkInserter_tt *>(raw_bulk_inserter)->add(entry);
							},
					.size =
							[](void const *raw_bulk_inserter) noexcept -> size_t {
						return reinterpret_cast<RawBulkInserter_tt const *>(raw_bulk_inserter)->size();
					}};
		}

		using RawMethosCache = std::vector<RawMethods>;
		inline static const RawMethosCache raw_method_cache = []() noexcept {
			using namespace ::Dice::hypertrie::internal::raw;
			using namespace ::Dice::hypertrie::internal::util;
			RawMethosCache raw_methods;
			raw_methods.resize(hypertrie_max_depth);
			for (size_t depth : iter::range(1UL, hypertrie_max_depth + 1)) {
				raw_methods[depth - 1] =
						switch_cases<1UL, hypertrie_max_depth + 1>(
								depth,
								[&](auto depth_arg) -> RawMethods {//
									return generate_raw_methods<depth_arg>();
								},
								[&]() -> RawMethods { assert(false); __builtin_unreachable(); });
			}
			return raw_methods;
		}();

		static RawMethods const &
		getRawMethods(size_t depth) noexcept {
			return raw_method_cache[depth - 1];
		};

		RawMethods const *raw_methods = nullptr;
		std::array<std::byte, sizeof(max_sized_RawBulkInserter_t)> raw_bulk_inserter;
		internal::pos_type depth_;

	public:
		BulkInserter() = delete;
		explicit BulkInserter(
				Hypertrie<tr> &hypertrie,
				size_t bulk_size = 1'000'000,
				BulkInserted_callback bulk_inserted_callback = []([[maybe_unused]] size_t processed_entries,
																  [[maybe_unused]] size_t inserted_entries,
																  [[maybe_unused]] size_t hypertrie_size_after) noexcept {})
			: raw_methods(&getRawMethods(hypertrie.depth())),
			  depth_(hypertrie.depth()) {
			raw_methods->construct(hypertrie, &raw_bulk_inserter, bulk_size, bulk_inserted_callback);
		}

		BulkInserter(Hypertrie<tr> const &) = delete;

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
	};
}// namespace Dice::hypertrie


#endif//HYPERTRIE_BULKINSERTER_HPP
