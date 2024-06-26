#ifndef HYPERTRIE_RAWDIAGONALPOSITIONS_HPP
#define HYPERTRIE_RAWDIAGONALPOSITIONS_HPP

#include "dice/hypertrie/internal/raw/RawKey.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth>
	class RawKeyPositions {
		static_assert(depth < 8);
		static constexpr size_t bytes = (depth > 0) ? depth / 8U + 1U : 0;
		std::array<std::byte, bytes> data_{};

		void set_true(size_t pos) noexcept {
			assert(pos < depth);
			size_t byte_id = pos / 8;
			data_[byte_id] |= (std::byte(1) << pos % 8U);
		}

		template<size_t other_depth>
		friend class RawKeyPositions;

	public:
		RawKeyPositions() = default;

		template<typename Iterable>
		explicit RawKeyPositions(Iterable positions) noexcept
				requires std::ranges::range<Iterable> and std::is_convertible_v<std::ranges::range_value_t<Iterable>, uint8_t> {
			for (const auto &pos : positions)
				set_true(pos);
		}

		bool operator[](size_t pos) const noexcept {
			assert(pos < depth);
			size_t byte_id = pos / 8;
			return bool(data_[byte_id] & (std::byte(1) << pos % 8U));
		}

		[[nodiscard]] size_t first_pos() const noexcept {
			for (size_t pos = 0; pos < depth; ++pos) {
				if ((*this)[pos]) {
					return pos;
				}
			}
			assert(false);
			__builtin_unreachable();
		}

		template<typename = void>
		[[nodiscard]] RawKeyPositions<depth - 1> sub_raw_key_positions(size_t remove_pos) const noexcept {
			static_assert(depth > 0);
			RawKeyPositions<depth - 1> sub_poss;
			bool offset = false;
			for (size_t pos = 0; pos < depth; ++pos) {
				if (pos == remove_pos) {
					offset = true;
				} else if ((*this)[pos]) {
					sub_poss.set_true(pos - static_cast<size_t>(offset));
				}
			}
			return sub_poss;
		}

		[[nodiscard]] size_t count() const noexcept {
			size_t count = 0;
			for (size_t pos = 0; pos < depth; ++pos) {
				count += (*this)[pos];
			}
			return count;
		}

		template<size_t fixed_depth, HypertrieTrait htt_t>
		[[nodiscard]] std::optional<RawKey<depth - fixed_depth, htt_t>> slice(RawKey<depth, htt_t> const &raw_key, typename htt_t::key_part_type fixed_key_part) const noexcept {
			static_assert(depth >= fixed_depth);
			if constexpr (fixed_depth == 0) {
				return raw_key;
			}
			else {
				RawKey<depth - fixed_depth, htt_t> result_key;
				size_t offset = 0;
				for (size_t pos = 0; pos < depth; ++pos) {
					if ((*this)[pos]) {
						if (raw_key[pos] != fixed_key_part) {
							return std::nullopt;
						}
						offset++;
					} else {
						result_key[pos - offset] = raw_key[pos];
					}
				}
				return result_key;
			}
		}
	};

}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_RAWDIAGONALPOSITIONS_HPP
