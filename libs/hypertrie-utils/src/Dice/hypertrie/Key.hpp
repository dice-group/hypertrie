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
		[[nodiscard]] size_t get_fixed_depth() const noexcept {
			return this->size() - std::ranges::count(*this, std::nullopt);
		}
	};
}// namespace hypertrie

#endif//HYPERTRIE_KEY_HPP