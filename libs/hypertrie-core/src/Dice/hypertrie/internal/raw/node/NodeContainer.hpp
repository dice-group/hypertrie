#ifndef HYPERTRIE_NODECONTAINER_HPP
#define HYPERTRIE_NODECONTAINER_HPP

#include "Dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "Dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"
#include "Dice/hypertrie/internal/raw/node/TensorHash.hpp"
#include "Dice/hypertrie/internal/raw/node/Identifier.hpp"
#include "Dice/hypertrie/internal/util/UnsafeCast.hpp"

namespace hypertrie::internal::raw {
	/**
	 * Parent type to NodeContainer, SpecificNodeContainer (and its aliases CompressedNodeContainer and UncompressedNodeContainer).
	 * This class defines the memory layout for the inheriting classes. Inheriting classes must not add any further fields.
	 */
	template<typename allocator_type_t>
	struct RawNodeContainer {
		using allocator_type = typename std::allocator_traits<allocator_type_t>::template rebind_alloc<void>;

		using VoidNodePtr = typename std::allocator_traits<allocator_type>::void_pointer;
		static_assert(sizeof(VoidNodePtr) <= sizeof(size_t), "The void node pointer type must fit into a size_t");

	protected:
		size_t node_identifier_ = 0;

		size_t node_ptr_;

	public:
		RawNodeContainer(size_t identifier, VoidNodePtr void_node_ptr) noexcept
			: node_identifier_(identifier),
			  node_ptr_(util::unsafe_cast<size_t>(void_node_ptr)) {}

		[[nodiscard]] VoidNodePtr &void_node_ptr() noexcept { return util::unsafe_cast<VoidNodePtr>(node_ptr_); }
		[[nodiscard]] const VoidNodePtr &void_node_ptr() const noexcept { return util::unsafe_cast<VoidNodePtr const>(node_ptr_); }
	};

	template<size_t depth,
			 HypertrieCoreTrait tri_t,
			 template<size_t, typename> typename node_type>
	struct SpecificNodeContainer;

	/**
	 * A Node container holds a pointer to a Node and it's TensorHash. The container does not own the node i.e. destructing the NodeContainer does not destruct the Node.
	 * @tparam depth
	 * @tparam tri_t
	 */
	template<size_t depth,
			 HypertrieCoreTrait tri_t>
	struct NodeContainer : public RawNodeContainer<typename tri_t::allocator_type> {
		using RawNodeContainer_t = RawNodeContainer<typename tri_t::allocator_type>;
		using tri = tri_t;
		using allocator_type = typename tri::allocator_type;

		using Identifier_t = Identifier<depth, tri>;
		using VoidNodePtr = typename RawNodeContainer_t::void_ptr_type;
		NodeContainer() = default;

		NodeContainer(const Identifier_t &identifier, VoidNodePtr node_ptr) noexcept
			: RawNodeContainer_t{(size_t) identifier,
								 util::unsafe_cast<size_t>(node_ptr)} {}

		template<template<size_t, typename> typename node_type>
		explicit NodeContainer(const SpecificNodeContainer<depth, tri, node_type> &other) noexcept
			: NodeContainer{other.node_identifier(),
							(VoidNodePtr) other.node_ptr()} {}
		template<template<size_t, typename> typename node_type>

		explicit NodeContainer(const SpecificNodeContainer<depth, tri, node_type> &&other) noexcept
			: NodeContainer{other.node_identifier(),
							(VoidNodePtr) other.node_ptr()} {}


		template<template<size_t, typename> typename node_type>
		[[nodiscard]] auto specific() const noexcept { return SpecificNodeContainer<depth, tri_t, node_type>{this->node_identifier_, this->node_ptr_}; }

		[[nodiscard]] Identifier_t identifier() const noexcept { return util::unsafe_cast<Identifier_t const>(this->node_identifier_); }

		[[nodiscard]] Identifier_t &identifier() noexcept { return util::unsafe_cast<Identifier_t>(this->node_identifier_); }

		[[nodiscard]] bool is_sen() const noexcept { return identifier().is_sen(); }

		[[nodiscard]] bool is_fn() const noexcept { return identifier().is_fn(); }

		/**
		 * Check if the NodeRepr (NodeHash/TaggedNodeHash) is empty.
		 */
		[[nodiscard]] bool empty() const noexcept { return identifier().empty(); }

		/**
		 * Check if the the NodePtr is null.
		 */
		[[nodiscard]] bool is_null_ptr() const noexcept { return this->void_node_ptr() == VoidNodePtr{}; }
	};

	template<size_t depth, HypertrieCoreTrait tri_t, template<size_t, typename> typename node_type_t>
	struct SpecificNodeContainer : public NodeContainer<depth, tri_t> {
		using tri = tri_t;
		using NodeContainer_t = NodeContainer<depth, tri>;
		using RawNodeContainer_t = typename NodeContainer_t::RawNodeContainer_t;
		using allocator_type = typename tri::allocator_type;

		using Identifier_t = typename NodeContainer_t::Identifier_t;
		using Node = node_type_t<depth, size_t>;
		using NodePtr = typename tri::template allocator_pointer<Node>;
		using VoidNodePtr = typename NodeContainer_t::VoidNodePtr;

		SpecificNodeContainer(Identifier_t identifier, NodePtr node_ptr) noexcept
			: RawNodeContainer_t{(size_t) identifier,
								 util::unsafe_cast<size_t>(node_ptr)} {}

		[[nodiscard]] constexpr NodePtr &node_ptr() noexcept { return util::unsafe_cast<NodePtr>(this->node_ptr_); }
		[[nodiscard]] constexpr const NodePtr &node_ptr() const noexcept { return util::unsafe_cast<NodePtr const>(this->node_ptr_); }

		[[nodiscard]] VoidNodePtr &void_node_ptr() noexcept { return (VoidNodePtr) node_ptr(); }
		[[nodiscard]] const VoidNodePtr &void_node_ptr() const noexcept { return (VoidNodePtr) node_ptr(); }

		//		inline auto getChildHashOrValue(size_t pos, typename tri_t::key_part_type key_part) {
		//			static_assert(compression == NodeCompression::uncompressed, "Function getChildHashOrValue is only applicable for uncompressed nodes.");
		//
		//			assert(pos < depth);
		//			return this->uncompressed_node()->child(pos, key_part);
		//		}

		explicit operator NodeContainer_t() const noexcept { return {this->identifier(), this->void_node_ptr()}; }
	};

	template<size_t depth, HypertrieCoreTrait tri>
	using FNContainer = SpecificNodeContainer<depth, tri, FullNode>;
	template<size_t depth, HypertrieCoreTrait tri>
	using SENContainer = SpecificNodeContainer<depth, tri, SingleEntryNode>;

};// namespace hypertrie::internal::raw


#endif//HYPERTRIE_NODECONTAINER_HPP
