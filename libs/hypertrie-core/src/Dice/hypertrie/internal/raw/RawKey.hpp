#ifndef HYPERTRIE_RAWKEY_HPP
#define HYPERTRIE_RAWKEY_HPP

#include <array>
#include <optional>

#include <Dice/hash/DiceHash.hpp>
#include <Dice/hypertrie/internal/commons/PosType.hpp>
#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>


namespace hypertrie::internal::raw {
	template<size_t depth, HypertrieCoreTrait tri>
	class RawKey : public std::array<typename tri::key_part_type, depth> {
	public:
		template<typename = void>
		auto subkey(pos_type remove_pos) const noexcept {
			static_assert(depth != 0, "A subkey of a key of length 0 is not possible.");
			RawKey<depth - 1, tri> sub_key;
			for (size_t i = 0, j = 0; i < depth; ++i)
				if (i != remove_pos) sub_key[j++] = (*this)[i];
			return sub_key;
		}
	};

	template<size_t depth, HypertrieCoreTrait tri>
	class RawSliceKey : public std::array<std::optional<typename tri::key_part_type>, depth> {};
}// namespace hypertrie::internal::raw


namespace Dice::hash {
	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct is_ordered_container<::hypertrie::internal::raw::RawKey<depth, tri>> : std::true_type {};
}// namespace Dice::hash

namespace Dice::hash {
	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct is_ordered_container<::hypertrie::internal::raw::RawSliceKey<depth, tri>> : std::true_type {};
}// namespace Dice::hash

#endif//HYPERTRIE_RAWKEY_HPP
