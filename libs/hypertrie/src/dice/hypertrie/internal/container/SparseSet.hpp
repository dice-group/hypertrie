#ifndef HYPERTRIE_SPARSESET_HPP
#define HYPERTRIE_SPARSESET_HPP

#include <dice/sparse-map/sparse_set.hpp>

#include <dice/hash/DiceHash.hpp>

#include <memory>//allocator_traits

namespace dice::hypertrie::internal::container {

	template<typename Key, typename Allocator = std::allocator<Key>>
	using dice_sparse_set = dice::sparse_map::sparse_set<
			Key,
			dice::hash::DiceHash<Key>,
			std::equal_to<Key>,
			typename std::allocator_traits<Allocator>::template rebind_alloc<Key>,
			dice::sparse_map::sh::power_of_two_growth_policy<2>,
			dice::sparse_map::sh::exception_safety::basic,
			dice::sparse_map::sh::sparsity::high>;

} // namespace dice::hypertrie::internal::container
#endif//HYPERTRIE_SPARSESET_HPP
