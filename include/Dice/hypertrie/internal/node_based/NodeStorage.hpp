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
		using Node = Node<depth, true, tri>;

		typename tri::template map_type<NodeHash, CompressedNode> compressed_nodes_;
		typename tri::template map_type<NodeHash, Node> uncompressed_nodes_;
	};
}

#endif //HYPERTRIE_NODESTORAGE_HPP
