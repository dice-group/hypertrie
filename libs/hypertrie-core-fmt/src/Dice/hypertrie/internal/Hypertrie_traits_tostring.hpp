#ifndef HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP
#define HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP

#include "Dice/hypertrie/Hypertrie_trait.hpp"
#include "Dice/hypertrie/internal/util/name_of_type.hpp"
#include <string>

namespace Dice::hypertrie::internal {

	template<HypertrieTrait hypertrie_trait>
	inline auto to_string() {
		using key_part_type = typename hypertrie_trait::key_part_type;
		using value_type = typename hypertrie_trait::value_type;
		return std::string("<") +
			   "key_part = " + util::name_of_type<key_part_type>() +
			   ", value = " + util::name_of_type<value_type>() +
			   ", allocator = " + util::name_of_type<typename hypertrie_trait::allocator_type>() +
			   ", map = " + util::name_of_type<typename hypertrie_trait::template map_type<key_part_type, value_type>>() +
			   ", set = " + util::name_of_type<typename hypertrie_trait::template set_type<key_part_type>>() +
			   ", key_part_tagging_bit = " + std::to_string(hypertrie_trait::key_part_tagging_bit) +
			   ", taggable_key_part = " + std::to_string(hypertrie_trait::taggable_key_part) +
			   " >";
	}
}// namespace Dice::hypertrie::internal
#endif//HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP
