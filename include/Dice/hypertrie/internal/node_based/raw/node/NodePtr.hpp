#ifndef HYPERTRIE_NODEPTR_HPP
#define HYPERTRIE_NODEPTR_HPP

#include "Dice/hypertrie/internal/node_based/raw/node/Node.hpp"

namespace hypertrie::internal::node_based {
	/**
	 * NodePtr is a union class of CompressedNode* and UncompressedNode*.
	 * @tparam depth
	 * @tparam tri_t
	 */
	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	union NodePtr {
		/// stored pointer
		void *raw;
		UncompressedNode<depth, tri_t> *uncompressed;
		CompressedNode<depth, tri_t> *compressed;

		/// Constructors
		NodePtr() noexcept : raw(nullptr) {}
		NodePtr(const NodePtr &node_ptr) noexcept : raw(node_ptr.raw) {}
		NodePtr(NodePtr &&node_ptr) noexcept : raw(node_ptr.raw) {}
		NodePtr(void *raw) noexcept : raw(raw) {}
		NodePtr(UncompressedNode<depth, tri_t> *uncompressed) noexcept : uncompressed(uncompressed) {}
		NodePtr(CompressedNode<depth, tri_t> *compressed) noexcept : compressed(compressed) {}


		/**
	 * Get CompressedNode pointer
	 */
		operator CompressedNode<depth, tri_t> *() const noexcept { return this->compressed; }
		/**
	 * Get UncompressedNode pointer
	 */
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
}// namespace hypertrie::internal::node_based

#endif//HYPERTRIE_NODEPTR_HPP
