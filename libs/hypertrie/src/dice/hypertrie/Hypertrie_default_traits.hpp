#ifndef HYPERTRIE_HYPERTRIE_DEFAULT_TRAITS_HPP
#define HYPERTRIE_HYPERTRIE_DEFAULT_TRAITS_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/container/AllContainer.hpp"

namespace dice::hypertrie {
	template<typename key_part_type_t,
			 typename value_type_t,
			 template<typename, typename, typename> class map_type_t = hypertrie::internal::container::dice_sparse_map,
			 template<typename, typename> class set_type_t = hypertrie::internal::container::dice_sparse_set,
			 ssize_t key_part_tagging_bit = -1>
	using Hypertrie_trait = Hypertrie_t<key_part_type_t, value_type_t, map_type_t, set_type_t, key_part_tagging_bit>;

	using default_bool_Hypertrie_trait = Hypertrie_trait<unsigned long,
														 bool,
														 hypertrie::internal::container::dice_sparse_map,
														 hypertrie::internal::container::dice_sparse_set>;
	using tagged_bool_Hypertrie_trait = Hypertrie_trait<unsigned long,
														bool,
														hypertrie::internal::container::dice_sparse_map,
														hypertrie::internal::container::dice_sparse_set,
														63>;

	using default_long_Hypertrie_trait = Hypertrie_trait<unsigned long,
														 long,
														 hypertrie::internal::container::dice_sparse_map,
														 hypertrie::internal::container::dice_sparse_set>;

	using default_double_Hypertrie_trait = Hypertrie_trait<unsigned long,
														   double,
														   hypertrie::internal::container::dice_sparse_map,
														   hypertrie::internal::container::dice_sparse_set>;
}// namespace dice::hypertrie

#endif//HYPERTRIE_HYPERTRIE_DEFAULT_TRAITS_HPP
