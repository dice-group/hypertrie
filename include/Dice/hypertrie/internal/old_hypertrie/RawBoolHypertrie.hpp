#ifndef HYPERTRIE_RAWBOOLHYPERTRIE_HPP
#define HYPERTRIE_RAWBOOLHYPERTRIE_HPP

#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/old_hypertrie/RawBoolHypertrie_impl.hpp"
#include "Dice/hypertrie/internal/old_hypertrie/RawBoolHypertrie_Hash_Diagonal_impl.hpp"

namespace hypertrie::internal::interface {
	template<typename key_part_type, template<typename, typename> typename map_type,
			template<typename> typename set_type>
	struct rawboolhypertrie {
		template<pos_type depth>
		using RawBoolHypertrie = hypertrie::internal::RawBoolHypertrie<depth, key_part_type, map_type, set_type>;
		 template<pos_type depth, pos_type diag_depth>
		using RawHashDiagonal = hypertrie::internal::RawHashDiagonal<diag_depth, depth, key_part_type, map_type, set_type, void>;
	};
}

#endif //HYPERTRIE_RAWBOOLHYPERTRIE_HPP

