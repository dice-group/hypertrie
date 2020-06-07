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
		value_type no_value = context.template set<depth>(nc, {1,4,5}, 2.0);
		REQUIRE(no_value == 0.0);

		// setting it a second time doesn't change a thing
		value_type unchanged_value = context.template set<depth>(nc, {1,4,5}, 2.0);
		REQUIRE(unchanged_value == 2.0);

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

		// < 1, :, : >
		UncompressedNodeContainer<2, tr> child_c1__ = context.getUncompressedNode<2>(childhash1__);
		UncompressedNode<2, tr> * child1__ = child_c1__.node();
		REQUIRE(child1__->size() == 2);
		REQUIRE(child1__->ref_count() == 1);
		INFO(fmt::format("child1__ {}",*child1__));
		TaggedNodeHash childhash12_ = child1__->child(0, 2);
		TaggedNodeHash childhash1_3 = child1__->child(1, 3);

		// < 1, 2, : >
		CompressedNodeContainer<1, tr> child_c12_ = context.getCompressedNode<1>(childhash12_);
		INFO(fmt::format("childhash12_ {}",childhash12_));
		INFO(fmt::format("{}",context.getNodeStorage<1, NodeCompression::compressed>()));
		CompressedNode<1, tr> * child12_  = child_c12_.node();
		INFO(fmt::format("child12_ {}",*child12_));
		REQUIRE(child12_->key() == tr::template RawKey<1>{3});
		REQUIRE(child12_->value() == 1.0);
		REQUIRE(child12_->ref_count() == 1);

		// < 1, :, 3 >
		CompressedNodeContainer<1, tr> child_c1_3 = context.getCompressedNode<1>(childhash1_3);
		INFO(fmt::format("childhash1_3 {}",childhash1_3));
		INFO(fmt::format("{}",context.getNodeStorage<1, NodeCompression::compressed>()));
		CompressedNode<1, tr> * child1_3  = child_c1_3.node();
		REQUIRE(child1_3->key() == tr::template RawKey<1>{2});
		REQUIRE(child1_3->value() == 1.0);
		REQUIRE(child1_3->ref_count() == 1);

		// < 1, :, : >
		TaggedNodeHash childhash14_ = child1__->child(0, 4);
		TaggedNodeHash childhash1_5 = child1__->child(1, 5);

		// < 1, 4, : >
		CompressedNodeContainer<1, tr> child_c14_ = context.getCompressedNode<1>(childhash14_);
		INFO(fmt::format("childhash14_ {}",childhash14_));
		INFO(fmt::format("{}",context.getNodeStorage<1, NodeCompression::compressed>()));
		CompressedNode<1, tr> * child14_  = child_c14_.node();
		INFO(fmt::format("child14_ {}",*child14_));
		REQUIRE(child14_->key() == tr::template RawKey<1>{5});
		REQUIRE(child14_->value() == 2.0);
		REQUIRE(child14_->ref_count() == 1);

		// < 1, :, 5 >
		CompressedNodeContainer<1, tr> child_c1_5 = context.getCompressedNode<1>(childhash1_5);
		INFO(fmt::format("childhash1_5 {}",childhash1_5));
		INFO(fmt::format("{}",context.getNodeStorage<1, NodeCompression::compressed>()));
		CompressedNode<1, tr> * child1_5  = child_c1_5.node();
		REQUIRE(child1_5->key() == tr::template RawKey<1>{4});
		REQUIRE(child1_5->value() == 2.0);
		REQUIRE(child1_5->ref_count() == 1);

		// < :, 2, : >
		CompressedNodeContainer<2, tr> child_c_2_ = context.getCompressedNode<2>(childhash_2_);
		CompressedNode<2, tr> * child_2_  = child_c_2_.node();
		INFO(fmt::format("child_2_ {}",*child_2_));
		REQUIRE(child_2_->key() == tr::template RawKey<2>{1,3});
		REQUIRE(child_2_->value() == 1.0);
		REQUIRE(child_2_->ref_count() == 1);

		// < :, :, 3 >
		CompressedNodeContainer<2, tr> child_c__3 = context.getCompressedNode<2>(childhash__3);
		CompressedNode<2, tr> * child__3  = child_c__3.node();
		INFO(fmt::format("child__3 {}",*child__3));
		REQUIRE(child__3->key() == tr::template RawKey<2>{1,2});
		REQUIRE(child__3->value() == 1.0);
		REQUIRE(child__3->ref_count() == 1);

		// < :, 4, : >
		CompressedNodeContainer<2, tr> child_c_4_ = context.getCompressedNode<2>(childhash_4_);
		CompressedNode<2, tr> * child_4_  = child_c_4_.node();
		INFO(fmt::format("child_4_ {}",*child_4_));
		REQUIRE(child_4_->key() == tr::template RawKey<2>{1,5});
		REQUIRE(child_4_->value() == 2.0);
		REQUIRE(child_4_->ref_count() == 1);

		// < :, :, 5 >
		CompressedNodeContainer<2, tr> child_c__5 = context.getCompressedNode<2>(childhash__5);
		CompressedNode<2, tr> * child__5  = child_c__5.node();
		INFO(fmt::format("child__5 {}",*child__5));
		REQUIRE(child__5->key() == tr::template RawKey<2>{1,4});
		REQUIRE(child__5->value() == 2.0);
		REQUIRE(child__5->ref_count() == 1);

	}

};// namespace hypertrie::tests::node_based::node_context

#endif//HYPERTRIE_TESTNODECONTEXT_H
