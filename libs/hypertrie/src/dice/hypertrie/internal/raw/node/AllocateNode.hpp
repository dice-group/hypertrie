#ifndef HYPERTRIE_ALLOCATENODE_HPP
#define HYPERTRIE_ALLOCATENODE_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/hypertrie_allocator_trait.hpp"
#include "dice/hypertrie/internal/raw/node/NodeTypes_reflection.hpp"
#include <memory>


namespace dice::hypertrie::internal::raw {

	/**
	 * Helper class to simplify allocation with allocators.
	 * You can use the new_() and delete_() methods exactly how you would use the new and delete commands inside c++.
	 * new_with_alloc() will construct an object and pass the allocator into the constructor of that object.
	 * This is useful if the created object should use the same allocator.
	 */
	template<size_t depth, HypertrieTrait htt_t, template<size_t, typename, typename> typename node_type_t, ByteAllocator allocator_type>
	class AllocateNode {
	public:
		using node_type = node_type_t<depth, htt_t, allocator_type>;
		using cn_allocator_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<node_type>;
		using cn_allocator_traits = typename std::allocator_traits<cn_allocator_type>;
		using pointer = typename cn_allocator_traits::pointer;

	private:
		cn_allocator_type allocator_;

		[[nodiscard]] pointer allocate(size_t n) {
			return cn_allocator_traits::allocate(allocator_, n);
		}
		template<typename... Args>
		void construct(pointer ptr, Args &&...args) {
			cn_allocator_traits::construct(allocator_, std::to_address(ptr), std::forward<Args>(args)...);
		}
		void destroy(pointer ptr) {
			cn_allocator_traits::destroy(allocator_, std::to_address(ptr));
		}

		void deallocate(pointer ptr, size_t n) {
			cn_allocator_traits::deallocate(allocator_, ptr, n);
		}

	public:
		explicit AllocateNode(allocator_type const &alloc) : allocator_(alloc) {}

		template<typename... Args>
		pointer new_(Args &&...args) {
			pointer ptr = allocate(1);
			construct(ptr, std::forward<Args>(args)...);
			return ptr;
		}

		template<typename... Args>
		pointer new_with_alloc(Args &&...args) {
			pointer ptr = allocate(1);
			construct(ptr, std::forward<Args>(args)..., allocator_);
			return ptr;
		}

		void delete_(pointer to_erase) {
			destroy(to_erase);
			deallocate(to_erase, 1);
		}
	};

}// namespace dice::hypertrie::internal::raw
#endif//HYPERTRIE_ALLOCATENODE_HPP
