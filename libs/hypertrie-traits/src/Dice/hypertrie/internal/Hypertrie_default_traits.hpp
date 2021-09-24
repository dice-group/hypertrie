#ifndef HYPERTRIE_HYPERTRIE_DEFAULT_TRAITS_HPP
#define HYPERTRIE_HYPERTRIE_DEFAULT_TRAITS_HPP

#include "Dice/hypertrie/internal/container/AllContainer.hpp"

#include "Dice/hypertrie/internal/Hypertrie_trait.hpp"

namespace hypertrie {
	template<typename key_part_type_t,
			 typename value_type_t,
			 typename Allocator = std::allocator<std::byte>,
			 template<typename, typename, typename> class map_type_t = hypertrie::internal::container::tsl_sparse_map,
			 template<typename, typename> class set_type_t = hypertrie::internal::container::tsl_sparse_set,
			 ssize_t key_part_tagging_bit = false>
	using Hypertrie_trait = internal::Hypertrie_t<key_part_type_t, value_type_t, Allocator, map_type_t, set_type_t, key_part_tagging_bit>;

	using default_bool_Hypertrie_trait = Hypertrie_trait<unsigned long,
														 bool,
														 std::allocator<std::byte>,
														 hypertrie::internal::container::tsl_sparse_map,
														 hypertrie::internal::container::tsl_sparse_set>;
	using tagged_bool_Hypertrie_trait = Hypertrie_trait<unsigned long,
														bool,
														std::allocator<std::byte>,
														hypertrie::internal::container::tsl_sparse_map,
														hypertrie::internal::container::tsl_sparse_set,
														63>;

	using default_long_Hypertrie_trait = Hypertrie_trait<unsigned long,
														 long,
														 std::allocator<std::byte>,
														 hypertrie::internal::container::tsl_sparse_map,
														 hypertrie::internal::container::tsl_sparse_set>;

	using default_double_Hypertrie_trait = Hypertrie_trait<unsigned long,
														   double,
														   std::allocator<std::byte>,
														   hypertrie::internal::container::tsl_sparse_map,
														   hypertrie::internal::container::tsl_sparse_set>;
}// namespace hypertrie

#endif//HYPERTRIE_HYPERTRIE_DEFAULT_TRAITS_HPP
