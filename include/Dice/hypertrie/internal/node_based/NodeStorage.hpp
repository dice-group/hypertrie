#ifndef HYPERTRIE_NODESTORAGE_HPP
#define HYPERTRIE_NODESTORAGE_HPP

#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/node_based/Hypertrie_internal_traits.hpp"
#include "Dice/hypertrie/internal/node_based/Node.hpp"
#include "TaggedNodeHash.hpp"

namespace hypertrie::internal::node_based {

	template<pos_type depth,
			typename tri_t = Hypertrie_internal_t<>,
			typename  = typename std::enable_if_t<(depth >= 1)>>
	struct NodeStorage {
		using tri = tri_t;
		using CompressedNode = Node<depth, true, tri>;
		using UncompressedNode = Node<depth, true, tri>;
		using CompressedNodeMap = typename tri::template map_type<TaggedNodeHash, CompressedNode>;
		using UncompressedNodeMap = typename tri::template map_type<TaggedNodeHash, UncompressedNode>;


		CompressedNodeMap compressed_nodes_;
		UncompressedNodeMap uncompressed_nodes_;
	};
}

#endif //HYPERTRIE_NODESTORAGE_HPP
