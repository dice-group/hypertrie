#ifndef HYPERTRIE_RAWKEY_HPP
#define HYPERTRIE_RAWKEY_HPP

#include <array>
#include <optional>

#include <Dice/hash/DiceHash.hpp>
#include <Dice/hypertrie/Key.hpp>
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

	template<size_t fixed_depth, HypertrieCoreTrait tri_t>
	class RawSliceKey {
	public:
		using tri = tri_t;
		using key_part_type = typename tri::key_part_type;
		struct FixedValue {
			pos_type pos;
			key_part_type key_part;
		};

	private:
		std::array<FixedValue, fixed_depth> fixed_values{};

	public:
		RawSliceKey() = default;

		explicit RawSliceKey(const SliceKey<typename tri::tr> &slice_key) noexcept {

			assert(slice_key.get_fixed_depth() == fixed_depth);
			size_t pos = 0;
			pos_type key_pos = 0;
			for (const auto &opt_key_part : slice_key) {
				if (opt_key_part.has_value())
					fixed_values[pos++] = {key_pos, opt_key_part.value()};
				++key_pos;
			}
		}
		auto begin() noexcept { return fixed_values.begin(); }

		auto end() noexcept { return fixed_values.end(); }

		auto begin() const noexcept { return fixed_values.begin(); }

		auto end() const noexcept { return fixed_values.end(); }

		template<size_t depth>
		std::optional<RawKey<depth - fixed_depth, tri>> slice(const RawKey<depth, tri> &raw_key) const noexcept {
			static_assert(depth >= fixed_depth);


			if constexpr (fixed_depth == 0)
				return raw_key;
			else {
				RawKey<depth - fixed_depth, tri> result_key;

				size_t key_pos = 0;
				size_t ith_fixed = 0;
				size_t result_key_pos = 0;
				while (key_pos < depth) {
					if (ith_fixed != fixed_depth and fixed_values[ith_fixed].pos == key_pos) {
						if (fixed_values[ith_fixed].key_part != raw_key[key_pos])
							return std::nullopt;
						ith_fixed++;
					} else {
						result_key[result_key_pos] = raw_key[key_pos];
						result_key_pos++;
					}
					key_pos++;
				}
				return result_key;
			}
		}

		const FixedValue &operator[](size_t i) const noexcept {
			return fixed_values[i];
		}

		FixedValue &operator[](size_t i) noexcept {
			return fixed_values[i];
		}

		template<typename = void>
		auto subkey(pos_type remove_pos) const noexcept {
			static_assert(fixed_depth != 0, "A sub-slice-key the count of fixed entries cannot be less than 0.");
			RawSliceKey<fixed_depth - 1, tri> sub_slicekey;

			assert(fixed_values.contains(remove_pos));

			pos_type offset = 0;
			for (size_t i = 0, j = 0; i < fixed_depth; ++i)
				if (fixed_values[i] == remove_pos)
					offset = 1;
				else
					sub_slicekey.fixed_values[j++] = {fixed_values[i].pos - offset, fixed_values[i].key_part};
			return sub_slicekey;
		}

		template<typename = void>
		auto subkey_i(pos_type remove_ith) const noexcept {
			static_assert(fixed_depth != 0, "A sub-slice-key the count of fixed entries cannot be less than 0.");
			RawSliceKey<fixed_depth - 1, tri> sub_slicekey;

			assert(remove_ith < fixed_depth);

			pos_type offset = 0;
			for (size_t i = 0, j = 0; i < fixed_depth; ++i)
				if (i == remove_ith)
					offset = 1;
				else
					sub_slicekey[j++] = {pos_type(fixed_values[i].pos - offset), fixed_values[i].key_part};
			return sub_slicekey;
		}
	};
}// namespace hypertrie::internal::raw


namespace Dice::hash {
	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct is_ordered_container<::hypertrie::internal::raw::RawKey<depth, tri>> : std::true_type {};
}// namespace Dice::hash


#endif//HYPERTRIE_RAWKEY_HPP
