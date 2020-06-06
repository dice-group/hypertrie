#ifndef HYPERTRIE_TESTNODECONTEXT_H
#define HYPERTRIE_TESTNODECONTEXT_H

#include <Dice/hypertrie/internal/node_based/NodeContext.hpp>

#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"


namespace hypertrie::tests::node_based::node_context {

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::node_based;

	template<size_t depth, typename key_part_type>
	using Key = hypertrie::internal::RawKey<depth, key_part_type>;

	template<HypertrieInternalTrait tr, pos_type depth>
	void basicUsage() {
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;

		SECTION(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
							depth, nameOfType<key_part_type>(), nameOfType<value_type>())) {
			utils::RawGenerator<depth, key_part_type, value_type> gen{};
			for (auto i : iter::range(5000)) {
				// create context
				NodeContext<depth, tr> context{};
				// create emtpy primary node
				NodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();

				const auto entries = gen.entries(2);
				const auto [key, value] = *entries.begin();
				const auto [second_key, second_value] = *entries.rbegin();

				// insert value
				context.template set<depth>(nc, key, value);

				// get value
				{
					auto read_value = context.get(nc, key);
					REQUIRE(read_value == value);
				}

				// insert another value

				context.template set<depth>(nc, second_key, second_value);

				// get both values
				{
					auto read_value = context.get(nc, key);
					REQUIRE(read_value == value);
				}

				{
					auto read_value = context.get(nc, second_key);
					REQUIRE(read_value == second_value);
				}
			}
		}
	}

	TEST_CASE("Test setting independent keys", "[NodeContext]") {

		basicUsage<default_bool_Hypertrie_internal_t, 1>();
		basicUsage<default_long_Hypertrie_internal_t, 1>();
		basicUsage<default_double_Hypertrie_internal_t, 1>();
		basicUsage<default_bool_Hypertrie_internal_t, 2>();
		basicUsage<default_long_Hypertrie_internal_t, 2>();
		basicUsage<default_double_Hypertrie_internal_t, 2>();
		basicUsage<default_bool_Hypertrie_internal_t, 3>();
		basicUsage<default_long_Hypertrie_internal_t, 3>();
		basicUsage<default_double_Hypertrie_internal_t, 3>();
		basicUsage<default_bool_Hypertrie_internal_t, 5>();
		basicUsage<default_long_Hypertrie_internal_t, 5>();
		basicUsage<default_double_Hypertrie_internal_t, 5>();
	}

	TEST_CASE("Test setting dependent keys", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;

		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();

		context.template set<depth>(nc, {1,2,3}, 1.0);

		// set second key with one common key part
		context.template set<depth>(nc, {1,4,5}, 2.0);

		UncompressedNode<3, tr> *node = nc.node();

		TaggedNodeHash childhash1__ = node->child(0, 1);
		REQUIRE(childhash1__.isUncompressed());
		TaggedNodeHash childhash_2_ = node->child(1, 2);
		REQUIRE(childhash_2_.isCompressed());
		TaggedNodeHash childhash_4_ = node->child(1, 4);
		REQUIRE(childhash_4_.isCompressed());
		TaggedNodeHash childhash__3 = node->child(2, 3);
		REQUIRE(childhash__3.isCompressed());
		TaggedNodeHash childhash__5 = node->child(2, 5);
		REQUIRE(childhash__5.isCompressed());

		UncompressedNodeContainer<2, tr> child_c1__ = context.getUncompressedNode<2>(childhash1__);
		UncompressedNode<2, tr> * child1__ = child_c1__.node();
		INFO(fmt::format("child1__ {}",*child1__));
		TaggedNodeHash childhash12_ = child1__->child(0, 2);

		CompressedNodeContainer<1, tr> child_c12_ = context.getCompressedNode<1>(childhash12_);
		INFO(fmt::format("childhash12_ {}",childhash12_));
		INFO(fmt::format("{}",context.getNodeStorage<1, NodeCompression::compressed>()));
		CompressedNode<1, tr> * child12_  = child_c12_.node();
		REQUIRE(child12_->key() == tr::template RawKey<1>{3});
		REQUIRE(child12_->value() == 1.0);

	}

};// namespace hypertrie::tests::node_based::node_context

#endif//HYPERTRIE_TESTNODECONTEXT_H
