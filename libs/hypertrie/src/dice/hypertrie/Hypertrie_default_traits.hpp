#ifndef HYPERTRIE_HYPERTRIE_DEFAULT_TRAITS_HPP
#define HYPERTRIE_HYPERTRIE_DEFAULT_TRAITS_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/container/AllContainer.hpp"

namespace dice::hypertrie {
	template<typename key_part_type_t,
			 typename value_type_t,
			 template<typename, typename, typename, typename, typename> typename map_type_t = hypertrie::internal::container::dice_sparse_map,
			 template<typename, typename, typename, typename> typename set_type_t = hypertrie::internal::container::dice_sparse_set,
			 bool taggable_key_part_v = false>
	using Hypertrie_trait = Hypertrie_t<key_part_type_t, value_type_t, map_type_t, set_type_t, taggable_key_part_v>;

	using default_bool_Hypertrie_trait = Hypertrie_trait<unsigned long,
														 bool,
														 hypertrie::internal::container::dice_sparse_map,
														 hypertrie::internal::container::dice_sparse_set>;
	using tagged_bool_Hypertrie_trait = Hypertrie_trait<unsigned long,
														bool,
														hypertrie::internal::container::dice_sparse_map,
														hypertrie::internal::container::dice_sparse_set,
														true>;

	using default_long_Hypertrie_trait = Hypertrie_trait<unsigned long,
														 long,
														 hypertrie::internal::container::dice_sparse_map,
														 hypertrie::internal::container::dice_sparse_set>;
	using tagged_long_Hypertrie_trait = Hypertrie_trait<unsigned long,
														long,
														hypertrie::internal::container::dice_sparse_map,
														hypertrie::internal::container::dice_sparse_set,
														true>;

	using default_double_Hypertrie_trait = Hypertrie_trait<unsigned long,
														   double,
														   hypertrie::internal::container::dice_sparse_map,
														   hypertrie::internal::container::dice_sparse_set>;
	using tagged_double_Hypertrie_trait = Hypertrie_trait<unsigned long,
														  double,
														  hypertrie::internal::container::dice_sparse_map,
														  hypertrie::internal::container::dice_sparse_set,
														  true>;
}// namespace dice::hypertrie

#endif//HYPERTRIE_HYPERTRIE_DEFAULT_TRAITS_HPP
