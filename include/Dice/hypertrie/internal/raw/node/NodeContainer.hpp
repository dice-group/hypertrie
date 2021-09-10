#ifndef HYPERTRIE_NODECONTAINER_HPP
#define HYPERTRIE_NODECONTAINER_HPP

#include "Dice/hypertrie/internal/raw/node/Node.hpp"
#include "Dice/hypertrie/internal/raw/node/NodePtr.hpp"
#include "Dice/hypertrie/internal/raw/node/TensorHash.hpp"
#include "Dice/hypertrie/internal/util/UnsafeCast.hpp"

namespace hypertrie::internal::raw {


	/**
	 * Parent type to NodeContainer, SpecificNodeContainer (and its aliases CompressedNodeContainer and UncompressedNodeContainer).
	 * This class defines the memory layout for the inheriting classes. Inheriting classes must not add any further fields.
	 *
	 * <p>Note: this class was not templated before.</p>
	 */
	template<typename Allocator = std::allocator<size_t>>
	struct RawNodeContainer {
		using alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<size_t>;

	protected:
		using void_ptr_type = typename std::allocator_traits<alloc>::void_pointer;

		size_t node_identifier;

		void_ptr_type node_ptr;

	public:
		RawNodeContainer() = default;
		RawNodeContainer(size_t nodeIdentifier, void_ptr_type nodePtr) : node_identifier(nodeIdentifier), node_ptr(nodePtr) {}
	};

	template<size_t depth,
			 NodeCompression compressed,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	struct SpecificNodeContainer;

	/**
	 * A Node container holds a pointer to a Node and it's TensorHash. The container does not own the node i.e. destructing the NodeContainer does not destruct the Node.
	 * @tparam depth
	 * @tparam tri_t
	 */
	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	struct NodeContainer : public RawNodeContainer<typename tri_t::allocator_type> {
		using RawNodeContainer_t = RawNodeContainer<typename tri_t::allocator_type>;
		using tri = tri_t;
		using alloc = typename RawNodeContainer_t::alloc;

		using NodeRepr = std::conditional_t<(not(depth == 1 and tri::is_bool_valued and tri::is_lsb_unused)), TensorHash, TaggedTensorHash<tri_t>>;
		using NodePtr_t = NodePtr<depth, tri>;
		using CompressedNodePtr = typename NodePtr_t::compressed_ptr_type;
		using UncompressedNodePtr = typename NodePtr_t::uncompressed_ptr_type;


	public:
		NodeContainer() = default;

		NodeContainer(const NodeRepr &repr, NodePtr_t node) noexcept : RawNodeContainer_t{(size_t) repr, node.raw} {}

		template<NodeCompression compressed>
		NodeContainer(const SpecificNodeContainer<depth, compressed, tri_t> &other) noexcept : RawNodeContainer_t{other.node_identifier, other.node_ptr} {}
		template<NodeCompression compressed>
		NodeContainer(SpecificNodeContainer<depth, compressed, tri_t> &&other) noexcept : RawNodeContainer_t{other.node_identifier, other.node_ptr} {}

		[[nodiscard]] NodePtr_t &node() noexcept { return util::unsafe_cast<NodePtr_t>(this->node_ptr); }
		[[nodiscard]] NodePtr_t node() const noexcept { return util::unsafe_cast<NodePtr_t const>(this->node_ptr); }

		[[nodiscard]] CompressedNodePtr &compressed_node() noexcept { return node().compressed; }
		[[nodiscard]] CompressedNodePtr compressed_node() const noexcept { return node().compressed; }

		[[nodiscard]] UncompressedNodePtr &uncompressed_node() noexcept { return node().uncompressed; }
		[[nodiscard]] UncompressedNodePtr uncompressed_node() const noexcept { return node().uncompressed; }


		template<NodeCompression compression>
		[[nodiscard]] auto &specific_node() noexcept {
			if constexpr (compression == NodeCompression::uncompressed) return uncompressed_node();
			else
				return compressed_node();
		}
		template<NodeCompression compression>
		[[nodiscard]] auto specific_node() const noexcept {
			if constexpr (compression == NodeCompression::uncompressed) return uncompressed_node();
			else
				return compressed_node();
		}

		template<NodeCompression compression>
		[[nodiscard]] auto specific() const noexcept { return static_cast<SpecificNodeContainer<depth, compression, tri_t>>(*this); }

		[[nodiscard]] auto compressed() const noexcept { return specific<NodeCompression::compressed>(); }

		[[nodiscard]] auto uncompressed() const noexcept { return specific<NodeCompression::uncompressed>(); }

		[[nodiscard]] NodeRepr hash() const noexcept { return util::unsafe_cast<NodeRepr const>(this->node_identifier); }

		[[nodiscard]] NodeRepr &hash() noexcept { return util::unsafe_cast<NodeRepr>(this->node_identifier); }

		[[nodiscard]] bool isCompressed() const noexcept { return hash().isCompressed(); }

		[[nodiscard]] bool isUncompressed() const noexcept { return hash().isUncompressed(); }

		/**
		 * Check if the NodeRepr (NodeHash/TaggedNodeHash) is empty.
		 */
		[[nodiscard]] bool empty() const noexcept { return hash().empty(); }

		/**
		 * Check if the the NodePtr is empty.
		 * TODO: might not always work
		 */
		[[nodiscard]] bool null() const noexcept { return node() == nullptr; }

		/**
		 * Get the reference Count of the contained Node.
		 */
		[[nodiscard]] size_t &ref_count() {
			if (isCompressed())
				return compressed_node()->ref_count();
			else
				return uncompressed_node()->ref_count();
		}

		operator NodeRepr() const noexcept { return this->repr_; }
	};

	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	using CompressedNodeContainer = SpecificNodeContainer<depth, NodeCompression::compressed, tri_t>;

	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	using UncompressedNodeContainer = SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t>;

