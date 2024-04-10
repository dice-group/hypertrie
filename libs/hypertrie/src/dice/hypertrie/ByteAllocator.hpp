#ifndef HYPERTRIE_BYTEALLOCATOR_HPP
#define HYPERTRIE_BYTEALLOCATOR_HPP

#include <memory>
#include <type_traits>

namespace dice::hypertrie {

	/**
	 * Concept for <div>Allocator</div>s with value_type std::byte.
	 * @tparam Allocator
	 */
	template<typename Allocator>
	concept ByteAllocator = std::is_same_v<typename std::allocator_traits<Allocator>::value_type, std::byte> ;

}// namespace dice::hypertrie
#endif//HYPERTRIE_BYTEALLOCATOR_HPP
