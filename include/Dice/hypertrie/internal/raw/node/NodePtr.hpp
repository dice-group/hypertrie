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
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	union NodePtr {
	public:
		using tri = tri_t;

		using allocator_type = typename tri::allocator_type;
		template<NodeCompression compression>
		using specific_alloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<Node<depth, compression, tri>>;
		using uncompressed_alloc = specific_alloc<NodeCompression::uncompressed>;
		using compressed_alloc = specific_alloc<NodeCompression::compressed>;

		using void_ptr_type = typename std::allocator_traits<allocator_type>::void_pointer;

		template<NodeCompression compression>
		using specific_ptr_type = typename std::allocator_traits<specific_alloc<compression>>::pointer;
		using uncompressed_ptr_type = specific_ptr_type<NodeCompression::uncompressed>;
		using compressed_ptr_type = specific_ptr_type<NodeCompression::compressed>;

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

		bool operator==(const NodePtr &other) const{
			return raw == other.raw;
		}
	};
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_NODEPTR_HPP
