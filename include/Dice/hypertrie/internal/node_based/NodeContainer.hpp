#ifndef HYPERTRIE_NODECONTAINER_HPP
#define HYPERTRIE_NODECONTAINER_HPP

#include "Dice/hypertrie/internal/node_based/Node.hpp"
#include "Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp"

namespace hypertrie::internal::node_based {

	template<size_t depth, NodeCompression compressed, HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	struct SpecificNodeContainer;

	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	using CompressedNodeContainer = SpecificNodeContainer<depth, NodeCompression::compressed, tri_t>;

	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	using UncompressedNodeContainer = SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t>;

	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	typedef union NodePtr {
		NodePtr() noexcept : raw(nullptr){}
		NodePtr(const NodePtr &node_ptr) noexcept : raw(node_ptr.raw){}
		NodePtr(NodePtr &&node_ptr) noexcept : raw(node_ptr.raw){}
		NodePtr(void *raw) noexcept : raw(raw){}
		NodePtr(UncompressedNode<depth, tri_t> *uncompressed) noexcept : uncompressed(uncompressed){}
		NodePtr(CompressedNode<depth, tri_t> *compressed) noexcept : compressed(compressed){}
		void *raw;
		UncompressedNode<depth, tri_t> *uncompressed;
		CompressedNode<depth, tri_t> *compressed;

		operator CompressedNode<depth, tri_t> *() const noexcept { return this->compressed; }
		operator UncompressedNode<depth, tri_t> *() const noexcept { return this->uncompressed; }

		NodePtr &operator=(const NodePtr &node_ptr) noexcept {
			raw = node_ptr.raw;
			return *this;
		}

		NodePtr &operator=(NodePtr &&node_ptr) noexcept {
			raw = node_ptr.raw;
			return *this;
		}
	};

	template<size_t depth, HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	struct NodeContainer {
	protected:
		TaggedNodeHash hash_{};
		NodePtr<depth, tri_t> node_ptr_{};

	public:
		NodeContainer() noexcept {}

		NodeContainer(const TaggedNodeHash &thash, NodePtr<depth, tri_t> node) noexcept : hash_(thash), node_ptr_(node) {}

		template<NodeCompression compressed>
		NodeContainer(const SpecificNodeContainer<depth, compressed, tri_t> &other) noexcept : hash_{other.hash_}, node_ptr_{other.node_ptr_} {}
		template<NodeCompression compressed>
		NodeContainer(SpecificNodeContainer<depth, compressed, tri_t> &&other) noexcept : hash_{other.hash_}, node_ptr_{other.node_ptr_} {}

		[[nodiscard]] auto * &compressed_node() noexcept { return node_ptr_.compressed; }
		[[nodiscard]] auto * compressed_node() const noexcept  { return node_ptr_.compressed; }

		[[nodiscard]] auto * &uncompressed_node() noexcept { return node_ptr_.uncompressed; }
		[[nodiscard]] auto * uncompressed_node() const noexcept  { return node_ptr_.uncompressed; }

		[[nodiscard]] auto * &node() noexcept { return node_ptr_.raw; }
		[[nodiscard]] auto * node() const noexcept  { return node_ptr_.raw; }

		template<NodeCompression compression>
		[[nodiscard]] auto specific() const noexcept { return SpecificNodeContainer<depth, compression, tri_t>{hash_, node_ptr_}; }

		[[nodiscard]] auto compressed() const noexcept { return specific<NodeCompression::compressed>(); }

		[[nodiscard]] auto uncompressed() const noexcept { return specific<NodeCompression::uncompressed>(); }

		[[nodiscard]] bool isCompressed() const noexcept { return hash_.isCompressed(); }

		[[nodiscard]] bool isUncompressed() const noexcept { return hash_.isUncompressed(); }

		[[nodiscard]] bool empty() const noexcept { return hash_.empty(); }

		[[nodiscard]] bool null() const noexcept { return node_ptr_.raw == nullptr; }

		[[nodiscard]] TaggedNodeHash hash() const noexcept { return this->hash_; }

		[[nodiscard]] TaggedNodeHash &hash() noexcept { return this->hash_; }

		size_t &ref_count() {
			if (isCompressed())
				return compressed_node()->ref_count();
			else
				return uncompressed_node()->ref_count();
		}

		operator TaggedNodeHash() const noexcept  { return this->hash_; }

		friend struct SpecificNodeContainer<depth, NodeCompression::compressed, tri_t>;
		friend struct SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t>;
	};