	template<size_t depth, NodeCompression compression, HypertrieInternalTrait tri_t>
	struct SpecificNodeContainer : public NodeContainer<depth, tri_t> {
		using tri = tri_t;
		using NodeContainer_t = NodeContainer<depth, tri>;
		using RawNodeContainer_t = typename NodeContainer_t::RawNodeContainer_t;
		using alloc = typename RawNodeContainer_t::alloc;

		using NodeRepr = typename NodeContainer_t::NodeRepr;
		using NodePtr_t = typename NodeContainer_t::NodePtr_t;

		using SpecificNodePtr = std::conditional_t<compression == NodeCompression::compressed,
												   typename NodePtr_t::compressed_ptr_type,
												   typename NodePtr_t::uncompressed_ptr_type>;

		SpecificNodeContainer() = default;

		SpecificNodeContainer(const SpecificNodeContainer &nodec) noexcept : NodeContainer_t{nodec} {}

		SpecificNodeContainer(SpecificNodeContainer &&nodec) noexcept : NodeContainer_t{nodec} {}

		SpecificNodeContainer(const NodeRepr &thash, NodePtr_t node) noexcept : NodeContainer_t{thash, node} {}

		SpecificNodeContainer &operator=(const SpecificNodeContainer &nodec) = default;

		SpecificNodeContainer &operator=(SpecificNodeContainer &&nodec) = default;


		[[nodiscard]] constexpr SpecificNodePtr node() noexcept { return this->template specific_node<compression>(); }

		[[nodiscard]] constexpr SpecificNodePtr node() const noexcept { return this->template specific_node<compression>(); }

		inline auto getChildHashOrValue(size_t pos, typename tri_t::key_part_type key_part) {
			static_assert(compression == NodeCompression::uncompressed, "Function getChildHashOrValue is only applicable for uncompressed nodes.");

			assert(pos < depth);
			return this->uncompressed_node()->child(pos, key_part);
		}

		[[nodiscard]] bool isCompressed() const noexcept { return compression == NodeCompression::compressed; }

		[[nodiscard]] bool isUncompressed() const noexcept { return compression == NodeCompression::uncompressed; }

		operator NodeContainer_t() const noexcept { return *this; }
		operator NodeContainer_t &&() const noexcept { return *this; }
	};

};// namespace hypertrie::internal::raw


#endif//HYPERTRIE_NODECONTAINER_HPP
