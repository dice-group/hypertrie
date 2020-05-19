#ifndef HYPERTRIE_HYPERTRIE_TRAIT_HPP
#define HYPERTRIE_HYPERTRIE_TRAIT_HPP

#include <vector>
#include <optional>
#include <Dice/hypertrie/internal/container/AllContainer.hpp>

namespace hypertrie::internal::node_based {

	template<typename key_part_type_t,
			typename value_type_t,
			template<typename, typename> class map_type_t,
			template<typename> class set_type_t>
	struct Hypertrie_t {
		using key_part_type = key_part_type_t;
		using value_type = key_part_type_t;
		template<typename key, typename value>
		using map_type = map_type_t<key, value>;
		template<typename key>
		using set_type = set_type_t<key>;

		using SliceKey = std::vector<std::optional<key_part_type>>;
		using Key =  std::vector<key_part_type>;
	};

	using default_bool_Hypertrie_t = Hypertrie_t<unsigned long,
			bool,
			hypertrie::internal::container::tsl_sparse_map,
			hypertrie::internal::container::tsl_sparse_set>;

	using default_long_Hypertrie_t = Hypertrie_t<unsigned long,
			long,
			hypertrie::internal::container::tsl_sparse_map,
			hypertrie::internal::container::tsl_sparse_set>;

	using default_double_Hypertrie_t = Hypertrie_t<unsigned long,
			double,
			hypertrie::internal::container::tsl_sparse_map,
			hypertrie::internal::container::tsl_sparse_set>;

};

#endif //HYPERTRIE_HYPERTRIE_TRAIT_HPP
