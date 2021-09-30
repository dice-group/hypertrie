#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include <AssetGenerator.hpp>
#include <Dice/hypertrie/internal/util/name_of_type.hpp>
#include <Node_test_configs.hpp>

#include <Dice/hypertrie/internal/raw/node/RawHypertrieContext.hpp>


namespace hypertrie::tests::core::node {

	TEST_SUITE("NodeStorage") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		template<size_t depth, HypertrieCoreTrait tri>
		void create() {
//			using key_part_type = typename tri::key_part_type;
//			using value_type = typename tri::value_type;

		}

		TEST_CASE("storage") {
			using tri = typename tagged_bool_cfg<5>::tri;
			RawHypertrieContext<5, tri> x{std::allocator<std::byte>()};
			NodeContainer<5, tri> nc{};
			std::vector<SingleEntry<5, tri_with_stl_alloc<tri>>> entries;
			x.insert(nc, entries);

//			update_node_in_context<5, tagged_bool_cfg<5>::tri>;
		}


		DOCTEST_TEST_CASE_TEMPLATE("allocate node", T,
								   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
								   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
								   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
								   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>) {
			create<T::depth, typename T::tri>();
		}
	}
};// namespace hypertrie::tests::core::node