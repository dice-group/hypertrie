#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>


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

namespace hypertrie::tests::core::node {

	TEST_SUITE("randomized testing of RawNodeContext") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;


		template<size_t depth, HypertrieCoreTrait tri>
		void write_and_read() {
			SUBCASE("{}"_format(tri{}).c_str()) {


				SUBCASE("hypertrie depth = {}"_format(depth).c_str()) {

					using key_part_type = typename tri::key_part_type;
					using value_type = typename tri::value_type;

					utils::RawEntryGenerator<depth, tri> gen{};

					for (size_t count : iter::chain(iter::range(1, 10), iter::range(10, 30, 5), iter::range(300, 301))) {
						SUBCASE("insert {} entries "_format(count).c_str()) {
							auto runs = (count != 300) ? 50 : 5;
							for (const auto i : iter::range(runs)) {
								SUBCASE("{}"_format(i).c_str()) {
									// TODO: reconsider -- bad for high count and low depth
									gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(count, 2.0 / depth))));
									gen.setValueMinMax(value_type(1), value_type(2));
									// std::cout << "key_part_max " << gen.getKeyPartMax() << "\n"
									// 		     << "value_max " << gen.getValueMax() << std::endl;
									gen.wind(i);

									auto entries = gen.entries(count);
									RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
									NodeContainer<depth, tri> nc{};
									context.insert(nc, entries);

									std::cout << fmt::format("result identifier: {}", nc.identifier()) << std::endl;
									// std::cout << fmt::format("{}", context) << std::endl;
									// std::cout << fmt::format("{{ {} }}", fmt::join(entries, ", \n")) << std::endl;
									ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), entries};

									REQUIRE(validation_context == context);
									for (const auto &entry : entries)
										REQUIRE(context.get(nc, entry.key()) == entry.value());
								}
							}
						}

						SUBCASE("insert 2x{} entries "_format(count).c_str()) {
							auto runs = (count != 300) ? 50 : 5;
							for (const auto i : iter::range(runs)) {
								SUBCASE("{}"_format(i).c_str()) {
									auto total_count = 2 * count;
									gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(total_count, 2.0 / depth))));
									gen.setValueMinMax(value_type(1), value_type(2));
									// std::cout << "key_part_max " << gen.getKeyPartMax() << "\n"
									// 		     << "value_max " << gen.getValueMax() << std::endl;
									gen.wind(i);

									auto all_entries = gen.entries(total_count);
									decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
									decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
									std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
									std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
									std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
									RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
									NodeContainer<depth, tri> nc{};
									context.insert(nc, entries_0);
									ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
									REQUIRE(validation_context_0 == context);
									for (const auto &entry : entries_0)
										REQUIRE(context.get(nc, entry.key()) == entry.value());
									std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;


									context.insert(nc, entries_1);
									ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
									REQUIRE(validation_context == context);
									for (const auto &entry : all_entries)
										REQUIRE(context.get(nc, entry.key()) == entry.value());
									std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;
								}
							}
						}
					}
				}
			}
		}

		TEST_CASE_TEMPLATE("write and read", T,
						   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
						   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
						   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
						   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>
						   //
		) {
			write_and_read<T::depth, typename T::tri>();
		}
	}
};// namespace hypertrie::tests::core::node
