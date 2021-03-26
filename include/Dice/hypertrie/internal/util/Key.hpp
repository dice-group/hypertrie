#ifndef HYPERTRIE_KEY_HPP
#define HYPERTRIE_KEY_HPP

#include <optional>
#include <vector>

#include <Dice/hypertrie/internal/util/PosType.hpp>

namespace hypertrie {
	template<typename key_part_type>
	using Key = ::std::vector<key_part_type>;
	template<typename key_part_type>
	using SliceKey = ::std::vector<::std::optional<key_part_type>>;
}// namespace hypertrie

#endif//HYPERTRIE_KEY_HPP
