#ifndef HYPERTRIE_VERSION_HPP
#define HYPERTRIE_VERSION_HPP

#include <array>

namespace dice::hypertrie {
	inline constexpr const char name[] = "hypertrie";
	inline constexpr const char version[] = "0.9.3";
	inline constexpr std::array<int, 3> version_tuple = {0, 9, 3};
}// namespace dice::hypertrie

#endif//HYPERTRIE_VERSION_HPP
