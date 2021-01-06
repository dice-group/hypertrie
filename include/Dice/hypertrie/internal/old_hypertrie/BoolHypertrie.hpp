#ifndef HYPERTRIE_BOOLHYPERTRIE_INTERFACE_HPP
#define HYPERTRIE_BOOLHYPERTRIE_INTERFACE_HPP

#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"

#include "Dice/hypertrie/internal/container/AllContainer.hpp"

#include "Dice/hypertrie/internal/old_hypertrie/BoolHypertrie_impl.hpp"
#include "Dice/hypertrie/internal/old_hypertrie/BoolHypertrie_Hash_Diagonal_impl.hpp"

namespace hypertrie::internal::interface {
	template<typename key_part_type, template<typename, typename> class map_type,
			template<typename> class set_type>
	struct boolhypertrie {
		using BoolHypertrie = hypertrie::internal::BoolHypertrie<key_part_type, map_type, set_type>;
		using const_BoolHypertrie = hypertrie::internal::const_BoolHypertrie<key_part_type, map_type, set_type>;
		using HashDiagonal = hypertrie::internal::HashDiagonal<key_part_type, map_type, set_type>;
		using OrderedDiagonal = hypertrie::internal::OrderedDiagonal<key_part_type, map_type, set_type>;
	};
}

#endif //HYPERTRIE_BOOLHYPERTRIE_INTERFACE_HPP
