#ifndef HYPERTRIE_NODECONTAINER_HPP
#define HYPERTRIE_NODECONTAINER_HPP

#include "Dice/hypertrie/internal/node_based/raw/node/Node.hpp"
#include "Dice/hypertrie/internal/node_based/raw/node/NodePtr.hpp"
#include "Dice/hypertrie/internal/node_based/raw/node/TensorHash.hpp"

namespace hypertrie::internal::node_based::raw {

	template<size_t depth, NodeCompression compressed, HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	struct SpecificNodeContainer;

	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	using CompressedNodeContainer = SpecificNodeContainer<depth, NodeCompression::compressed, tri_t>;

	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	using UncompressedNodeContainer = SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t>;

	/**
	 * A Node container holds a pointer to a Node and it's TensorHash. The container does not own the node i.e. destructing the NodeContainer does not destruct the Node.
	 * @tparam depth
	 * @tparam tri_t
	 */
	template<size_t depth, HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	struct NodeContainer {
		using tri = tri_t;
		/**
		 * Normally the node representation is just a TensorHash.
		 * However, if depth == 1 and tri::is_bool_valued and tri::is_lsb_unused, then the key_part is directly stored in a TaggedTensorHash.
		 */
		using NodeRepr = std::conditional_t<(not (depth == 1 and tri::is_bool_valued and tri::is_lsb_unused)), TensorHash, TaggedTensorHash<tri_t>>;
	protected:
		/**
		 * NodeHash (or TaggedNodeHash) that identifies (or represents) the Node.
		 */
		NodeRepr repr_{};
		/**
		 * Pointer to the Node.
		 */
		NodePtr<depth, tri_t> node_ptr_{};

	public:
		NodeContainer() noexcept {}

		NodeContainer(const NodeRepr &repr, NodePtr<depth, tri_t> node) noexcept : repr_(repr), node_ptr_(node) {}

		template<NodeCompression compressed>
		NodeContainer(const SpecificNodeContainer<depth, compressed, tri_t> &other) noexcept : repr_{other.repr_}, node_ptr_{other.node_ptr_} {}
		template<NodeCompression compressed>
		NodeContainer(SpecificNodeContainer<depth, compressed, tri_t> &&other) noexcept : repr_{other.repr_}, node_ptr_{other.node_ptr_} {}

		[[nodiscard]] auto * &compressed_node() noexcept { return node_ptr_.compressed; }
		[[nodiscard]] auto * compressed_node() const noexcept  { return node_ptr_.compressed; }

		[[nodiscard]] auto * &uncompressed_node() noexcept { return node_ptr_.uncompressed; }
		[[nodiscard]] auto * uncompressed_node() const noexcept  { return node_ptr_.uncompressed; }

		[[nodiscard]] auto * &node() noexcept { return node_ptr_.raw; }
		[[nodiscard]] auto * node() const noexcept  { return node_ptr_.raw; }

		template<NodeCompression compression>
		[[nodiscard]] auto specific() const noexcept { return SpecificNodeContainer<depth, compression, tri_t>{repr_, node_ptr_}; }

		[[nodiscard]] auto compressed() const noexcept { return specific<NodeCompression::compressed>(); }

		[[nodiscard]] auto uncompressed() const noexcept { return specific<NodeCompression::uncompressed>(); }

		[[nodiscard]] bool isCompressed() const noexcept { return repr_.isCompressed(); }

		[[nodiscard]] bool isUncompressed() const noexcept { return repr_.isUncompressed(); }

		/**
		 * Check if the NodeRepr (NodeHash/TaggedNodeHash) is empty.
		 */
		[[nodiscard]] bool empty() const noexcept { return repr_.empty(); }

		/**
		 * Check if the the NodePtr is empty.
		 */
		[[nodiscard]] bool null() const noexcept { return node_ptr_.raw == nullptr; }

		[[nodiscard]] NodeRepr hash() const noexcept { return this->repr_; }

		[[nodiscard]] NodeRepr &hash() noexcept { return this->repr_; }

		/**
		 * Get the reference Count of the contained Node.
		 */
		[[nodiscard]] size_t &ref_count() {
			if (isCompressed())
				return compressed_node()->ref_count();
			else
				return uncompressed_node()->ref_count();
		}

		operator NodeRepr() const noexcept  { return this->repr_; }

		friend struct SpecificNodeContainer<depth, NodeCompression::compressed, tri_t>;
		friend struct SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t>;
	};

