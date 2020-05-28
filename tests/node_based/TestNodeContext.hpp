#ifndef HYPERTRIE_TESTNODECONTEXT_H
#define HYPERTRIE_TESTNODECONTEXT_H

#include <Dice/hypertrie/internal/node_based/NodeContext.hpp>

#include "../utils/AssetGenerator.hpp"


namespace hypertrie::tests::node_based::node_context {

	using namespace hypertrie::internal::node_based;

	template<size_t depth, typename key_part_type>
	using Key = hypertrie::internal::RawKey<depth, key_part_type>;

	TEST_CASE("Basic Example", "[NodeContext]") {

		constexpr size_t depth = 5;
		using tr = default_long_Hypertrie_internal_t;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		// create context
		NodeContext<depth, tr> context{};
		// create emtpy primary node
		NodeContainer<depth, tr> nc = context.newPrimaryNode<depth>();

		auto entries = gen.entries(2);
		auto [key, value] = *entries.begin();

		// insert value
		context.set<depth>(nc, key, value);

		// get value
		{
			auto read_value = context.get(nc, key);
			REQUIRE(read_value == value);
		}

		// insert another value
		auto [second_key, second_value] = *entries.rbegin();

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

};// namespace hypertrie::tests::node_based::node_context

#endif//HYPERTRIE_TESTNODECONTEXT_H
