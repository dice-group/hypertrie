#ifndef HYPERTRIE_SPARSEMAP_HPP
#define HYPERTRIE_SPARSEMAP_HPP

#include <dice/sparse-map/sparse_map.hpp>


#include <dice/hash/DiceHash.hpp>

#include <memory>//allocator_traits


namespace dice::hypertrie::internal::container {
	template<typename Key, typename Value, typename Hash = dice::hash::DiceHash<Key>, typename Equal = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<Key, Value>>>
	using dice_sparse_map = dice::sparse_map::sparse_map<
			Key,
			Value,
			Hash,
			Equal,
			typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<Key, Value>>,
			dice::sparse_map::sh::power_of_two_growth_policy<2>,
			dice::sparse_map::sh::exception_safety::basic,
			dice::sparse_map::sh::sparsity::high>;
}// namespace dice::hypertrie::internal::container

#endif//HYPERTRIE_SPARSEMAP_HPP
