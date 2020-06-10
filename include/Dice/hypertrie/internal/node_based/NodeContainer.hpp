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

	template<size_t depth, HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	struct NodeContainer {
		TaggedNodeHash thash_{};

	protected:
		void *node_ = nullptr;

	public:
		NodeContainer() {}

		NodeContainer(const TaggedNodeHash &thash, void *node) : thash_(thash), node_(node) {}

		template<NodeCompression compressed>
		NodeContainer(const SpecificNodeContainer<depth, compressed, tri_t> &other) : thash_{other.thash_}, node_{other.node_} {}
		template<NodeCompression compressed>
		NodeContainer(SpecificNodeContainer<depth, compressed, tri_t> &&other) : thash_{other.thash_}, node_{other.node_} {}

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

		[[nodiscard]] bool null() const { return node_ == nullptr; }

		operator TaggedNodeHash() const {
			return this->thash_;
		}

		friend struct SpecificNodeContainer<depth, NodeCompression::compressed, tri_t>;
		friend struct SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t>;
	};

	template<NodeCompression compression, HypertrieInternalTrait tri>
	auto getValue(SpecificNodeContainer<1, compression, tri> &nodec, typename tri::key_part_type key_part)
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

		SpecificNodeContainer() : NodeContainer<depth, tri_t>{} {}

		SpecificNodeContainer(const TaggedNodeHash &thash, void *node) : NodeContainer<depth, tri_t>(thash, node) {}


		[[nodiscard]] auto node() {
			return static_cast<CompressedNode<depth, tri_t> *>(this->node_);
		}

		operator NodeContainer<depth, tri_t>() const { return {this->thash_, this->node_}; }
		operator NodeContainer<depth, tri_t> &&() const { return {this->thash_, this->node_}; }
	};


	template<size_t depth,
			 HypertrieInternalTrait tri_t>
	struct SpecificNodeContainer<depth, NodeCompression::uncompressed, tri_t> : public NodeContainer<depth, tri_t> {

		SpecificNodeContainer() : NodeContainer<depth, tri_t>{} {}

		SpecificNodeContainer(const TaggedNodeHash &thash, void *node) : NodeContainer<depth, tri_t>(thash, node) {}

		[[nodiscard]] auto node() {
			return static_cast<UncompressedNode<depth, tri_t> *>(this->node_);
		}

		inline auto getChildHashOrValue(size_t pos, typename tri_t::key_part_type key_part)
				-> std::conditional_t<(depth > 1), TaggedNodeHash, typename tri_t::value_type> {
			assert(pos < depth);
			return this->node()->child(pos, key_part);
		}

		operator NodeContainer<depth, tri_t>() const { return {this->thash_, this->node_}; }
		operator NodeContainer<depth, tri_t> &&() const { return {this->thash_, this->node_}; }
	};// namespace hypertrie::internal::node_based


}// namespace hypertrie::internal::node_based
#endif//HYPERTRIE_NODECONTAINER_HPP
