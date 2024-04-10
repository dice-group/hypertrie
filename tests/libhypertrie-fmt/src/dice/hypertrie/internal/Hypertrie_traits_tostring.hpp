#ifndef HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP
#define HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/util/name_of_type.hpp"
#include <string>

namespace dice::hypertrie::internal {

	template<HypertrieTrait htt_t>
	inline auto to_string() {
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		return std::string("<") +
			   "key_part = " + util::name_of_type<key_part_type>() +
			   ", value = " + util::name_of_type<value_type>() +
			   ", map = " + util::name_of_type<typename htt_t::template map_type<key_part_type, value_type, std::allocator<std::byte>>>() +
			   ", set = " + util::name_of_type<typename htt_t::template set_type<key_part_type, std::allocator<std::byte>>>() +
			   ", key_part_tagging_bit = " + std::to_string(htt_t::key_part_tagging_bit) +
			   ", taggable_key_part = " + std::to_string(htt_t::taggable_key_part) +
			   " >";
	}
}// namespace dice::hypertrie::internal
#endif//HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP
