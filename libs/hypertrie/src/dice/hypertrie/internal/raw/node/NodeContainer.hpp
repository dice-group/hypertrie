#ifndef HYPERTRIE_NODECONTAINER_HPP
#define HYPERTRIE_NODECONTAINER_HPP

#include "dice/hypertrie/internal/raw/node/Identifier.hpp"
#include "dice/hypertrie/internal/raw/node/NodeTypes_reflection.hpp"
#include "dice/hypertrie/internal/util/UnsafeCast.hpp"

namespace dice::hypertrie::internal::raw {
	/**
	 * Parent type to NodeContainer, SpecificNodeContainer (and its aliases CompressedNodeContainer and UncompressedNodeContainer).
	 * This class defines the memory layout for the inheriting classes. Inheriting classes must not add any further fields.
	 */
	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct RawNodeContainer {
	private:
		using ht_allocator_trait = hypertrie_allocator_trait<allocator_type>;

	public:
		using VoidNodePtr = typename ht_allocator_trait::void_pointer;
		using Identifier_t = Identifier<htt_t>;
		static_assert(sizeof(VoidNodePtr) <= sizeof(size_t), "The void node pointer type must fit into a size_t");
		static_assert(sizeof(Identifier<htt_t>) <= sizeof(size_t), "The Identifier type must fit into a size_t");
		static constexpr bool allocator_uses_raw_ptr = ht_allocator_trait::uses_raw_pointer;
		using PtrTypes = std::conditional_t<allocator_uses_raw_ptr,
											std::variant<VoidNodePtr>,// TODO: why??
											std::variant<VoidNodePtr, void *>>;


	protected:
		Identifier<htt_t> node_identifier_{};

		PtrTypes node_ptr_{};

	public:
		RawNodeContainer() = default;

		explicit RawNodeContainer(Identifier<htt_t> identifier, VoidNodePtr void_node_ptr = {}) noexcept
			: node_identifier_(identifier), node_ptr_{void_node_ptr} {}

		RawNodeContainer(Identifier<htt_t> identifier, void *raw_void_node_ptr = {}) noexcept
				requires(not allocator_uses_raw_ptr)
			: node_identifier_(identifier), node_ptr_{raw_void_node_ptr} {}

		[[nodiscard]] VoidNodePtr &void_node_ptr() noexcept { return std::get<VoidNodePtr>(node_ptr_); }
		[[nodiscard]] VoidNodePtr const &void_node_ptr() const noexcept { return std::get<VoidNodePtr>(node_ptr_); }

		[[nodiscard]] void *&raw_void_node_ptr() noexcept { return std::get<void *>(node_ptr_); }
		[[nodiscard]] void *const &raw_void_node_ptr() const noexcept { return std::get<void *>(node_ptr_); }


		[[nodiscard]] Identifier<htt_t> identifier() const noexcept { return node_identifier_; }

		[[nodiscard]] Identifier<htt_t> &identifier() noexcept { return node_identifier_; }

		[[nodiscard]] bool is_sen() const noexcept { return this->identifier().is_sen(); }

		[[nodiscard]] bool is_fn() const noexcept { return this->identifier().is_fn(); }

		/**
		 * Check if the NodeRepr (NodeHash/TaggedNodeHash) is empty.
		 */
		[[nodiscard]] bool empty() const noexcept { return node_identifier_.empty(); }
		/**
		 * Check if the the NodePtr is null. Node pointer is the pointer type used by the allocator defined in tri.
		 */
		[[nodiscard]] bool is_null_ptr() const noexcept { return this->void_node_ptr() == VoidNodePtr{}; }

		[[nodiscard]] bool is_raw_null_ptr() const noexcept { return this->raw_void_node_ptr() == nullptr; }

		[[nodiscard]] bool is_raw_ptr() const noexcept {
			if constexpr (allocator_uses_raw_ptr) return true;
			else
				return std::holds_alternative<void *>(node_ptr_);
		}
		/**
		 * Converts a RawNodeContainer using std::allocator to a RawNodeContainer with another allocator. Identifier and node pointer do not change.
		 * @tparam tri2
		 * @return
		 */
		template<ByteAllocator allocator_type2>
		operator RawNodeContainer<htt_t, allocator_type2>() const noexcept
				requires(ht_allocator_trait::is_stl_alloc and                         // tri2 does NOT use std::allocator
						 not hypertrie_allocator_trait<allocator_type2>::is_stl_alloc)// tri uses std::allocator
		{
			// TODO: why the hell do we need this?
			assert(this->is_raw_ptr());
			return {this->identifier(), this->raw_void_node_ptr()};
		}
	};

	template<size_t depth,
			 HypertrieTrait tri_t,
			 template<size_t, typename, typename> typename node_type, ByteAllocator allocator_type>
	struct SpecificNodeContainer;

