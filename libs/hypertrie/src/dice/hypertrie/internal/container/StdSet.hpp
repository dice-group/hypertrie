#ifndef HYPERTRIE_STDSET_HPP
#define HYPERTRIE_STDSET_HPP

#include <memory>//allocator_traits
#include <set>
#include <vector>


namespace dice::hypertrie::internal::container {
	template<typename key_type, typename Allocator = std::allocator<key_type>>
	using std_set = std::set<
			key_type,
			std::less<key_type>,
			typename std::allocator_traits<Allocator>::template rebind_alloc<key_type>>;
} // namespace dice::hypertrie::internal::container

#endif//HYPERTRIE_STDSET_HPP
