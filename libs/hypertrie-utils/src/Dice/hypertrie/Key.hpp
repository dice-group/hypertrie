#ifndef HYPERTRIE_KEY_HPP
#define HYPERTRIE_KEY_HPP

#include <Dice/hypertrie/internal/Hypertrie_trait.hpp>
#include <optional>
#include <vector>

namespace hypertrie {
	template<::hypertrie::internal::HypertrieTrait tr>
	class Key : public ::std::vector<typename tr::key_part_type> {};


	template<::hypertrie::internal::HypertrieTrait tr>
	class SliceKey : public ::std::vector<::std::optional<typename tr::key_part_type>> {
		size_t get_fixed_depth() {
			size_t fixed_depth = 0;
			for (auto opt_key_part : (*this)) {
				if (opt_key_part.has_value())
					++fixed_depth;
			}
			return fixed_depth;
		}
	};
}// namespace hypertrie

#endif//HYPERTRIE_KEY_HPP