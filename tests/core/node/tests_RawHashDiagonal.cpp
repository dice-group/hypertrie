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
		void test_fn_diagonal() {
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
							boost::hana::range_c<size_t, 1UL, depth + 1>,
							[&](/** the fixed depth of the diagonals */ auto fixed_depth) {
								for (/** the positions for the diagonal */ auto const &positions : iter::combinations(iter::range(depth), fixed_depth)) {
									static constexpr size_t result_depth = depth - fixed_depth;

									RawKeyPositions<depth> diag_poss(positions);
									size_t count = 0;
									ValidationRawHashDiagonal<fixed_depth, depth, tri> validation_raw_hash_diagonal(entries, diag_poss);
									for (const auto &[key_part, slice] : RawHashDiagonal<fixed_depth, depth, FullNode, tri, depth>(nodec.template specific<FullNode>(), diag_poss, context)) {
										validation_raw_hash_diagonal.has_diagonal(key_part);
										if constexpr (result_depth != 0) {
											if (slice.uses_tri_alloc()) {
												auto slice_instance = slice.get_with_tri_alloc();
												CHECK(slice_instance.raw_identifier() == validation_raw_hash_diagonal.raw_identifier(key_part));
												for (const auto &entry : validation_raw_hash_diagonal.entries(key_part))
													CHECK(context.get(slice_instance, entry.key()) == entry.value());

												//											CHECK(context.size(nodec) == validation_raw_hash_diagonal.entries(key_part).size());
											} else {
												auto slice_instance = slice.get_with_stl_alloc();
												CHECK(validation_raw_hash_diagonal.entries(key_part).size() == 1);

												CHECK(slice_instance.raw_identifier() == validation_raw_hash_diagonal.raw_identifier(key_part));

												for (const auto &entry : validation_raw_hash_diagonal.entries(key_part))
													CHECK(context.get(slice_instance, entry.key()) == entry.value());
											}
										}
										++count;
									}
									CHECK(validation_raw_hash_diagonal.size() == count);
								}
							});
				}
			}
		}

		TEST_CASE("Diagonal on FullNode") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			constexpr auto count = 2;
			constexpr auto no_key_parts = 2;

			test_fn_diagonal<depth, tri, no_key_parts, count>();
		};
	};
};// namespace hypertrie::tests::core::node