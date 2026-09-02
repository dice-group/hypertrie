#ifndef HYPERTRIE_HASHED_HPP
#define HYPERTRIE_HASHED_HPP

#include "dice/hypertrie/internal/raw/node/RawIdentifier.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t>
	struct Hashed {
		using hash_type = typename RawIdentifier<depth, htt_t>::internal_hash_type;

	protected:
		hash_type hash_ = RawIdentifier<depth, htt_t>::seed;

	public:
		constexpr Hashed() noexcept = default;

		explicit constexpr Hashed(hash_type hash) noexcept
			: hash_{hash} {
		}

		[[nodiscard]] constexpr hash_type &hash() noexcept {
			return hash_;
		}

		[[nodiscard]] constexpr hash_type hash() const noexcept {
			return hash_;
		}
	};

}  //namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_HASHED_HPP
