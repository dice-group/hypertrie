#ifndef HYPERTRIE_NODEPTR_HPP
#define HYPERTRIE_NODEPTR_HPP

#include "Dice/hypertrie/internal/raw/node/Node.hpp"

namespace hypertrie::internal::raw {
	/**
	 * NodePtr is a union class of void, CompressedNode and UncompressedNode pointers.
	 * @tparam depth
	 * @tparam tri_t
	 */
	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			 typename Allocator = std::allocator<size_t>>
	union NodePtr {
	public:
		using tri = tri_t;

		using alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<size_t>;
		using uncompressed_alloc = typename std::allocator_traits<alloc>::template rebind_alloc<UncompressedNode<depth, tri, alloc>>;
		using compressed_alloc = typename std::allocator_traits<alloc>::template rebind_alloc<CompressedNode<depth, tri, alloc>>;

		using void_ptr_type = typename std::allocator_traits<alloc>::void_pointer;
		using uncompressed_ptr_type = typename std::allocator_traits<uncompressed_alloc>::pointer;
		using compressed_ptr_type = typename std::allocator_traits<compressed_alloc>::pointer;

		void_ptr_type raw;                 //!< void pointer
		uncompressed_ptr_type uncompressed;//!< uncompressed node pointer
		compressed_ptr_type compressed;    //!< compressed node pointer

		/// Constructors
		NodePtr() noexcept : raw(nullptr) {}
		NodePtr(const NodePtr &node_ptr) noexcept : raw(node_ptr.raw) {}
		NodePtr(NodePtr &&node_ptr) noexcept : raw(node_ptr.raw) {}

		NodePtr(void_ptr_type raw) noexcept : raw(raw) {}
		NodePtr(uncompressed_ptr_type uncompressed) noexcept : uncompressed(uncompressed) {}
		NodePtr(compressed_ptr_type compressed) noexcept : compressed(compressed) {}

		/**
		 * Cast to CompressedNode ptr
		 */
		operator compressed_ptr_type() const noexcept { return this->compressed; }

		/**
		 * Cast to UncompressedNode Ptr
		 */
		operator uncompressed_ptr_type() const noexcept { return this->uncompressed; }

		NodePtr &operator=(const NodePtr &node_ptr) noexcept {
			raw = node_ptr.raw;
			return *this;
		}

		NodePtr &operator=(NodePtr &&node_ptr) noexcept {
			raw = node_ptr.raw;
			return *this;
		}
	};
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_NODEPTR_HPP
