#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <dice/hypertrie/internal/util/name_of_type.hpp>
#include <utils/AssetGenerator.hpp>
#include <utils/Node_test_configs.hpp>

#include <dice/hypertrie/internal/raw/node/FullNode.hpp>
#include <utils/ValidationRawNodeContext.hpp>


namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("FullNode") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{};// allocator instance

		template<size_t depth, HypertrieTrait htt_t>
		void create() {
			using key_part_type = typename htt_t::key_part_type;
			using value_type = typename htt_t::value_type;

			hypertrie::tests::utils::RawGenerator<depth, htt_t> gen{};

			SUBCASE(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
								depth, name_of_type<key_part_type>(), name_of_type<value_type>())
							.c_str()) {

				SUBCASE("construct empty node") {
					FullNode<depth, htt_t, allocator_type> node{RawIdentifier<depth, htt_t>{}.retag_as_fn(), 1, std::allocator<std::byte>()};
					FullNode<depth - 1, htt_t, allocator_type> child{RawIdentifier<depth - 1, htt_t>{}.retag_as_fn(), 1, std::allocator<std::byte>{}};

					for (size_t pos : iter::range(depth))
						REQUIRE(node.edges(pos).size() == 0);
					REQUIRE(node.size() == 0);

					SUBCASE("add entry") {
						auto [raw_key, value] = gen.entry();
						for (size_t pos : iter::range(depth)) {
							if constexpr (depth == 1) {
								if constexpr (htt_t::is_bool_valued) {
									node.edges(0).insert(raw_key[pos]);
								} else {
									node.edges(0)[raw_key[pos]] = value;
								}
							} else {
								node.edges(pos)[raw_key[pos]] = &child;
							}
						}

						node.hash() = RawIdentifier<depth, htt_t>::hash_single_entry(SingleEntry<depth, htt_t>{raw_key, value});

						if constexpr (depth > 1) {
							node.size() = 1;
						}

						SUBCASE("copy node") {
							FullNode<depth, htt_t, allocator_type> copied_node{node};
							Equal::check_equal(node, copied_node);
						}
					}
				}
			}
		}


		DOCTEST_TEST_CASE_TEMPLATE("create node", T,
								   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
								   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
								   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
								   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>) {
			create<T::depth, typename T::htt_t>();
		}
	}
};// namespace dice::hypertrie::tests::core::node