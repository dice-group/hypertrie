#ifndef HYPERTRIE_RAWDIAGONALPOSITIONS_HPP
#define HYPERTRIE_RAWDIAGONALPOSITIONS_HPP

#include <Dice/hypertrie/internal/raw/RawKey.hpp>

namespace hypertrie::internal::raw {

	template<size_t depth>
	class RawKeyPositions {
		static_assert(depth < 8);
		static constexpr size_t bytes = (depth > 0) ? depth / 8U + 1U : 0;
		std::array<std::byte, bytes> data_{};

		void set_true(size_t pos) const noexcept {
			assert(pos < depth);
			auto byte_id = pos / 8;
			data_[byte_id] |= (std::byte(1) << pos % 8U);
		}

	public:
		bool operator[](size_t pos) const noexcept {
			assert(pos < depth);
			auto byte_id = pos / 8;
			return data_[byte_id] & (std::byte(1) << pos % 8U);
		}

		[[nodiscard]] size_t first_pos() const noexcept {
			[[maybe_unused]] size_t pos = 0;
			size_t byte_id = 0;
			for (const auto &byte : data_) {

				for (size_t pos_in_byte = 0; pos_in_byte < 8; ++pos_in_byte) {
					if (byte[pos_in_byte])
						return pos_in_byte + byte_id * 8;
					assert(pos++ < depth);
				}
				byte_id++;
			}
			__builtin_unreachable();
		}

		template<typename = void>
		RawKeyPositions<depth - 1> sub_raw_key_positions(size_t remove_pos) const noexcept {
			static_assert(depth > 0);
			RawKeyPositions<depth - 1> sub_poss;
			bool offset = false;
			for (size_t pos = 0; pos < depth; ++pos)
				if (pos == remove_pos)
					offset = true;
				else if ((*this)[pos])
					sub_poss.set_true(pos - offset);
			return sub_poss;
		}

		template<size_t fixed_depth, HypertrieCoreTrait tri>
		std::optional<RawKey<depth - fixed_depth, tri>> slice(RawKey<depth, tri> const &raw_key, typename tri::key_part_type fixed_key_part) {
			static_assert(depth >= fixed_depth);


			if constexpr (fixed_depth == 0)
				return raw_key;
			else {
				RawKey<depth - fixed_depth, tri> result_key;

				size_t offset = 0;
				for (size_t pos = 0; pos < depth; ++pos) {
					if ((*this)[pos]) {
						if (raw_key[pos] != fixed_key_part)
							return std::nullopt;
						offset++;
					} else
						result_key[pos - offset] = raw_key[pos];
				}
				return result_key;
			}
		}
	};

}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_RAWDIAGONALPOSITIONS_HPP