	/**
	 * A Node container holds a pointer to a Node and it's TensorHash. The container does not own the node i.e. destructing the NodeContainer does not destruct the Node.
	 * @tparam depth
	 * @tparam tri_t
	 */
	template<size_t depth,
			 HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct NodeContainer {
	private:
		using ht_allocator_trait = hypertrie_allocator_trait<allocator_type>;

	public:
		using RawNodeContainer_t = RawNodeContainer<htt_t, allocator_type>;

		using Identifier_t = typename RawNodeContainer_t::Identifier_t;
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;
		using VoidNodePtr = typename RawNodeContainer_t::VoidNodePtr;
		using FNPtr = typename ht_allocator_trait::template pointer<FullNode<depth, htt_t, allocator_type>>;
		using SENPtr = typename ht_allocator_trait::template pointer<SingleEntryNode<depth, htt_t, allocator_type>>;
		static constexpr bool allocator_uses_raw_ptr = ht_allocator_trait::uses_raw_pointer;


	protected:
		RawIdentifier_t node_identifier_{};

		VoidNodePtr node_ptr_{};

		NodeContainer(Identifier<htt_t> identifier, VoidNodePtr void_node_ptr) noexcept
			: node_identifier_{identifier}, node_ptr_{void_node_ptr} {}

		NodeContainer(Identifier<htt_t> node_identifier, size_t node_ptr) noexcept : RawNodeContainer_t(node_identifier, node_ptr) {}

	public:
		NodeContainer() = default;

		explicit NodeContainer(const RawIdentifier_t &identifier, VoidNodePtr node_ptr = {}) noexcept
			: node_identifier_{identifier}, node_ptr_{node_ptr} {}

		explicit NodeContainer(const RawIdentifier_t &identifier, FNPtr fn_ptr) noexcept
			: node_identifier_{identifier}, node_ptr_{fn_ptr} {}

		explicit NodeContainer(const RawIdentifier_t &identifier, SENPtr sen_ptr) noexcept
			: node_identifier_{identifier}, node_ptr_{sen_ptr} {}

		explicit NodeContainer(RawNodeContainer<htt_t, allocator_type> const &raw_node_container) noexcept
			: node_identifier_{raw_node_container.identifier()},
			  node_ptr_{raw_node_container.void_node_ptr()} {}

		template<ByteAllocator allocator_type2>
		explicit NodeContainer(RawNodeContainer<htt_t, allocator_type2> const &raw_node_container) noexcept
				requires(ht_allocator_trait::is_stl_alloc and                         // tri2 does NOT use std::allocator
						 not hypertrie_allocator_trait<allocator_type2>::is_stl_alloc)// tri uses std::allocator
			: node_identifier_{raw_node_container.identifier()},
			  node_ptr_{raw_node_container.raw_void_node_ptr()} {}

		template<template<size_t, typename, typename> typename node_type>
		explicit NodeContainer(const SpecificNodeContainer<depth, htt_t, node_type, allocator_type> &other) noexcept
			: node_identifier_{other.raw_identifier()},
			  node_ptr_{other.template specific_ptr<node_type>()} {}

		template<template<size_t, typename, typename> typename node_type>
		explicit NodeContainer(const SpecificNodeContainer<depth, htt_t, node_type, allocator_type> &&other) noexcept
			: node_identifier_{other.raw_identifier()}, node_ptr_{other.template specific_ptr<node_type>()} {}

		template<template<size_t, typename, typename> typename node_type>
		[[nodiscard]] auto specific_ptr() const noexcept {
			// if node_type is FullNode
			if constexpr (is_FullNode_v<node_type>) {
				if constexpr (allocator_uses_raw_ptr)
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
					return reinterpret_cast<FNPtr>(this->node_ptr_);
				else
					return FNPtr{this->node_ptr_};
			} else {// else node_type is SingleEntryNode
				if constexpr (allocator_uses_raw_ptr)
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
					return reinterpret_cast<SENPtr>(this->node_ptr_);
				else
					return SENPtr{this->node_ptr_};
			}
		}

		template<template<size_t, typename, typename> typename node_type>
		[[nodiscard]] auto specific() const noexcept { return SpecificNodeContainer<depth, htt_t, node_type, allocator_type>{this->raw_identifier(), this->template specific_ptr<node_type>()}; }

		[[nodiscard]] Identifier<htt_t> identifier() const noexcept { return node_identifier_; }
		[[nodiscard]] Identifier<htt_t> &identifier() noexcept { return node_identifier_; }
		[[nodiscard]] RawIdentifier_t raw_identifier() const noexcept { return this->node_identifier_; }
		[[nodiscard]] RawIdentifier_t &raw_identifier() noexcept { return this->node_identifier_; }

		[[nodiscard]] VoidNodePtr &void_node_ptr() noexcept { return node_ptr_; }
		[[nodiscard]] VoidNodePtr const &void_node_ptr() const noexcept { return node_ptr_; }

		[[nodiscard]] bool is_sen() const noexcept { return this->identifier().is_sen(); }
		[[nodiscard]] bool is_fn() const noexcept { return this->identifier().is_fn(); }

		[[nodiscard]] bool operator==(const NodeContainer &rhs) const noexcept {
			return std::tie(node_identifier_, node_ptr_.void_ptr) == std::tie(rhs.node_identifier_, rhs.node_ptr_.void_ptr);
		}
		[[nodiscard]] bool operator!=(const NodeContainer &rhs) const noexcept {
			return !this->operator==(rhs);
		}

		/**
		 * Check if the NodeRepr (NodeHash/TaggedNodeHash) is empty.
		 */
		[[nodiscard]] bool empty() const noexcept { return node_identifier_.empty(); }
		/**
		 * Check if the the NodePtr is null. Node pointer is the pointer type used by the allocator defined in tri.
		 */
		[[nodiscard]] bool is_null_ptr() const noexcept { return node_ptr_ == VoidNodePtr{}; }

		template<ByteAllocator allocator_type2>
		operator RawNodeContainer<htt_t, allocator_type2>() const noexcept
				requires allocator_uses_raw_ptr// this must use std::allocator, allocator_type2 can be some other allocator
		{
			return RawNodeContainer<htt_t, allocator_type2>{this->identifier(), this->node_ptr_};
		}

//		// TODO: can we merge with the converter above?
		operator RawNodeContainer<htt_t, allocator_type>() const noexcept
				requires (not allocator_uses_raw_ptr)// this must not use std::allocator, allocator_type2 can be some other allocator
		{
			return RawNodeContainer<htt_t, allocator_type>{this->identifier(), this->node_ptr_};
		}
	};


