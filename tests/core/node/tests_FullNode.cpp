#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include <AssetGenerator.hpp>
#include <Dice/hypertrie/internal/util/name_of_type.hpp>
#include <Node_test_configs.hpp>

#include <Dice/hypertrie/internal/raw/node/FullNode.hpp>


namespace hypertrie::tests::core::node {

	TEST_SUITE("FullNode") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		template<size_t depth, HypertrieCoreTrait tri>
		void create() {
			using key_part_type = typename tri::key_part_type;
			using value_type = typename tri::value_type;

			hypertrie::tests::utils::RawGenerator<depth, tri> gen{};

			SUBCASE(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
								depth, name_of_type<key_part_type>(), name_of_type<value_type>())
							.c_str()) {

				SUBCASE("construct empty node") {
					FullNode<depth, tri> node{1, std::allocator<std::byte>()};

					for (size_t pos : iter::range(depth))
						REQUIRE(node.edges(pos).size() == 0);
					REQUIRE(node.size() == 0);

					SUBCASE("add entry") {
						auto [raw_key, value] = gen.entry();
						for (size_t pos : iter::range(depth)) {
							if constexpr (depth == 1) {
								if constexpr (tri::is_bool_valued) {
									node.edges(0).insert(raw_key[pos]);
								} else {
									node.edges(0)[raw_key[pos]] = value;
								}
							} else {
								if constexpr (depth == 2 and tri::taggable_key_part) {// this implies boolean-valued
									node.edges(pos)[raw_key[pos]] = Hash_or_InplaceNode<depth - 1, tri>(raw_key.subkey(pos)[0]);
								} else {
									node.edges(pos)[raw_key[pos]] = TensorHash<depth - 1, tri>().addFirstEntry(raw_key.subkey(pos), value);
								}
							}
						}

						SUBCASE("copy node") {
							FullNode<depth, tri> copied_node{node};
							REQUIRE(node == copied_node);
						}
					}
				}

				if constexpr (depth == 1) {
					SUBCASE("Create height 1 node with two entries") {
						auto key_0 = gen.key();
						auto key_1 = gen.key();
						auto value_0 = gen.value();
						auto value_1 = gen.value();

						// initialize node with two entries
						FullNode<1, tri> node_init_list{{{key_0, value_0}, {key_1, value_1}}, 1, std::allocator<std::byte>()};
						// alternative initialization for boolean valued node
						if constexpr (tri::is_bool_valued) {
							FullNode<1, tri> node_init_list_same{{key_0, key_1}, 1, std::allocator<std::byte>()};
							REQUIRE(node_init_list == node_init_list_same);
						}

						// populate node manually
						FullNode<1, tri> node_manual{1, std::allocator<std::byte>()};
						// insert first entry
						if constexpr (tri::is_bool_valued)
							node_manual.edges(0).insert(key_0[0]);
						else
							node_manual.edges(0).insert({key_0[0], value_0});

						// check if it was inserted correctly
						if (std::tie(key_0, value_0) != std::tie(key_1, value_1))
							REQUIRE(node_init_list != node_manual);
						else
							REQUIRE(node_init_list == node_manual);

						// insert second entry
						if constexpr (tri::is_bool_valued)
							node_manual.edges(0).insert(key_1[0]);
						else
							node_manual.edges(0).insert({key_1[0], value_1});

						// check if node_init_list is equivalent to manual insertion
						REQUIRE(node_init_list == node_manual);
					}
				}
			}
		}


		DOCTEST_TEST_CASE_TEMPLATE("create node", T,
								   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
								   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
								   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
								   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>) {
			create<T::depth, typename T::tri>();
		}
	}
};// namespace hypertrie::tests::core::node