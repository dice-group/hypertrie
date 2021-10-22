#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include "ValidationRawNodeContext.hpp"
#include <Dice/hypertrie/internal/util/fmt_utils.hpp>
#include <Node_test_configs.hpp>
#include <RawEntryGenerator.hpp>


#include <Dice/hypertrie/internal/raw/fmt_Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <Dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

#include <EntrySetGenerator.hpp>

namespace Dice::hypertrie::tests::core::node {

	TEST_SUITE("systematic testing of RawNodeContext") {
		using namespace ::Dice::hypertrie::internal::raw;
		using namespace ::Dice::hypertrie::internal::util;

		template<size_t depth, HypertrieCoreTrait tri,
				 size_t no_key_parts,
				 size_t min_no_entries,
				 size_t max_no_entries>
		void write_and_read() {
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;

			using key_part_type = typename tri::key_part_type;

			static constexpr auto max_key_part = key_part_type(no_key_parts);


			SUBCASE("{}"_format(tri{}).c_str()) {
				SUBCASE("hypertrie depth = {}"_format(depth).c_str()) {
					boost::hana::for_each(boost::hana::range_c<size_t, min_no_entries, max_no_entries + 1>, [&](auto no_entries_0) {
						boost::hana::for_each(boost::hana::range_c<size_t, min_no_entries, max_no_entries + 1>, [&](auto no_entries_1) {
							SUBCASE("first {} entries, then {} entries"_format(no_entries_0, no_entries_1).c_str()) {


								utils::EntrySetGenerator<depth, no_entries_0, tri, max_key_part> outer_generator{};
								for (const auto &entries_0 : outer_generator) {
									SUBCASE("first_entries: {}"_format(fmt::join(entries_0, " | ")).c_str()) {
										RawHypertrieContext<depth, tri> context{std::allocator<std::byte>()};
										NodeContainer<depth, tri> nc{};

										context.insert(nc, entries_0);
										ValidationRawNodeContext<depth, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
										CHECK(context == validation_context_0);
										std::cout << fmt::format("result identifier: {}", nc.raw_identifier()) << std::endl;

										utils::EntrySetGenerator_with_exclude<depth, no_entries_1, tri, max_key_part> inner_generator{entries_0};
										for (const auto &entries_1 : inner_generator) {
											SUBCASE("second_entries: {}"_format(fmt::join(entries_1, " | ")).c_str()) {
												std::vector<SingleEntry_t> all_entries = entries_0;
												all_entries.insert(all_entries.end(), entries_1.begin(), entries_1.end());

												context.insert(nc, entries_1);
												std::cout << fmt::format("result identifier: {}", nc.raw_identifier()) << std::endl;
												// std::cout << fmt::format("Actual:\n {}", context) << std::endl;
												ValidationRawNodeContext<depth, tri> validation_context{std::allocator<std::byte>(), all_entries};
												// std::cout << fmt::format("Verification:\n {}", (RawHypertrieContext<depth, tri>&)validation_context) << std::endl;
												CHECK(context == validation_context);
											}
										}
									}
								}
							}
						});
					});
				}
			}
		}

		TEST_CASE("entry_generator") {
			using config = bool_cfg<2>;
			constexpr auto depth = config::depth;
			using tri = config::tri;
			utils::EntrySetGenerator<depth, 1, tri, 2> entry_set_generator{};
			for (const auto &item : entry_set_generator) {
				fmt::print("outer: {}\n", fmt::join(item, " | "));
				utils::EntrySetGenerator_with_exclude<depth, 1, tri, 2> inner_generator{item};
				for (const auto &inner_item : inner_generator) {
					fmt::print("  inner: {}\n", fmt::join(inner_item, " | "));
				}
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 1", T,
						   bool_cfg<1>,
						   tagged_bool_cfg<1>,
						   long_cfg<1>,
						   double_cfg<1>) {
			constexpr size_t no_key_parts = 3;
			constexpr size_t min_no_entries = 1;
			constexpr size_t max_no_entries = 3;
			write_and_read<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
		}

		TEST_CASE_TEMPLATE("hypertrie depth 2", T,
						   bool_cfg<2>,
						   tagged_bool_cfg<2>,
						   long_cfg<2>,
						   double_cfg<2>) {
			constexpr size_t no_key_parts = 2;
			constexpr size_t min_no_entries = 1;
			constexpr size_t max_no_entries = 2;
			write_and_read<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
		}

		TEST_CASE_TEMPLATE("hypertrie depth 3", T,
						   bool_cfg<3>,
						   tagged_bool_cfg<3>,
						   long_cfg<3>,
						   double_cfg<3>) {
			{
				constexpr size_t no_key_parts = 3;
				constexpr size_t min_no_entries = 1;
				constexpr size_t max_no_entries = 1;
				write_and_read<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
			}
			{
				constexpr size_t no_key_parts = 2;
				constexpr size_t min_no_entries = 1;
				constexpr size_t max_no_entries = 3;
				write_and_read<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
			}

		}

		TEST_CASE_TEMPLATE("hypertrie depth 4", T,
						   bool_cfg<4>,
						   tagged_bool_cfg<4>,
						   long_cfg<4>,
						   double_cfg<4>) {
			{
				constexpr size_t no_key_parts = 3;
				constexpr size_t min_no_entries = 1;
				constexpr size_t max_no_entries = 1;
				write_and_read<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
			}
			{
				constexpr size_t no_key_parts = 2;
				constexpr size_t min_no_entries = 2;
				constexpr size_t max_no_entries = 3;
				write_and_read<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
			}
		}
	}
}// namespace Dice::hypertrie::tests::core::node
