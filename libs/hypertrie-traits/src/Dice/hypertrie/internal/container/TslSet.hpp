#ifndef HYPERTRIE_TSLSET_HPP
#define HYPERTRIE_TSLSET_HPP

#include <tsl/sparse_set.h>
#include <vector>
#include <memory> //allocator_traits

namespace hypertrie::internal::container {

	template<typename Key, typename Allocator = std::allocator<Key>>
	using tsl_sparse_set = tsl::sparse_set<
			Key,
			Dice::hash::DiceHash<Key>,
			std::equal_to<Key>,
			typename std::allocator_traits<Allocator>::template rebind_alloc<Key>,
			tsl::sh::power_of_two_growth_policy<2>,
			tsl::sh::exception_safety::basic,
			tsl::sh::sparsity::high>;

}
#endif//HYPERTRIE_TSLSET_HPP
