#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <AssetGenerator.hpp>
#include <Node_test_configs.hpp>

#include <Dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/util/name_of_type.hpp>


namespace hypertrie::tests::core::node {

	TEST_SUITE("SingleEntryNode") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		template<size_t depth, HypertrieCoreTrait tri>
		void createCompressedNode() {
			using key_part_type = typename tri::key_part_type;
			using value_type = typename tri::value_type;
			using RawKey = ::hypertrie::internal::raw::RawKey<depth, tri>;

			hypertrie::tests::utils::RawGenerator<depth, tri> gen{};

			SUBCASE(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
								depth, name_of_type<key_part_type>(), name_of_type<value_type>())
							.c_str()) {

				RawKey key = gen.key();

				// unused for bool
				[[maybe_unused]] value_type value = gen.value();

				SingleEntryNode<depth, tri> node = [&]() {
					if constexpr (tri::is_bool_valued) return SingleEntryNode<depth, tri>{key};
					else
						return SingleEntryNode<depth, tri>{key, value};
				}();

				REQUIRE(node.key() == key);
				if constexpr (not std::is_same_v<value_type, bool>)
					REQUIRE(node.value() == value);
				REQUIRE(node.size() == 1);
			}
		}


		DOCTEST_TEST_CASE_TEMPLATE("create node", T, bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>) {
			createCompressedNode<T::depth, typename T::tri>();
		}
	}
};// namespace hypertrie::tests::core::node