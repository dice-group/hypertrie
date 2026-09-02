#ifndef HYPERTRIE_ALLOCATENODE_HPP
#define HYPERTRIE_ALLOCATENODE_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/NodeTypes_reflection.hpp"
#include <memory>


namespace dice::hypertrie::internal::raw {

	/**
	 * Helper class to simplify allocation with allocators.
	 * You can use the new_() and delete_() methods exactly how you would use the new and delete commands inside c++.
	 * new_with_alloc() will construct an object and pass the allocator into the constructor of that object.
	 * This is useful if the created object should use the same allocator.
	 */
	template<template<size_t, HypertrieTrait, ByteAllocator ...> typename node_type_t, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	class AllocateNode {
	public:
		using node_type = instantiate_Node_t<node_type_t, depth, htt_t, allocator_type>;
		using node_allocator_traits = typename std::allocator_traits<allocator_type>::template rebind_traits<node_type>;
		using node_allocator_type = typename node_allocator_traits::allocator_type;
		using node_pointer = typename node_allocator_traits::pointer;

	private:
		node_allocator_type alloc_;

	public:
		explicit AllocateNode(allocator_type const &alloc) : alloc_{alloc} {}

		template<typename ...Args>
		node_pointer new_(Args &&...args) {
			node_pointer ptr = node_allocator_traits::allocate(alloc_, 1);
			new (std::to_address(ptr)) node_type{std::forward<Args>(args)...};
			return ptr;
		}

		template<typename ...Args>
		node_pointer new_with_alloc(Args &&...args) {
			node_pointer ptr = node_allocator_traits::allocate(alloc_, 1);
			new (std::to_address(ptr)) node_type{std::forward<Args>(args)..., alloc_};
			return ptr;
		}

		void delete_(node_pointer ptr) {
			ptr->~node_type();
			node_allocator_traits::deallocate(alloc_, ptr, 1);
		}
	};

}// namespace dice::hypertrie::internal::raw
#endif//HYPERTRIE_ALLOCATENODE_HPP
