#ifndef HYPERTRIE_HYPERTRIE_ALLOCATOR_TRAIT_HPP
#define HYPERTRIE_HYPERTRIE_ALLOCATOR_TRAIT_HPP

#include <cstddef>
#include <memory>

namespace dice::hypertrie {

	template<typename Allocator>
	struct hypertrie_allocator_trait {
		using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<std::byte>;

		template<typename PointedType>
		using pointer = typename std::pointer_traits<typename std::allocator_traits<allocator_type>::pointer>::template rebind<PointedType>;

		using void_pointer = typename std::allocator_traits<allocator_type>::void_pointer;

		template<typename PointedType>
		using const_pointer = typename std::pointer_traits<typename std::allocator_traits<allocator_type>::const_pointer>::template rebind<PointedType>;

		using const_void_pointer = typename std::allocator_traits<allocator_type>::const_void_pointer;

		template<typename T>
		using rebind_alloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<T>;

		static constexpr bool is_stl_alloc = std::is_same_v<allocator_type, std::allocator<std::byte>>;

		static constexpr bool uses_raw_pointer = std::is_same_v<void_pointer, void *>;
	};
}// namespace dice::hypertrie
#endif//HYPERTRIE_HYPERTRIE_ALLOCATOR_TRAIT_HPP
