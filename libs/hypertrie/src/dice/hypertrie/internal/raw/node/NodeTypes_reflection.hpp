#ifndef HYPERTRIE_NODETYPES_REFLECTION_HPP
#define HYPERTRIE_NODETYPES_REFLECTION_HPP

#include "dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"

namespace dice::hypertrie::internal::raw {

	template<template<size_t, typename, typename> typename node_type_t>
	struct is_SingleEntryNode : std::false_type {
	};

	template<>
	struct is_SingleEntryNode<SingleEntryNode> : std::true_type {
	};

	template<template<size_t, typename, typename> typename node_type_t>
	struct is_FullNode : std::false_type {
	};

	template<>
	struct is_FullNode<FullNode> : std::true_type {
	};

	template<template<size_t, typename, typename> typename node_type_t>
	static constexpr bool is_SingleEntryNode_v = is_SingleEntryNode<node_type_t>::value;

	template<template<size_t, typename, typename> typename node_type_t>
	static constexpr bool is_FullNode_v = is_FullNode<node_type_t>::value;

}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_NODETYPES_REFLECTION_HPP
