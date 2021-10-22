#ifndef HYPERTRIE_STDMAP_HPP
#define HYPERTRIE_STDMAP_HPP

#include <map>
#include <memory>//allocator_traits
#include "Dice/hypertrie/internal/container/deref_map_iterator.hpp"

namespace Dice::hypertrie::internal::container {
	template<typename key_type, typename value, typename Allocator = std::allocator<std::pair<key_type, value>>>
	using std_map = std::map<
			key_type,
			value,
			std::less<key_type>,
			typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<const key_type, value>>>;
}

#endif//HYPERTRIE_STDMAP_HPP