	template<size_t depth, HypertrieTrait htt_t, template<size_t, typename, typename> typename node_type_t, ByteAllocator allocator_type>
	struct SpecificNodeContainer : public NodeContainer<depth, htt_t, allocator_type> {
	private:
		using ht_allocator_trait = hypertrie_allocator_trait<allocator_type>;

	public:
		//	template<size_t depth, HypertrieTrait tri_t, template<size_t, typename> typename node_type_t>
		//	struct SpecificNodeContainer : public NodeContainer<depth, tri_t> {
		using NodeContainer_t = NodeContainer<depth, htt_t, allocator_type>;
		using RawNodeContainer_t = typename NodeContainer_t::RawNodeContainer_t;

		using Identifier_t = typename NodeContainer_t::Identifier_t;
		using RawIdentifier_t = typename NodeContainer_t::RawIdentifier_t;
		using Node = node_type_t<depth, htt_t, allocator_type>;
		using NodePtr = typename ht_allocator_trait::template pointer<Node>;
		using VoidNodePtr = typename NodeContainer_t::VoidNodePtr;
		static constexpr bool allocator_uses_raw_ptr = std::is_same_v<VoidNodePtr, void *>;


		friend NodeContainer<depth, htt_t, allocator_type>;

	protected:
		SpecificNodeContainer(Identifier<htt_t> node_identifier, size_t node_ptr) noexcept : NodeContainer_t(node_identifier, node_ptr) {}

	private:
		static auto extract_and_cast_node_ptr(RawNodeContainer<htt_t, allocator_type> const &raw_node_container) noexcept {
			if constexpr (allocator_uses_raw_ptr) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				return reinterpret_cast<NodePtr>(raw_node_container.void_node_ptr());
			} else {
				// fancy pointers are cast by means of a copy constructor
				return NodePtr{raw_node_container.void_node_ptr()};
			}
		}

	public:
		SpecificNodeContainer() = default;

		SpecificNodeContainer(RawNodeContainer<htt_t, allocator_type> const &raw_node_container) noexcept
			: NodeContainer_t{raw_node_container.identifier(), extract_and_cast_node_ptr(raw_node_container)} {}

		template<ByteAllocator allocator_type2>
		SpecificNodeContainer(RawNodeContainer<htt_t, allocator_type2> const &raw_node_container) noexcept
				requires(hypertrie_allocator_trait<allocator_type>::is_stl_alloc and  // tri uses std::allocator
						 not hypertrie_allocator_trait<allocator_type2>::is_stl_alloc)// tri2 does NOT use std::allocator
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			: NodeContainer_t{RawIdentifier<depth, htt_t>(raw_node_container.identifier()), reinterpret_cast<NodePtr>(raw_node_container.raw_void_node_ptr())} {}

		SpecificNodeContainer(NodeContainer<depth, htt_t, allocator_type> const &node_container) noexcept
			: NodeContainer_t{node_container} {}

		explicit SpecificNodeContainer(RawIdentifier_t raw_identifier, NodePtr node_ptr = {}) noexcept
			: NodeContainer_t{raw_identifier, node_ptr} {}

		[[nodiscard]] constexpr NodePtr node_ptr() const noexcept { return this->template specific_ptr<node_type_t>(); }
		constexpr void node_ptr(NodePtr node_ptr) noexcept {
			this->node_ptr_ = node_ptr;
		}

		operator NodeContainer_t() const noexcept { return {this->identifier(), this->node_ptr()}; }
	};

	template<size_t depth, HypertrieTrait tri, ByteAllocator allocator_type>
	using FNContainer = SpecificNodeContainer<depth, tri, FullNode, allocator_type>;
	template<size_t depth, HypertrieTrait tri, ByteAllocator allocator_type>
	using SENContainer = SpecificNodeContainer<depth, tri, SingleEntryNode, allocator_type>;

};// namespace dice::hypertrie::internal::raw


#endif//HYPERTRIE_NODECONTAINER_HPP
