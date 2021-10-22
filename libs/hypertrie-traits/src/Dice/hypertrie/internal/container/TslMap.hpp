#ifndef HYPERTRIE_TSLMAP_HPP
#define HYPERTRIE_TSLMAP_HPP

#include <tsl/sparse_map.h>

#include "Dice/hypertrie/internal/container/deref_map_iterator.hpp"
#include <Dice/hash/DiceHash.hpp>
#include <memory>//allocator_traits


namespace Dice::hypertrie::internal::container {
	template<typename Key, typename T, typename Allocator = std::allocator<std::pair<Key, T>>>
	using tsl_sparse_map = tsl::sparse_map<Key,
										   T,
										   //hypertrie::internal::robin_hood::hash<Key>,
										   Dice::hash::DiceHash<Key>,
										   std::equal_to<Key>,
										   typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<Key, T>>,
										   tsl::sh::power_of_two_growth_policy<2>,
										   tsl::sh::exception_safety::basic,
										   tsl::sh::sparsity::high>;

	template<typename K, typename V, typename Allocator>
	inline V &deref(typename tsl_sparse_map<K, V, Allocator>::iterator &map_it) {
		return map_it->value();
	}
}// namespace Dice::hypertrie::internal::container

#endif//HYPERTRIE_TSLMAP_HPP