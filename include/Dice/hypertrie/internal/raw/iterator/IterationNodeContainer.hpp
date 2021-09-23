#ifndef HYPERTRIE_ITERATIONNODECONTAINER_HPP
#define HYPERTRIE_ITERATIONNODECONTAINER_HPP

#include "Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp"
#include "Dice/hypertrie/internal/raw/node/NodeContainer.hpp"

namespace hypertrie::internal::raw {

	/**
	 * Container for results of Diagonal and Iterator.
	 * @tparam result_depth depth of the result tensor
	 * @tparam tri_t
	 */
	// TODO: rename to SliceContainer
	template<size_t result_depth, HypertrieInternalTrait tri_t>
	class IterationNodeContainer {
	public:
		using tri = tri_t;
		using std_tri = typename tri::with_std_allocator;

	private: /**
* NodeContainer containing an result if there is any.
* If it is not empty and not an compressed depth 1 lsb-unused node, it is safe to use the node.
* IMPORTANT: if is_managed is false you are responsible to delete the pointed node. Otherwise, the reference to the allocated memory will be lost.
*/
		using NodeRepr = typename NodeContainer<result_depth, tri>::NodeRepr;

		static constexpr bool tri_uses_std_alloc = std::is_same_v<tri, std_tri>;

		NodeRepr identifier_{};

		union {
			NodePtr<result_depth, tri> tri_alloc_ptr{};
			NodePtr<result_depth, std_tri> std_alloc_ptr;
		} node;

		bool is_managed_ = false;

		bool std_alloc_ = tri_uses_std_alloc;

	public:
		IterationNodeContainer() = default;

		static inline IterationNodeContainer make_managed_tri_alloc(NodeRepr identifier, NodePtr<result_depth, tri> managed_node_ptr = {}) {
			return {.identifier_ = identifier, .node = managed_node_ptr, .is_managed_ = true, .std_alloc_ = false, .std_alloc_ = tri_uses_std_alloc};
		}

		static inline IterationNodeContainer make_managed_tri_alloc(const NodeContainer<result_depth, tri> &node_container) {
			return {.identifier_ = node_container.hash(), .node = node_container.node(), .is_managed_ = true, .std_alloc_ = tri_uses_std_alloc};
		}

		static inline IterationNodeContainer make_managed_std_alloc(const NodeContainer<result_depth, std_tri> &node_container) {
			return {.identifier_ = node_container.hash(), .node = node_container.node(), .is_managed_ = true, .std_alloc_ = true};
		}

		static inline IterationNodeContainer make_unmanaged_std_alloc(NodeRepr identifier, NodePtr<result_depth, std_tri> unmanaged_node_ptr = {}) {
			return {.identifier_ = identifier, .node = unmanaged_node_ptr, .is_managed_ = false, .std_alloc_ = true};
		}

		static inline IterationNodeContainer make_managed_std_alloc(NodeRepr identifier, NodePtr<result_depth, std_tri> unmanaged_node_ptr = {}) {
			return {.identifier_ = identifier, .node = unmanaged_node_ptr, .is_managed_ = true, .std_alloc_ = true};
		}

		const NodeRepr &identifier() { return identifier_; }

		NodeRepr identifier() const { return identifier_; }

		bool is_managed() const { return is_managed_; }
		bool &is_managed() { return is_managed_; }

		bool uses_std_alloc() const {
			return std_alloc_;
		}

		NodePtr<result_depth, tri> tri_alloc_node_ptr() const {
			assert(not uses_std_alloc());
			return node.tri_alloc_ptr;
		}

		NodePtr<result_depth, tri> &tri_alloc_node_ptr() {
			assert(not uses_std_alloc());
			return node.tri_alloc_ptr;
		}

		NodeContainer<result_depth, tri> tri_alloc_node_container() const {
			assert(not uses_std_alloc());
			return {identifier_, node.tri_alloc_ptr};
		}

		NodePtr<result_depth, std_tri> std_alloc_node_ptr() const {
			assert(uses_std_alloc());
			return node.std_alloc_ptr;
		}

		NodePtr<result_depth, std_tri> &std_alloc_node_ptr() {
			assert(uses_std_alloc());
			return node.std_alloc_ptr;
		}

		bool empty() {
			if (uses_std_alloc())
				return tri_alloc_node_container().empty();
			else
				return NodeContainer<result_depth, tri>{identifier_, node.tri_alloc_ptr}.empty();
		}


		/**
		 * If the memory for the Node from NodeContainer is managed elsewhere or if the user is responsible to clean it up.
		 */
	};

}// namespace hypertrie::internal::raw
#endif//HYPERTRIE_ITERATIONNODECONTAINER_HPP