	template<NodeCompression compression, HypertrieInternalTrait tri>
	auto getValue(SpecificNodeContainer<1, compression, tri> &nodec, typename tri::key_part_type key_part) noexcept
			-> typename tri::value_type {
		if constexpr (compression == NodeCompression::compressed) {
			if (key_part == nodec.node()->key()[0]) {
				if constexpr (tri::is_bool_valued)
					return true;
				else
					return nodec.node()->value();
			}
		} else {
			return nodec.node()->child(0, key_part);
		}
	}

	template<size_t depth, HypertrieInternalTrait tri_t>
	struct SpecificNodeContainer<depth, NodeCompression::compressed, tri_t> : public NodeContainer<depth, tri_t> {

		SpecificNodeContainer() noexcept : NodeContainer<depth, tri_t>{} {}

		SpecificNodeContainer(const SpecificNodeContainer & nodec) noexcept : NodeContainer<depth, tri_t>{nodec} {}

		SpecificNodeContainer(SpecificNodeContainer && nodec) noexcept : NodeContainer<depth, tri_t>{nodec} {}

		SpecificNodeContainer(const TaggedNodeHash &thash, NodePtr<depth, tri_t> node) noexcept : NodeContainer<depth, tri_t>{thash, node} {}

		SpecificNodeContainer& operator=(const SpecificNodeContainer &nodec) =default;

		SpecificNodeContainer& operator=(SpecificNodeContainer &&nodec) =default;


		[[nodiscard]] constexpr auto node() noexcept { return this->node_ptr_.compressed; }

		[[nodiscard]] constexpr auto node() const noexcept  { return this->node_ptr_.compressed; }

		[[nodiscard]] bool isCompressed() const noexcept  { return true; }

		[[nodiscard]] bool isUncompressed() const noexcept  { return false; }

		operator NodeContainer<depth, tri_t>() const noexcept { return {this->hash_, this->node_ptr_}; }
		operator NodeContainer<depth, tri_t> &&() const noexcept { return {this->hash_, this->node_ptr_}; }
	};


	template<size_t depth,
			 HypertrieInternalTrait tri_t>
	struct SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t> : public NodeContainer<depth, tri_t> {

		SpecificNodeContainer() noexcept : NodeContainer<depth, tri_t>{} {}

		SpecificNodeContainer(const SpecificNodeContainer & nodec) noexcept : NodeContainer<depth, tri_t>{nodec} {}

		SpecificNodeContainer(SpecificNodeContainer && nodec) noexcept : NodeContainer<depth, tri_t>{nodec} {}

		SpecificNodeContainer(const TaggedNodeHash &thash, NodePtr<depth, tri_t> node) noexcept : NodeContainer<depth, tri_t>{thash, node} {}

		SpecificNodeContainer& operator=(const SpecificNodeContainer &nodec) =default;

		SpecificNodeContainer& operator=(SpecificNodeContainer &&nodec) =default;

		[[nodiscard]] constexpr auto node() noexcept { return this->node_ptr_.uncompressed; }

		[[nodiscard]] constexpr auto node() const noexcept { return this->node_ptr_.uncompressed; }

		[[nodiscard]] bool isCompressed() const noexcept { return false; }

		[[nodiscard]] bool isUncompressed() const noexcept { return true; }

		inline auto getChildHashOrValue(size_t pos, typename tri_t::key_part_type key_part)
				-> std::conditional_t<(depth > 1), TaggedNodeHash, typename tri_t::value_type> {
			assert(pos < depth);
			return this->node()->child(pos, key_part);
		}

		operator NodeContainer<depth, tri_t>() const noexcept { return {this->hash_, this->node_ptr_}; }
		operator NodeContainer<depth, tri_t> &&() const noexcept { return {this->hash_, this->node_ptr_}; }
	};// namespace hypertrie::internal::node_based


}// namespace hypertrie::internal::node_based
#endif//HYPERTRIE_NODECONTAINER_HPP
