#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <utils/EntrySetGenerator.hpp>
#include <utils/Node_test_configs.hpp>
#include <utils/RawEntryGenerator.hpp>
#include <utils/ValidationRawNodeContext_slice.hpp>


#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/fmt_RawKey.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

#include <dice/template-library/for.hpp>

namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawNodeContext") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		// TODO: check handling all all positions fixed
		// TODO: add handling of no positions fixed
		// TODO: test for tri_with_stl_alloc for SingleEntryNodes

		template<size_t depth, HypertrieTrait htt_t,
				 size_t no_key_parts,
				 size_t no_entries>
		void slice(size_t max = std::numeric_limits<size_t>::max());

		TEST_CASE_TEMPLATE("slice depth 1", T,
						   bool_cfg<1>,
						   tagged_bool_cfg<1>,
						   long_cfg<1>,
						   double_cfg<1>) {
			constexpr size_t no_key_parts = 3;
			SUBCASE("{}"_format(typename T::htt_t{}).c_str()) {
				dice::template_library::for_range<0UL, 3UL>(
						[&](auto no_entries) {
							SUBCASE(fmt::format("#entries: {}", no_entries).c_str()) {
								slice<T::depth, typename T::htt_t, no_key_parts, no_entries>();
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
			SUBCASE("{}"_format(typename T::htt_t{}).c_str()) {
				dice::template_library::for_range<0UL, 4UL>(
						[&](auto no_entries) {
							SUBCASE(fmt::format("#entries: {}", no_entries).c_str()) {
								slice<T::depth, typename T::htt_t, no_key_parts, no_entries>();
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
			SUBCASE("{}"_format(typename T::htt_t{}).c_str()) {
				dice::template_library::for_range<0UL, 3UL>(
						[&](auto no_entries) {
							SUBCASE(fmt::format("#entries: {}", no_entries).c_str()) {
								slice<T::depth, typename T::htt_t, no_key_parts, no_entries>();
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
			SUBCASE("{}"_format(typename T::htt_t{}).c_str()) {
				dice::template_library::for_range<0UL, 3UL>(
						[&](auto no_entries) {
							SUBCASE(fmt::format("#entries: {}", no_entries).c_str()) {
								slice<T::depth, typename T::htt_t, no_key_parts, no_entries>(500UL);
							}
						});
			}
		}

		template<size_t depth, HypertrieTrait htt_t,
				 size_t no_key_parts,
				 size_t no_entries>
		void slice(size_t max) {

			using key_part_type = typename htt_t::key_part_type;
			using value_type = typename htt_t::value_type;

			utils::RawEntryGenerator<depth, htt_t> gen{};

			static constexpr key_part_type min_key_part = 1;

			static constexpr key_part_type max_key_part = 1 + no_key_parts;

			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance


			gen.setKeyPartMinMax(key_part_type(1), key_part_type(2));
			gen.setValueMinMax(true, true);

			RawHypertrieContext<depth, htt_t, allocator_type> context((std::allocator<std::byte>()));
			NodeContainer<depth, htt_t, allocator_type> nodec;

			utils::EntrySetGenerator<depth, no_entries, htt_t, max_key_part, min_key_part> outer_generator{};
			for (const auto &entries : outer_generator) {
				SUBCASE(fmt::format("entries: {}", fmt::join(entries, ", ")).c_str()) {
					context.insert(nodec, entries);

					dice::template_library::for_range<0UL, depth>(
							[&](/** the fixed depth of slices */ auto fixed_depth) {
								for (/** the positions where the raw_slice_key has <div>key_part</div> */ auto positions : iter::combinations(iter::range(depth), fixed_depth)) {
									static constexpr size_t result_depth = depth - fixed_depth;
									for (auto key_parts : iter::combinations_with_replacement(iter::range(min_key_part, max_key_part + 1), fixed_depth)) {
										RawSliceKey<fixed_depth, htt_t> raw_slice_key;
										for (const auto [i, pos_and_key_part] : iter::enumerate(iter::zip(positions, key_parts))) {
											const auto &[pos, key_part] = pos_and_key_part;
											raw_slice_key[i].pos = pos;
											raw_slice_key[i].key_part = key_part;
										}
										fmt::print("# SliceKey: {}\n", raw_slice_key);

										auto expected_entries_v = slice_entries(entries, raw_slice_key);
										std::unordered_map<RawKey<result_depth, htt_t>, value_type, dice::hash::DiceHashMartinus<RawKey<result_depth, htt_t>>> expected_entries;
										for (const auto expected_entry : expected_entries_v)
											expected_entries[expected_entry.key()] = expected_entry.value();

										for (const auto &entry : utils::SingleEntryGenerator<result_depth, htt_t, max_key_part, 0>()) {
											fmt::print("{} -> {}\n", entry.key(),
													   (expected_entries.contains(entry.key())) ? expected_entries[entry.key()] : value_type(0));
											auto slice = context.template slice(nodec, raw_slice_key);
											if constexpr (fixed_depth == depth) {
												if (expected_entries.contains(entry.key()))
													CHECK(slice == expected_entries[entry.key()]);
												else
													CHECK(slice == value_type{});
											} else {
												if (slice.uses_provided_alloc()) {
													NodeContainer<result_depth, htt_t, allocator_type> slice_instance =
															(slice.holds_fn())
																	? NodeContainer<result_depth, htt_t, allocator_type>(slice.get_fn())
																	: NodeContainer<result_depth, htt_t, allocator_type>(slice.get_sen());
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
													auto slice_instance = slice.get_stl_alloc_sen();
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
};// namespace dice::hypertrie::tests::core::node