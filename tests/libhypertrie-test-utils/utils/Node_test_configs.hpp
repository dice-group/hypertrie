#ifndef HYPERTRIE_NODE_TEST_CONFIGS_HPP
#define HYPERTRIE_NODE_TEST_CONFIGS_HPP

#include <dice/hypertrie/internal/container/AllContainer.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>

namespace dice::hypertrie::tests {


	template<size_t depth_v, HypertrieTrait htt>
	struct Node_test_config {
		static constexpr size_t depth = depth_v;
		using htt_t = htt;
	};

	template<size_t depth>
	using bool_cfg = Node_test_config<depth, ::dice::hypertrie::default_bool_Hypertrie_trait>;

	template<size_t depth>
	using tagged_bool_cfg = Node_test_config<depth, ::dice::hypertrie::tagged_bool_Hypertrie_trait>;

	template<size_t depth>
	using long_cfg = Node_test_config<depth, ::dice::hypertrie::default_long_Hypertrie_trait>;

	template<size_t depth>
	using double_cfg = Node_test_config<depth, ::dice::hypertrie::default_double_Hypertrie_trait>;
}// namespace dice::hypertrie::tests

#endif//HYPERTRIE_NODE_TEST_CONFIGS_HPP
