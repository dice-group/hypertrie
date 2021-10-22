#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include "ValidationRawNodeContext_slice.hpp"
#include <EntrySetGenerator.hpp>
#include <Node_test_configs.hpp>
#include <RawEntryGenerator.hpp>


#include <Dice/hypertrie/internal/raw/fmt_Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/fmt_RawKey.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <Dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

namespace Dice::hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawNodeContext") {
		using namespace ::Dice::hypertrie::internal::raw;
		using namespace ::Dice::hypertrie::internal::util;

		// TODO: check handling all all positions fixed
		// TODO: add handling of no positions fixed
		// TODO: test for tri_with_stl_alloc for SingleEntryNodes

		template<size_t depth, HypertrieCoreTrait tri,
				 size_t no_key_parts,
				 size_t no_entries>
		void slice(size_t max = std::numeric_limits<size_t>::max());

		TEST_CASE_TEMPLATE("slice depth 1", T,
						   bool_cfg<1>,
						   tagged_bool_cfg<1>,
						   long_cfg<1>,
						   double_cfg<1>) {
			constexpr size_t no_key_parts = 3;
			SUBCASE("{}"_format(typename T::tri{}).c_str()) {
				boost::hana::for_each(
						boost::hana::range_c<size_t, 0UL, 3UL>,
						[&](auto no_entries) {
							SUBCASE(fmt::format("#entries: {}", no_entries).c_str()) {
								slice<T::depth, typename T::tri, no_key_parts, no_entries>();
							}
						});
			}
		}

		TEST_CASE_TEMPLATE("slice depth 2", T,
						   bool_cfg<2>,
						   tagged_bool_cfg<2>,
						   long_cfg<2>,
						   double_cfg<2>) {
			constexpr size_t no_key_parts = 3;
			SUBCASE("{}"_format(typename T::tri{}).c_str()) {
				boost::hana::for_each(
						boost::hana::range_c<size_t, 0UL, 4UL>,
						[&](auto no_entries) {
							SUBCASE(fmt::format("#entries: {}", no_entries).c_str()) {
								slice<T::depth, typename T::tri, no_key_parts, no_entries>();
							}
						});
			}
		}

		TEST_CASE_TEMPLATE("slice depth 3", T,
						   bool_cfg<3>,
						   tagged_bool_cfg<3>,
						   long_cfg<3>,
						   double_cfg<3>) {
			constexpr size_t no_key_parts = 2;
			SUBCASE("{}"_format(typename T::tri{}).c_str()) {
				boost::hana::for_each(
						boost::hana::range_c<size_t, 0UL, 3UL>,
						[&](auto no_entries) {
							SUBCASE(fmt::format("#entries: {}", no_entries).c_str()) {
								slice<T::depth, typename T::tri, no_key_parts, no_entries>();
							}
						});
			}
		}

		TEST_CASE_TEMPLATE("slice depth 4", T,
						   bool_cfg<4>,
						   tagged_bool_cfg<4>,
						   long_cfg<4>,
						   double_cfg<4>) {
			constexpr size_t no_key_parts = 2;
			SUBCASE("{}"_format(typename T::tri{}).c_str()) {
				boost::hana::for_each(
						boost::hana::range_c<size_t, 0UL, 3UL>,
						[&](auto no_entries) {
							SUBCASE(fmt::format("#entries: {}", no_entries).c_str()) {
								slice<T::depth, typename T::tri, no_key_parts, no_entries>(500UL);
							}
						});
			}
		}

		template<size_t depth, HypertrieCoreTrait tri,
				 size_t no_key_parts,
				 size_t no_entries>
		void slice(size_t max) {

			using key_part_type = typename tri::key_part_type;
			using value_type = typename tri::value_type;

			utils::RawEntryGenerator<depth, tri> gen{};

			static constexpr key_part_type min_key_part = 1;

			static constexpr key_part_type max_key_part = 1 + no_key_parts;


			gen.setKeyPartMinMax(key_part_type(1), key_part_type(2));
			gen.setValueMinMax(true, true);

			RawHypertrieContext<depth, tri> context((std::allocator<std::byte>()));
			NodeContainer<depth, tri> nodec;

			utils::EntrySetGenerator<depth, no_entries, tri, max_key_part, min_key_part> outer_generator{};
			for (const auto &entries : outer_generator) {
				SUBCASE(fmt::format("entries: {}", fmt::join(entries, ", ")).c_str()) {
					context.insert(nodec, entries);

					boost::hana::for_each(
							boost::hana::range_c<size_t, 0UL, depth>,
							[&](/** the fixed depth of slices */ auto fixed_depth) {
								for (/** the positions where the raw_slice_key has <div>key_part</div> */ auto positions : iter::combinations(iter::range(depth), fixed_depth)) {
									static constexpr size_t result_depth = depth - fixed_depth;
									for (auto key_parts : iter::combinations_with_replacement(iter::range(min_key_part, max_key_part + 1), fixed_depth)) {
										RawSliceKey<fixed_depth, tri> raw_slice_key;
										for (const auto [i, pos_and_key_part] : iter::enumerate(iter::zip(positions, key_parts))) {
											const auto &[pos, key_part] = pos_and_key_part;
											raw_slice_key[i].pos = pos;
											raw_slice_key[i].key_part = key_part;
										}
										fmt::print("# SliceKey: {}\n", raw_slice_key);

										auto expected_entries_v = slice_entries(entries, raw_slice_key);
										std::unordered_map<RawKey<result_depth, tri>, value_type, Dice::hash::DiceHashMartinus<RawKey<result_depth, tri>>> expected_entries;
										for (const auto expected_entry : expected_entries_v)
											expected_entries[expected_entry.key()] = expected_entry.value();

										for (const auto &entry : utils::SingleEntryGenerator<result_depth, tri, max_key_part, 0>()) {
											fmt::print("{} -> {}\n", entry.key(),
													   (expected_entries.contains(entry.key())) ? expected_entries[entry.key()] : value_type(0));
											auto slice = context.template slice(nodec, raw_slice_key);
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
				if (--max == 0)
					break;
			}
		}
	}
};// namespace Dice::hypertrie::tests::core::node