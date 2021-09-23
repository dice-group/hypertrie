#ifndef HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP
#define HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP

#include "Dice/hypertrie/internal/Hypertrie_trait.hpp"

#include <string>

// TODO: link against required boost library
#include <boost/type_index.hpp>

namespace hypertrie::internal {
	template<typename T>
	inline std::string nameOfType() {
		std::string string = boost::typeindex::type_id<T>().pretty_name();
		auto pos = string.find('<');
		return string.substr(0, pos);
	}

	template<HypertrieTrait hypertrie_trait>
	inline auto to_string() {
		using key_part_type = typename hypertrie_trait::key_part_type;
		using value_type = typename hypertrie_trait::value_type;
		return std::string("<" ) +
			   "key_part = " + nameOfType<key_part_type>() +
			   ", value = " + nameOfType<value_type>() +
			   ", allocator = " + nameOfType<typename hypertrie_trait::allocator_type>() +
			   ", map = " + nameOfType<typename hypertrie_trait::template map_type<key_part_type, value_type>>() +
			   ", set = " + nameOfType<typename hypertrie_trait::template set_type<key_part_type>>() +
			   ", lsb_unused = " + hypertrie_trait::lsb_unused +
				" >";
	}
}// namespace hypertrie::internal
#endif//HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP
