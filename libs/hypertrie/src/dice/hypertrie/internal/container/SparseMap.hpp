#ifndef HYPERTRIE_SPARSEMAP_HPP
#define HYPERTRIE_SPARSEMAP_HPP

#include <dice/sparse-map/sparse_map.hpp>


#include <dice/hash/DiceHash.hpp>

#include <memory>//allocator_traits


namespace dice::hypertrie::internal::container {
	template<typename Key, typename T, typename Allocator = std::allocator<std::pair<Key, T>>>
	using dice_sparse_map = dice::sparse_map::sparse_map<
			Key,
			T,
			//hypertrie::internal::robin_hood::hash<Key>,
			dice::hash::DiceHash<Key>,
			std::equal_to<Key>,
			typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<Key, T>>,
			dice::sparse_map::sh::power_of_two_growth_policy<2>,
			dice::sparse_map::sh::exception_safety::basic,
			dice::sparse_map::sh::sparsity::high>;
}// namespace dice::hypertrie::internal::container

#endif//HYPERTRIE_SPARSEMAP_HPP