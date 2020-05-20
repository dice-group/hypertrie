#ifndef HYPERTRIE_HYPERTRIE_INTERNAL_TRAITS_HPP
#define HYPERTRIE_HYPERTRIE_INTERNAL_TRAITS_HPP

#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"

#include "Dice/hypertrie/internal/util/RawKey.hpp"

namespace hypertrie::internal::node_based {



	template<HypertrieTrait tr_t = default_bool_Hypertrie_t>
	struct Hypertrie_internal_t {
		using tr = tr_t;
		/// public definitions
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		template<typename key, typename value>
		using map_type = typename tr::template map_type<key, value>;
		template<typename key>
		using set_type = typename tr::template set_type<key>;

		using SliceKey = typename tr::SliceKey;
		using Key = typename tr::Key;
		/// internal definitions
		template<pos_type depth>
		using RawKey = hypertrie::internal::RawKey<depth, typename tr::key_part_type>;

		template<pos_type depth>
		using RawSliceKey = hypertrie::internal::RawSliceKey<depth, typename tr::key_part_type>;

		constexpr static bool is_bool_valued = std::is_same_v<value_type, bool>;

		template<pos_type depth>
		static auto subkey(const RawKey<depth> &key, pos_type remove_pos) -> RawKey<depth - 1> {
			RawKey<depth - 1> sub_key;
			for (auto i = 0, j = 0; i < depth; ++i)
				if (i != remove_pos) sub_key[j++] = key[i];
			return sub_key;
		}
	};
};

#endif //HYPERTRIE_HYPERTRIE_INTERNAL_TRAITS_HPP
