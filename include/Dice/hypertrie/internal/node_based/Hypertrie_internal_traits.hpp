#ifndef HYPERTRIE_HYPERTRIE_INTERNAL_TRAITS_HPP
#define HYPERTRIE_HYPERTRIE_INTERNAL_TRAITS_HPP

#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"

namespace hypertrie::internal::node_based {

	template<typename tr_t = default_bool_Hypertrie_t>
	struct Hypertrie_internal_t {
		using tr = tr_t;
		/// public definitions
		using key_part_type = typename tr::key_part_type_t;
		using value_type = typename tr::key_part_type_t;
		template<typename key, typename value>
		using map_type = typename tr::template map_type_t<key, value>;
		template<typename key>
		using set_type = typename tr::template set_type_t<key>;

		using SliceKey = typename tr::SliceKey;
		using Key =  typename tr::Key;
		/// internal definitions
		template<pos_type depth>
		using RawKey = std::array<typename tr::key_part_type, depth>;

		template<pos_type depth>
		using RawSliceKey = std::array<std::optional<typename tr::key_part_type>, depth>;
	};
};

#endif //HYPERTRIE_HYPERTRIE_INTERNAL_TRAITS_HPP
