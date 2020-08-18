#ifndef HYPERTRIE_HYPERTRIEPREDECLARE_HPP
#define HYPERTRIE_HYPERTRIEPREDECLARE_HPP

#include "Dice/hypertrie/internal/node_based/interface/Hypertrie_traits.hpp"

namespace hypertrie::internal::node_based {

	template<HypertrieTrait tr = default_bool_Hypertrie_t>
	class const_Hypertrie;

	template<HypertrieTrait tr = default_bool_Hypertrie_t>
	class Hypertrie;

}


#endif//HYPERTRIE_HYPERTRIEPREDECLARE_HPP
