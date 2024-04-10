#ifndef HYPERTRIE_RAWKEY_HPP
#define HYPERTRIE_RAWKEY_HPP

#include <array>
#include <cassert>
#include <optional>

#include "dice/hash/DiceHash.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/Key.hpp"
#include "dice/hypertrie/internal/commons/PosType.hpp"


namespace dice::hypertrie::internal::raw {
	template<size_t depth, HypertrieTrait htt_t>
	struct RawKey : std::array<typename htt_t::key_part_type, depth> {
		// I don't see a reason for this to be a template
		//template<typename = void>
		// TODO: double check
		[[nodiscard]] auto subkey(pos_type remove_pos) const noexcept {
			static_assert(depth != 0, "A subkey of a key of length 0 is not possible.");
			RawKey<depth - 1, htt_t> sub_key;
			for (size_t i = 0, j = 0; i < depth; ++i) {
				if (i != remove_pos) sub_key[j++] = (*this)[i];
			}
			return sub_key;
		}
	};

	template<size_t fixed_depth, HypertrieTrait htt_t>
	struct RawSliceKey {
		using key_part_type = typename htt_t::key_part_type;
		struct FixedValue {
			pos_type pos;
			key_part_type key_part;
		};

	private:
		std::array<FixedValue, fixed_depth> fixed_values{};

	public:
		RawSliceKey() = default;

		explicit RawSliceKey(SliceKey<htt_t> const &slice_key) noexcept {
			assert(slice_key.get_fixed_depth() == fixed_depth);
			size_t pos = 0;
			pos_type key_pos = 0;
			for (auto const &opt_key_part : slice_key) {
				if (opt_key_part.has_value()) {
					fixed_values[pos++] = {key_pos, opt_key_part.value()};
				}
				++key_pos;
			}
		}

		[[nodiscard]] auto begin() noexcept { return fixed_values.begin(); }

		[[nodiscard]] auto end() noexcept { return fixed_values.end(); }

		[[nodiscard]] auto begin() const noexcept { return fixed_values.begin(); }

		[[nodiscard]] auto end() const noexcept { return fixed_values.end(); }

		template<size_t depth>
		[[nodiscard]] std::optional<RawKey<depth - fixed_depth, htt_t>> slice(RawKey<depth, htt_t> const &raw_key) const noexcept {
			static_assert(depth >= fixed_depth);
			if constexpr (fixed_depth == 0) {
				return raw_key;
			} else {
				RawKey<depth - fixed_depth, htt_t> result_key;
				size_t ith_fixed = 0;
				size_t result_key_pos = 0;
				for (size_t key_pos = 0; key_pos < depth; ++key_pos) {
					if (ith_fixed != fixed_depth and fixed_values[ith_fixed].pos == key_pos) {
						if (fixed_values[ith_fixed].key_part != raw_key[key_pos]) {
							return std::nullopt;
						}
						ith_fixed++;
					} else {
						result_key[result_key_pos] = raw_key[key_pos];
						result_key_pos++;
					}
				}
				return result_key;
			}
		}

		[[nodiscard]] FixedValue const &operator[](size_t i) const noexcept {
			assert(i < fixed_depth);
			return fixed_values[i];
		}

		[[nodiscard]] FixedValue &operator[](size_t i) noexcept {
			assert(i < fixed_depth);
			return fixed_values[i];
		}

		// I don't see a reason for this to be a template
		//<typename = void>
		// TODO: double check
		[[nodiscard]] auto subkey(pos_type const remove_pos) const noexcept {
			static_assert(fixed_depth != 0, "A sub-slice-key the count of fixed entries cannot be less than 0.");
			assert(fixed_values.contains(remove_pos));
			RawSliceKey<fixed_depth - 1, htt_t> sub_slicekey;
			pos_type offset = 0;
			for (size_t i = 0, j = 0; i < fixed_depth; ++i) {
				if (fixed_values[i] == remove_pos) {
					offset = 1;
				} else {
					sub_slicekey.fixed_values[j++] = {fixed_values[i].pos - offset, fixed_values[i].key_part};
				}
			}
			return sub_slicekey;
		}

		// I don't see a reason for this to be a template
		//<typename = void>
		// TODO: double check
		[[nodiscard]] auto subkey_i(pos_type const remove_ith) const noexcept {
			static_assert(fixed_depth != 0, "A sub-slice-key the count of fixed entries cannot be less than 0.");
			assert(remove_ith < fixed_depth);
			RawSliceKey<fixed_depth - 1, htt_t> sub_slicekey;
			pos_type offset = 0;
			for (size_t i = 0, j = 0; i < fixed_depth; ++i) {
				if (i == remove_ith) {
					offset = 1;
				} else {
					sub_slicekey[j++] = {pos_type(fixed_values[i].pos - offset), fixed_values[i].key_part};
				}
			}
			return sub_slicekey;
		}
	};
}// namespace dice::hypertrie::internal::raw


namespace dice::hash {
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct is_ordered_container<::dice::hypertrie::internal::raw::RawKey<depth, htt_t>> : std::true_type {};
}// namespace dice::hash


#endif//HYPERTRIE_RAWKEY_HPP
