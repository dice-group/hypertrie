#ifndef HYPERTRIE_NODESTORAGE_HPP
#define HYPERTRIE_NODESTORAGE_HPP

#include "Dice/hypertrie/internal/node_based/Hypertrie_internal_traits.hpp"
#include "Dice/hypertrie/internal/node_based/Node.hpp"
#include "Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"

namespace hypertrie::internal::node_based {

	template<size_t depth,
			 typename tri_t = Hypertrie_internal_t<>,
			 typename = typename std::enable_if_t<(depth >= 1)>>
	struct NodeStorage {
		using tri = tri_t;
		template<typename K, typename V>
		using map_type = typename tri::template map_type<K, V>;
		using CompressedNodeMap = map_type<TaggedNodeHash, CompressedNode<depth, tri>>;
		using UncompressedNodeMap = map_type<TaggedNodeHash, UncompressedNode<depth, tri>>;

	protected:
		CompressedNodeMap compressed_nodes_;
		UncompressedNodeMap uncompressed_nodes_;

	public:
		const CompressedNodeMap &compressedNodes() const { return this->compressed_nodes_; }
		CompressedNodeMap &compressedNodes() { return this->compressed_nodes_; }

		const UncompressedNodeMap &uncompressedNodes() const { return this->uncompressed_nodes_; }
		UncompressedNodeMap &uncompressedNodes() { return this->uncompressed_nodes_; }
	};
}// namespace hypertrie::internal::node_based

#endif//HYPERTRIE_NODESTORAGE_HPP
