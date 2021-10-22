#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <AssetGenerator.hpp>
#include <Dice/hypertrie/internal/util/name_of_type.hpp>
#include <Node_test_configs.hpp>

#include <Dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>


namespace Dice::hypertrie::tests::core::node {

	TEST_SUITE("SingleEntryNode") {
		using namespace ::Dice::hypertrie::internal::raw;
		using namespace ::Dice::hypertrie::internal::util;

		template<size_t depth, HypertrieCoreTrait tri>
		void create() {
			using key_part_type = typename tri::key_part_type;
			using value_type = typename tri::value_type;
			using RawKey = ::Dice::hypertrie::internal::raw::RawKey<depth, tri>;

			hypertrie::tests::utils::RawGenerator<depth, tri> gen{};

			SUBCASE(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
								depth, name_of_type<key_part_type>(), name_of_type<value_type>())
							.c_str()) {

				RawKey key = gen.key();

				[[maybe_unused]] value_type value = gen.value();

				SingleEntryNode<depth, tri> node{key, value};

				REQUIRE(node.key() == key);
				REQUIRE(node.value() == value);
				REQUIRE(node.size() == 1);

				SUBCASE("copy") {
					SingleEntryNode<depth, tri> another_node{node};
					REQUIRE(another_node == node);
				}
			}
		}


		DOCTEST_TEST_CASE_TEMPLATE("create node", T,
								   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
								   tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
								   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
								   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>) {
			create<T::depth, typename T::tri>();
		}
	}
};// namespace Dice::hypertrie::tests::core::node