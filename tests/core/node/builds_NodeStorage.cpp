#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <dice/hypertrie/internal/util/name_of_type.hpp>
#include <utils/AssetGenerator.hpp>
#include <utils/Node_test_configs.hpp>

#include <dice/hypertrie/internal/raw/node/NodeStorage.hpp>
#include <dice/hypertrie/internal/raw/node/RawIdentifier.hpp>
#include <dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp>


namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("NodeStorage") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		template<size_t depth, HypertrieTrait htt_t>
		void create() {
			using key_part_type = typename htt_t::key_part_type;
			using value_type = typename htt_t::value_type;
			using RawIdentifier_t = RawIdentifier<depth, htt_t>;

			hypertrie::tests::utils::RawGenerator<depth, htt_t> gen{};

			SUBCASE(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
								depth, name_of_type<key_part_type>(), name_of_type<value_type>())
							.c_str()) {
				SpecificNodeStorage<depth, htt_t, SingleEntryNode, std::allocator<std::byte>> node_storage{std::allocator<std::byte>()};

				SingleEntry<depth, htt_t> const e{{}, true};
				auto node_ptr = node_storage.node_lifecycle().new_(e, 0UL);
				node_storage.nodes().insert(node_ptr);

				CHECK(node_storage.nodes().find(RawIdentifier_t{e}) != node_storage.nodes().end());
			}
		}

		TEST_CASE("storage") {
			NodeStorage<5, tagged_bool_cfg<5>::htt_t, std::allocator<std::byte>> node_storage{std::allocator<std::byte>()};
		}


		DOCTEST_TEST_CASE_TEMPLATE("allocate node", T,
								   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
								   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
								   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
								   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>) {
			create<T::depth, typename T::htt_t>();
		}
	}
};// namespace dice::hypertrie::tests::core::node
