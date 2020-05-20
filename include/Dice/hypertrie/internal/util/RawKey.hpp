#ifndef HYPERTRIE_RAWKEY_HPP
#define HYPERTRIE_RAWKEY_HPP

#include <array>
#include <optional>

namespace hypertrie::internal {
	template<size_t depth, typename key_part_type>
	using RawKey = std::array<key_part_type, depth>;

	template<size_t depth, typename key_part_type>
	using RawSliceKey = std::array<std::optional<key_part_type>, depth>;
}
#endif //HYPERTRIE_RAWKEY_HPP
