#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include "ValidationRawHashDiagonal.hpp"
#include <Node_test_configs.hpp>
#include <RawEntryGenerator.hpp>


#include <Dice/hypertrie/internal/raw/iteration/RawHashDiagonal.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <Dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>
#include <EntrySetGenerator.hpp>

namespace hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawHashDiagonal") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		template<size_t depth, HypertrieCoreTrait tri,
				 size_t no_key_parts,
				 size_t no_entries>
		void test_diagonal(size_t max_entry_sets = 10'000) {
			SUBCASE(fmt::format("no_entries: {}", no_entries).c_str()) {

				using key_part_type = typename tri::key_part_type;
				//			using value_type = typename tri::value_type;

				utils::RawEntryGenerator<depth, tri> gen{};

				static constexpr key_part_type min_key_part = 1;

				static constexpr key_part_type max_key_part = no_key_parts;


				gen.setKeyPartMinMax(key_part_type(1), key_part_type(2));
				gen.setValueMinMax(true, true);

				RawHypertrieContext<depth, tri> context((std::allocator<std::byte>()));
				NodeContainer<depth, tri> nodec;

				utils::EntrySetGenerator<depth, no_entries, tri, max_key_part, min_key_part> outer_generator{};
				for (const auto &entries : outer_generator) {
					SUBCASE(fmt::format("entries: {}", fmt::join(entries, ", ")).c_str()) {
						context.insert(nodec, entries);
						boost::hana::for_each(
								boost::hana::range_c<size_t, 1UL, depth + 1>,
								[&](/** the fixed depth of the diagonals */ auto fixed_depth) {
									for (/** the positions for the diagonal */ auto const &positions : iter::combinations(iter::range(depth), fixed_depth)) {
										SUBCASE(fmt::format("diagonal positions: [{}]", fmt::join(positions, ", ")).c_str()) {
											static constexpr size_t result_depth = depth - fixed_depth;

											RawKeyPositions<depth> diag_poss(positions);


											ValidationRawHashDiagonal<fixed_depth, depth, tri> validation_raw_hash_diagonal(entries, diag_poss);

											auto raw_hash_diagonal = [&]() {
												if constexpr (no_entries == 1)
													return RawHashDiagonal<fixed_depth, depth, SingleEntryNode, tri, depth>(nodec.template specific<SingleEntryNode>(), diag_poss);
												else
													return RawHashDiagonal<fixed_depth, depth, FullNode, tri, depth>(nodec.template specific<FullNode>(), diag_poss, context);
											}();

											CHECK_MESSAGE(validation_raw_hash_diagonal.size() <= raw_hash_diagonal.size(), "size estimation must be an upper bound to the actual number of non-zero slices in the diagonal.");

											SUBCASE("check iterator") {

												if (raw_hash_diagonal.empty()) {
													CHECK(validation_raw_hash_diagonal.size() == 0);
													CHECK(raw_hash_diagonal.size() == 0);
												} else {
													size_t count = 0;
													for (const auto &[key_part, slice] : raw_hash_diagonal) {
														fmt::print("key_part: {}\n", key_part);
														CHECK(validation_raw_hash_diagonal.has_diagonal(key_part));
														if constexpr (result_depth != 0) {
															if (slice.uses_tri_alloc()) {
																auto slice_instance = slice.get_with_tri_alloc();
																CHECK(slice_instance.raw_identifier() == validation_raw_hash_diagonal.raw_identifier(key_part));
																for (const auto &entry : validation_raw_hash_diagonal.entries(key_part))
																	CHECK(context.get(slice_instance, entry.key()) == entry.value());

																fmt::print("{}\n", fmt::join(validation_raw_hash_diagonal.entries(key_part), ", "));
																CHECK(context.size(slice_instance) == validation_raw_hash_diagonal.entries(key_part).size());
															} else {
																auto slice_instance = slice.get_with_stl_alloc();
																CHECK(validation_raw_hash_diagonal.entries(key_part).size() == 1);

																CHECK(slice_instance.raw_identifier() == validation_raw_hash_diagonal.raw_identifier(key_part));

																for (const auto &entry : validation_raw_hash_diagonal.entries(key_part))
																	CHECK(context.get(slice_instance, entry.key()) == entry.value());
															}
														} else {
															fmt::print("{}\n", slice);
															CHECK(slice == validation_raw_hash_diagonal.entries(key_part)[0].value());
														}
														++count;
													}

													CHECK(validation_raw_hash_diagonal.size() == count);
												}
											}
											SUBCASE("check element retrieval") {
												if (raw_hash_diagonal.empty()) {
													CHECK(validation_raw_hash_diagonal.size() == 0);
													CHECK(raw_hash_diagonal.size() == 0);
												} else {
													for (const auto &key_part : validation_raw_hash_diagonal.key_parts()) {
														fmt::print("key_part: {}\n", key_part);
														bool found = raw_hash_diagonal.find(key_part);
														CHECK_MESSAGE(found, "make sure the key_part was found");
														auto slice = raw_hash_diagonal.current_value();

														if constexpr (result_depth != 0) {
															if (slice.uses_tri_alloc()) {
																auto slice_instance = slice.get_with_tri_alloc();
																CHECK(slice_instance.raw_identifier() == validation_raw_hash_diagonal.raw_identifier(key_part));
																for (const auto &entry : validation_raw_hash_diagonal.entries(key_part))
																	CHECK(context.get(slice_instance, entry.key()) == entry.value());

																fmt::print("{}\n", fmt::join(validation_raw_hash_diagonal.entries(key_part), ", "));
																CHECK(context.size(slice_instance) == validation_raw_hash_diagonal.entries(key_part).size());
															} else {
																auto slice_instance = slice.get_with_stl_alloc();
																CHECK(validation_raw_hash_diagonal.entries(key_part).size() == 1);

																CHECK(slice_instance.raw_identifier() == validation_raw_hash_diagonal.raw_identifier(key_part));

																for (const auto &entry : validation_raw_hash_diagonal.entries(key_part))
																	CHECK(context.get(slice_instance, entry.key()) == entry.value());
															}
														} else {
															fmt::print("{}\n", slice);
															CHECK(slice == validation_raw_hash_diagonal.entries(key_part)[0].value());
														}
													}
												}
											}
										}
									}
								});
					}
					if (max_entry_sets-- == 0)
						break;
				}
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 1", T,
						   bool_cfg<1>,
						   tagged_bool_cfg<1>,
						   long_cfg<1>,
						   double_cfg<1>) {
			constexpr size_t no_key_parts = 3;

			{
				constexpr size_t no_entries = 1;
				test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
			}

			{
				constexpr size_t no_entries = 2;
				test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
			}

			{
				constexpr size_t no_entries = 3;
				test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 2", T,
						   bool_cfg<2>,
						   tagged_bool_cfg<2>,
						   long_cfg<2>,
						   double_cfg<2>) {
			constexpr size_t no_key_parts = 2;
			{
				constexpr size_t no_entries = 1;
				test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
			}

			{
				constexpr size_t no_entries = 2;
				test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 3", T,
						   bool_cfg<3>,
						   tagged_bool_cfg<3>,
						   long_cfg<3>,
						   double_cfg<3>) {
			{
				constexpr size_t no_key_parts = 3;
				{
					constexpr size_t no_entries = 1;
					test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
				}
			}
			{
				constexpr size_t no_key_parts = 2;
				{
					constexpr size_t no_entries = 2;
					test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
				}
				{
					constexpr size_t no_entries = 3;
					test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
				}
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 4", T,
						   bool_cfg<4>,
						   tagged_bool_cfg<4>,
						   long_cfg<4>,
						   double_cfg<4>) {
			{
				constexpr size_t no_key_parts = 3;
				constexpr size_t no_entries = 1;
				test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
			}

			{
				constexpr size_t no_key_parts = 2;
				constexpr size_t no_entries = 2;
				test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
			}

			{
				constexpr size_t no_key_parts = 2;
				constexpr size_t no_entries = 3;
				test_diagonal<T::depth, typename T::tri, no_key_parts, no_entries>();
			}
		}
	};
};// namespace hypertrie::tests::core::node