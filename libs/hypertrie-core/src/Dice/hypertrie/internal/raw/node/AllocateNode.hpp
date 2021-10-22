#ifndef HYPERTRIE_ALLOCATENODE_HPP
#define HYPERTRIE_ALLOCATENODE_HPP

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <memory>


namespace Dice::hypertrie::internal::raw {

	// TODO: concept for a Node
	template<size_t depth, HypertrieCoreTrait tri_t, template<size_t, typename> typename node_type_t>
	class AllocateNode {
	public:
		using tri = tri_t;
		using allocator_type = typename tri::allocator_type;
		using node_type = node_type_t<depth, tri>;
		using cn_allocator_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<node_type>;
		using cn_allocator_traits = typename std::allocator_traits<cn_allocator_type>;
		using pointer = typename cn_allocator_traits::pointer;

	private:
		cn_allocator_type allocator_;

		auto allocate(size_t n) {
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
		explicit AllocateNode(const typename tri::allocator_type &alloc) : allocator_(alloc) {}

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

}// namespace Dice::hypertrie::internal::raw
#endif//HYPERTRIE_ALLOCATENODE_HPP
