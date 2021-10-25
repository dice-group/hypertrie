#ifndef HYPERTRIE_NODE_TEST_CONFIGS_HPP
#define HYPERTRIE_NODE_TEST_CONFIGS_HPP

#include <Dice/hypertrie/Hypertrie_default_traits.hpp>

namespace Dice::hypertrie::tests {


	template<size_t depth_v, HypertrieTrait tr_t>
	struct Node_test_config {
		static constexpr size_t depth = depth_v;
		using tr = tr_t;
		using tri = internal::raw::Hypertrie_core_t<tr>;
	};

	template<size_t depth>
	using bool_cfg = Node_test_config<depth, ::Dice::hypertrie::default_bool_Hypertrie_trait>;

	template<size_t depth>
	using tagged_bool_cfg = Node_test_config<depth, ::Dice::hypertrie::tagged_bool_Hypertrie_trait>;

	template<size_t depth>
	using long_cfg = Node_test_config<depth, ::Dice::hypertrie::default_long_Hypertrie_trait>;

	template<size_t depth>
	using double_cfg = Node_test_config<depth, ::Dice::hypertrie::default_double_Hypertrie_trait>;
}// namespace Dice::hypertrie::tests

#endif//HYPERTRIE_NODE_TEST_CONFIGS_HPP
