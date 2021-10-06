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
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>


namespace hypertrie::tests::core::node {

	TEST_SUITE("randomized testing of RawNodeContext") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;


		template<size_t depth, HypertrieCoreTrait tri>
		void insert_and_read() {
			SUBCASE("hypertrie depth = {}"_format(depth).c_str()) {

				SUBCASE("{}"_format(tri{}).c_str()) {

					using key_part_type = typename tri::key_part_type;
					using value_type = typename tri::value_type;

					static utils::RawEntryGenerator<depth, tri> gen{};

					for (size_t count : iter::chain(iter::range(1, 10), iter::range(10, 30, 5), iter::range(300, 301))) {
						SUBCASE("insert {} entries "_format(count).c_str()) {
							auto runs = (count != 300) ? 25 : 1;
							for (const auto i : iter::range(runs)) {
								SUBCASE("{}"_format(i).c_str()) {
									gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + double(count) / double(depth + 1)));
									gen.setValueMinMax(value_type(1), value_type(1 + double(count) / double(10 + 1)));

									auto entries = gen.entries(count);
									RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
									NodeContainer<depth, tri> nc{};
									context.insert(nc, entries);
									ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), entries};

									REQUIRE(validation_context == context);
									for (const auto &entry : entries)
										REQUIRE(context.get(nc, entry.key()) == entry.value());
								}
							}
						}
					}
				}
			}
		}

		TEST_CASE_TEMPLATE("insert and read", T,
						   //								   								   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
						   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>// ,
																															 //									   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>
																															 //								   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>
		) {
			insert_and_read<T::depth, typename T::tri>();
		}
	}
};// namespace hypertrie::tests::core::node