	template<size_t depth, HypertrieInternalTrait tri_t>
	struct SpecificNodeContainer<depth, NodeCompression::compressed, tri_t> : public NodeContainer<depth, tri_t> {
		using NodeRepr = std::conditional_t<(not (depth == 1 and tri_t::is_bool_valued and tri_t::is_lsb_unused)), TensorHash, TaggedTensorHash<tri_t>>;

		SpecificNodeContainer() noexcept : NodeContainer<depth, tri_t>{} {}

		SpecificNodeContainer(const SpecificNodeContainer & nodec) noexcept : NodeContainer<depth, tri_t>{nodec} {}

		SpecificNodeContainer(SpecificNodeContainer && nodec) noexcept : NodeContainer<depth, tri_t>{nodec} {}

		SpecificNodeContainer(const NodeRepr &thash, NodePtr<depth, tri_t> node) noexcept : NodeContainer<depth, tri_t>{thash, node} {}

		SpecificNodeContainer& operator=(const SpecificNodeContainer &nodec) =default;

		SpecificNodeContainer& operator=(SpecificNodeContainer &&nodec) =default;


		[[nodiscard]] constexpr auto node() noexcept { return this->node_ptr_.compressed; }

		[[nodiscard]] constexpr auto node() const noexcept  { return this->node_ptr_.compressed; }

		[[nodiscard]] bool isCompressed() const noexcept  { return true; }

		[[nodiscard]] bool isUncompressed() const noexcept  { return false; }

		operator NodeContainer<depth, tri_t>() const noexcept { return {this->repr_, this->node_ptr_}; }
		operator NodeContainer<depth, tri_t> &&() const noexcept { return {this->repr_, this->node_ptr_}; }
	};


	template<size_t depth,
			 HypertrieInternalTrait tri_t>
	struct SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t> : public NodeContainer<depth, tri_t> {
		using NodeRepr = std::conditional_t<(not (depth == 1 and tri_t::is_bool_valued and tri_t::is_lsb_unused)), TensorHash, TaggedTensorHash<tri_t>>;

		SpecificNodeContainer() noexcept : NodeContainer<depth, tri_t>{} {}

		SpecificNodeContainer(const SpecificNodeContainer & nodec) noexcept : NodeContainer<depth, tri_t>{nodec} {}

		SpecificNodeContainer(SpecificNodeContainer && nodec) noexcept : NodeContainer<depth, tri_t>{nodec} {}

		SpecificNodeContainer(const NodeRepr &thash, NodePtr<depth, tri_t> node) noexcept : NodeContainer<depth, tri_t>{thash, node} {}

		SpecificNodeContainer& operator=(const SpecificNodeContainer &nodec) =default;

		SpecificNodeContainer& operator=(SpecificNodeContainer &&nodec) =default;

		[[nodiscard]] constexpr auto node() noexcept { return this->node_ptr_.uncompressed; }

		[[nodiscard]] constexpr auto node() const noexcept { return this->node_ptr_.uncompressed; }

		[[nodiscard]] bool isCompressed() const noexcept { return false; }

		[[nodiscard]] bool isUncompressed() const noexcept { return true; }

		inline auto getChildHashOrValue(size_t pos, typename tri_t::key_part_type key_part)
				-> std::conditional_t<(depth > 1), typename Node<depth, NodeCompression::uncompressed, tri_t>::ChildType, typename tri_t::value_type> {
			assert(pos < depth);
			return this->node()->child(pos, key_part);
		}

		operator NodeContainer<depth, tri_t>() const noexcept { return {this->repr_, this->node_ptr_}; }
		operator NodeContainer<depth, tri_t> &&() const noexcept { return {this->repr_, this->node_ptr_}; }
	};// namespace hypertrie::internal::node_based


}// namespace hypertrie::internal::node_based
#endif//HYPERTRIE_NODECONTAINER_HPP
