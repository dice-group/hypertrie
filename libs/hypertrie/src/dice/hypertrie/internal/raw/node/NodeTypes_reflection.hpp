#ifndef HYPERTRIE_NODETYPES_REFLECTION_HPP
#define HYPERTRIE_NODETYPES_REFLECTION_HPP

#include "dice/hypertrie/internal/raw/node/NodeTypes_predeclare.hpp"
#include "dice/hypertrie/internal/raw/node/RawIdentifier.hpp"

namespace dice::hypertrie::internal::raw {

	template<template<size_t, typename, typename...> typename node_type_t>
	struct is_SingleEntryNode : std::false_type {
	};

	template<>
	struct is_SingleEntryNode<SingleEntryNode> : std::true_type {
	};

	template<template<size_t, typename, typename...> typename node_type_t>
	struct is_FullNode : std::false_type {
	};

	template<>
	struct is_FullNode<FullNode> : std::true_type {
	};

	template<template<size_t, typename, typename...> typename node_type_t>
	struct is_CartesianNode : std::false_type {
	};

	template<>
	struct is_CartesianNode<CartesianNode> : std::true_type {
	};

	template<template<size_t, typename, typename...> typename node_type_t>
	inline constexpr bool is_SingleEntryNode_v = is_SingleEntryNode<node_type_t>::value;

	template<template<size_t, typename, typename...> typename node_type_t>
	inline constexpr bool is_FullNode_v = is_FullNode<node_type_t>::value;

	template<template<size_t, typename, typename...> typename node_type_t>
	inline constexpr bool is_CartesianNode_v = is_CartesianNode<node_type_t>::value;


	template<template<size_t, HypertrieTrait, ByteAllocator ...> typename node_type_t, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct instantiate_Node;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct instantiate_Node<FullNode, depth, htt_t, allocator_type> {
		using type = FullNode<depth, htt_t, allocator_type>;
	};

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct instantiate_Node<SingleEntryNode, depth, htt_t, allocator_type> {
		using type = SingleEntryNode<depth, htt_t>;
	};

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct instantiate_Node<CartesianNode, depth, htt_t, allocator_type> {
		using type = CartesianNode<depth, htt_t, allocator_type>;
	};

	template<template<size_t, HypertrieTrait, ByteAllocator ...> typename node_type_t, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	using instantiate_Node_t = typename instantiate_Node<node_type_t, depth, htt_t, allocator_type>::type;


	template<template<size_t, HypertrieTrait, ByteAllocator ...> typename node_type>
	constexpr IdentifierTag get_node_tag() noexcept {
		if constexpr (is_FullNode_v<node_type>) {
			return IdentifierTag::FN;
		} else if constexpr (is_SingleEntryNode_v<node_type>) {
			return IdentifierTag::SEN;
		} else if constexpr (is_CartesianNode_v<node_type>) {
			return IdentifierTag::XN;
		} else {
			return IdentifierTag::Indeterminate;
		}
	}

	template<template<size_t, HypertrieTrait, ByteAllocator ...> typename node_type>
	constexpr bool node_tag_matches(IdentifierTag tag) noexcept {
		return get_node_tag<node_type>() == tag;
	}

}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_NODETYPES_REFLECTION_HPP
