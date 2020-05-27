#ifndef HYPERTRIE_NODECONTAINER_HPP
#define HYPERTRIE_NODECONTAINER_HPP

#include "Dice/hypertrie/internal/node_based/Node.hpp"
#include "Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp"

namespace hypertrie::internal::node_based {

	template<pos_type depth, NodeCompression compressed, typename tri_t = Hypertrie_internal_t<>>
	struct SpecificNodeContainer;

	template<pos_type depth,
			 typename tri_t = Hypertrie_internal_t<>>
	using CompressedNodeContainer = SpecificNodeContainer<depth, NodeCompression::compressed, tri_t>;

	template<pos_type depth,
			 typename tri_t = Hypertrie_internal_t<>>
	using UncompressedNodeContainer = SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t>;

	template<pos_type depth, typename tri_t = Hypertrie_internal_t<>>
	struct NodeContainer {
		TaggedNodeHash thash_{};

	protected:
		void *node_ = nullptr;

	public:
		NodeContainer(const TaggedNodeHash &thash, void *node) : thash_(thash), node_(node) {}

		[[nodiscard]] auto compressed_node() {
			return static_cast<CompressedNode<depth, tri_t> *>(node_);
		}

		[[nodiscard]] auto uncompressed_node() {
			return static_cast<UncompressedNode<depth, tri_t> *>(node_);
		}

		template<NodeCompression compressed>
		[[nodiscard]] auto specific() {
			return SpecificNodeContainer<depth, compressed, tri_t>(thash_, node_);
		}

		[[nodiscard]] auto compressed() {
			return specific<NodeCompression::compressed>();
		}

		[[nodiscard]] auto uncompressed() {
			return specific<NodeCompression::uncompressed>();
		}

		[[nodiscard]] bool empty() const { return thash_ == TaggedNodeHash{}; }
	};

	template<pos_type depth, typename tri_t>
	struct SpecificNodeContainer<depth, NodeCompression::compressed, tri_t> : public NodeContainer<depth, tri_t> {

		SpecificNodeContainer(const TaggedNodeHash &thash, void *node) : NodeContainer<depth, tri_t>(thash, node) {}


		[[nodiscard]] auto node() {
			return static_cast<CompressedNode<depth, tri_t> *>(this->node_);
		}

		operator NodeContainer<depth, tri_t>() const { return {this->thash_, this->node_}; }
	};


	template<pos_type depth,
			 typename tri_t>
	struct SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t> : public NodeContainer<depth, tri_t> {

		SpecificNodeContainer(const TaggedNodeHash &thash, void *node) : NodeContainer<depth, tri_t>(thash, node) {}

		[[nodiscard]] auto node() {
			return static_cast<UncompressedNode<depth, tri_t> *>(this->node_);
		}

		operator NodeContainer<depth, tri_t>() const { return {this->thash_, this->node_}; }
	};


}// namespace hypertrie::internal::node_based
#endif//HYPERTRIE_NODECONTAINER_HPP
