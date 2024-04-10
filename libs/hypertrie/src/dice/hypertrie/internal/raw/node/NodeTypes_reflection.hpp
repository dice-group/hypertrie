#ifndef HYPERTRIE_NODETYPES_REFLECTION_HPP
#define HYPERTRIE_NODETYPES_REFLECTION_HPP

#include "dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"

namespace dice::hypertrie::internal::raw {

	template<template<size_t, typename, typename...> typename node_type_t>
	struct is_SingleEntryNode {
	private:
		template<template<size_t, typename, typename> typename is_FullNode>
		static constexpr std::false_type eval() { return {}; }
		template<template<size_t, typename> typename Is_SingleEntryNode>
		static constexpr std::true_type eval() { return {}; }

	public:
		using type = decltype(eval<node_type_t>());
		static constexpr bool value = type::value;
	};

	template<template<size_t, typename, typename...> typename node_type_t>
	static constexpr bool is_SingleEntryNode_v = is_SingleEntryNode<node_type_t>::value;

	template<template<size_t, typename, typename...> typename node_type_t>
	static constexpr bool is_FullNode_v = !is_SingleEntryNode<node_type_t>::value;

	template<template<size_t, typename, typename...> typename node_type_t, size_t depth, HypertrieTrait htt_t, typename... allocator_type>
	using instantiate_NodeTemplate = std::conditional_t<is_SingleEntryNode_v<node_type_t>,
														SingleEntryNode<depth, htt_t>,
														FullNode<depth, htt_t, allocator_type...>>;

}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_NODETYPES_REFLECTION_HPP
