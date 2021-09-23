#ifndef HYPERTRIE_RAWKEY_HPP
#define HYPERTRIE_RAWKEY_HPP

#include <array>
#include <optional>
#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>

namespace hypertrie::internal::raw {
	template<size_t depth, HypertrieCoreTrait tri>
	class RawKey : public std::array<typename tri::key_part_type, depth>{};

	template<size_t depth, HypertrieCoreTrait tri>
	class RawSliceKey : public std::array<std::optional<typename tri::key_part_type>, depth>{};
}// namespace hypertrie::internal

#endif//HYPERTRIE_RAWKEY_HPP
