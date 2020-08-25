#ifndef HYPERTRIE_ITERATIONNODECONTAINER_HPP
#define HYPERTRIE_ITERATIONNODECONTAINER_HPP

#include "Dice/hypertrie/internal/node_based/raw/node/NodeContainer.hpp"
#include "Dice/hypertrie/internal/node_based/raw/Hypertrie_internal_traits.hpp"

namespace hypertrie::internal::node_based::raw {

	/**
	 * Container for results of Diagonal and Iterator.
	 * @tparam result_depth depth of the result tensor
	 * @tparam tri_t
	 */
	template<size_t result_depth, HypertrieInternalTrait tri>
	struct IterationNodeContainer {
		/**
		 * NodeContainer containing an result if there is any.
		 * If it is not empty and not an compressed depth 1 lsb-unused node, it is safe to use the node.
		 * IMPORTANT: if is_managed is false you are responsible to delete the pointed node. Otherwise, the reference to the allocated memory will be lost.
		 */
		NodeContainer<result_depth, tri> nodec{};
		/**
		 * If the memory for the Node from NodeContainer is managed elsewhere or if the user is responsible to clean it up.
		 */
		bool is_managed = false;
	};

}
#endif//HYPERTRIE_ITERATIONNODECONTAINER_HPP
