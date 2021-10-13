#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include "ValidationRawNodeContext_slice.hpp"
#include <Dice/hypertrie/internal/util/fmt_utils.hpp>
#include <EntrySetGenerator.hpp>
#include <Node_test_configs.hpp>
#include <RawEntryGenerator.hpp>


#include <Dice/hypertrie/internal/raw/fmt_Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <Dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

namespace hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawNodeContext") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		TEST_CASE("problematic entries 11") {
			using cfg = tagged_bool_cfg<4>;
			static constexpr size_t depth = cfg::depth;
			using tri = typename cfg::tri;

			using key_part_type = typename tri::key_part_type;
			using value_type = typename tri::value_type;

			utils::RawEntryGenerator<depth, tri> gen{};

			static size_t count = 1;
			// static size_t count = std::pow(2, depth * 0.9);


			gen.setKeyPartMinMax(key_part_type(1), key_part_type(2));
			gen.setValueMinMax(true, true);

			RawHypertrieContext<depth, tri> context((std::allocator<std::byte>()));
			NodeContainer<depth, tri> nodec;

			for (const auto &run : iter::range(1000)) {
				SUBCASE(fmt::format("{}", run).c_str()) {
					gen.wind(run);
					auto entries = gen.entries(count);
					context.insert(nodec, entries);

					boost::hana::for_each(boost::hana::range_c<size_t, 0UL, depth>, [&](auto fixed_depth) {
						for (auto positions : iter::combinations(iter::range(depth), fixed_depth)) {
							static constexpr size_t result_depth = depth - fixed_depth;
							// TODO: add one at the lower and upper end of the range
							for (auto key_parts : iter::combinations_with_replacement(iter::range(key_part_type(1), key_part_type(2 + 1)), fixed_depth)) {
								RawSliceKey<fixed_depth, tri> raw_slice;
								for (auto [i, pos_and_key_part] : iter::enumerate(iter::zip(positions, key_parts))) {
									auto [pos, key_part] = pos_and_key_part;
									raw_slice[i].pos = pos;
									raw_slice[i].key_part = key_part;
								}
								auto expected_entries_v = slice_entries(entries, raw_slice);
								std::unordered_map<RawKey<result_depth, tri>, value_type, Dice::hash::DiceHashMartinus<RawKey<result_depth, tri>>> expected_entries;
								for (const auto expected_entry : expected_entries_v)
									expected_entries[expected_entry.key()] = expected_entry.value();

								for (const auto &entry : utils::SingleEntryGenerator<result_depth, tri>()) {
									auto slice = context.template slice(nodec, raw_slice);
									if constexpr (fixed_depth == depth) {
										if (expected_entries.contains(entry.key()))
											CHECK(slice == expected_entries[entry.key()]);
										else
											CHECK(slice == value_type{});
									} else {
										if (slice.uses_tri_alloc()) {
											auto slice_instance = slice.get_with_tri_alloc();
											if (expected_entries.contains(entry.key())) {
												CHECK(not slice.empty());
												CHECK(context.template get(slice_instance, entry.key()) == expected_entries[entry.key()]);
											} else {
												if (slice.empty()) {
													CHECK_MESSAGE(true, "is empty");
												} else {
													CHECK(context.template get(slice_instance, entry.key()) == value_type{});
												}
											}
										} else {
											auto slice_instance = slice.get_with_stl_alloc();
											assert(expected_entries.size() == 1);
											CHECK_MESSAGE(not slice.empty(), "If a node is not allocated with the allocator from the Trait that means that it is an slice of a SingleEntryNode. -> only one entry. ");
											if (expected_entries.contains(entry.key())) {
												CHECK(context.template get(slice_instance, entry.key()) == expected_entries[entry.key()]);
											} else {
												CHECK(context.template get(slice_instance, entry.key()) == value_type{});
											}
											assert(slice.is_managed() == false);

											if (not slice.is_managed())
												delete slice_instance.node_ptr();
										}
									}
								}
							}
						}
					});
				}
			}
		}
	}
};// namespace hypertrie::tests::core::node