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

	TEST_CASE("Basic Usage", "[NodeContext]") {

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

};// namespace hypertrie::tests::node_based::node_context

#endif//HYPERTRIE_TESTNODECONTEXT_H
