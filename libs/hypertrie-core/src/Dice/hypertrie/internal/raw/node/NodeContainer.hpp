#ifndef HYPERTRIE_NODECONTAINER_HPP
#define HYPERTRIE_NODECONTAINER_HPP

#include "Dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "Dice/hypertrie/internal/raw/node/Identifier.hpp"
#include "Dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"
#include "Dice/hypertrie/internal/util/UnsafeCast.hpp"

namespace Dice::hypertrie::internal::raw {
	/**
	 * Parent type to NodeContainer, SpecificNodeContainer (and its aliases CompressedNodeContainer and UncompressedNodeContainer).
	 * This class defines the memory layout for the inheriting classes. Inheriting classes must not add any further fields.
	 */
	template<HypertrieCoreTrait tri_t>
	struct RawNodeContainer {
		using tri = tri_t;
		using allocator_type = typename std::allocator_traits<typename tri::allocator_type>::template rebind_alloc<void>;

		using VoidNodePtr = typename std::allocator_traits<allocator_type>::void_pointer;
		using Identifier_t = Identifier<tri>;
		static_assert(sizeof(VoidNodePtr) <= sizeof(size_t), "The void node pointer type must fit into a size_t");
		static_assert(sizeof(Identifier<tri>) <= sizeof(size_t), "The Identifier type must fit into a size_t");


	protected:
		Identifier<tri> node_identifier_{};

		size_t node_ptr_{};

		RawNodeContainer(Identifier<tri> node_identifier, size_t node_ptr) noexcept : node_identifier_(node_identifier), node_ptr_(node_ptr) {}

	public:
		RawNodeContainer() = default;
		RawNodeContainer(Identifier<tri> identifier, VoidNodePtr void_node_ptr) noexcept
			: node_identifier_(identifier),
			  node_ptr_(util::unsafe_cast<size_t>(void_node_ptr)) {}

		[[nodiscard]] VoidNodePtr &void_node_ptr() noexcept { return util::unsafe_cast<VoidNodePtr>(node_ptr_); }
		[[nodiscard]] const VoidNodePtr &void_node_ptr() const noexcept { return util::unsafe_cast<VoidNodePtr const>(node_ptr_); }

		/**
		 * Check if the NodeRepr (NodeHash/TaggedNodeHash) is empty.
		 */
		[[nodiscard]] bool empty() const noexcept { return node_identifier_.empty(); }

		[[nodiscard]] Identifier<tri> identifier() const noexcept { return node_identifier_; }

		[[nodiscard]] Identifier<tri> &identifier() noexcept { return node_identifier_; }

		[[nodiscard]] bool is_sen() const noexcept { return this->identifier().is_sen(); }

		[[nodiscard]] bool is_fn() const noexcept { return this->identifier().is_fn(); }

		/**
		 * Check if the the NodePtr is null.
		 */
		[[nodiscard]] bool is_null_ptr() const noexcept { return this->void_node_ptr() == VoidNodePtr{}; }

		bool operator==(const RawNodeContainer &rhs) const {
			return std::tie(node_identifier_, node_ptr_) == std::tie(rhs.node_identifier_, rhs.node_ptr_);
		}
		bool operator!=(const RawNodeContainer &rhs) const {
			return !(rhs == *this);
		}
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
	struct NodeContainer : public RawNodeContainer<tri_t> {
		using tri = tri_t;
		using RawNodeContainer_t = RawNodeContainer<tri>;
		using allocator_type = typename tri::allocator_type;

		using Identifier_t = typename RawNodeContainer_t::Identifier_t;
		using RawIdentifier_t = RawIdentifier<depth, tri>;
		using VoidNodePtr = typename RawNodeContainer_t::VoidNodePtr;

	protected:
		NodeContainer(Identifier<tri> identifier, VoidNodePtr void_node_ptr) noexcept
			: RawNodeContainer_t(identifier, void_node_ptr) {}

		NodeContainer(Identifier<tri> node_identifier, size_t node_ptr) noexcept : RawNodeContainer_t(node_identifier, node_ptr) {}

	public:
		NodeContainer() = default;

		explicit NodeContainer(const RawIdentifier_t &identifier, VoidNodePtr node_ptr = {}) noexcept
			: RawNodeContainer_t{identifier,
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

		[[nodiscard]] RawIdentifier_t raw_identifier() const noexcept { return util::unsafe_cast<RawIdentifier_t const>(this->node_identifier_); }

		[[nodiscard]] RawIdentifier_t &raw_identifier() noexcept { return util::unsafe_cast<RawIdentifier_t>(this->node_identifier_); }

		//		operator RawNodeContainer_t () const noexcept { return {this->identifier(), this->void_node_ptr()}; }
	};

	template<size_t depth, HypertrieCoreTrait tri_t, template<size_t, typename> typename node_type_t>
	struct SpecificNodeContainer : public NodeContainer<depth, tri_t> {
		using tri = tri_t;
		using NodeContainer_t = NodeContainer<depth, tri>;
		using RawNodeContainer_t = typename NodeContainer_t::RawNodeContainer_t;
		using allocator_type = typename tri::allocator_type;

		using Identifier_t = typename NodeContainer_t::Identifier_t;
		using RawIdentifier_t = typename NodeContainer_t::RawIdentifier_t;
		using Node = node_type_t<depth, tri>;
		using NodePtr = typename tri::template allocator_pointer<Node>;
		using VoidNodePtr = typename NodeContainer_t::VoidNodePtr;

		friend NodeContainer<depth, tri>;

	protected:
		SpecificNodeContainer(Identifier<tri> node_identifier, size_t node_ptr) noexcept : NodeContainer_t(node_identifier, node_ptr) {}

	public:
		SpecificNodeContainer() = default;

		explicit SpecificNodeContainer(Identifier_t identifier, NodePtr node_ptr = {}) noexcept
			: NodeContainer_t{identifier, node_ptr} {}

		explicit SpecificNodeContainer(RawIdentifier_t raw_identifier, NodePtr node_ptr = {}) noexcept
			: NodeContainer_t{raw_identifier, node_ptr} {}

		[[nodiscard]] constexpr NodePtr &node_ptr() noexcept { return util::unsafe_cast<NodePtr>(this->node_ptr_); }
		[[nodiscard]] constexpr const NodePtr &node_ptr() const noexcept { return util::unsafe_cast<NodePtr const>(this->node_ptr_); }

		operator NodeContainer_t() const noexcept { return {this->identifier(), this->void_node_ptr()}; }
		//		operator RawNodeContainer_t () const noexcept { return {this->identifier(), this->void_node_ptr()}; }
	};

	template<size_t depth, HypertrieCoreTrait tri>
	using FNContainer = SpecificNodeContainer<depth, tri, FullNode>;
	template<size_t depth, HypertrieCoreTrait tri>
	using SENContainer = SpecificNodeContainer<depth, tri, SingleEntryNode>;

};// namespace Dice::hypertrie::internal::raw


#endif//HYPERTRIE_NODECONTAINER_HPP
