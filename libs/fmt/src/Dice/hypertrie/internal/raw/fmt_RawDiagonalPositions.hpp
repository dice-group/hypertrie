#ifndef HYPERTRIE_FMT_RAWDIAGONALPOSITIONS_HPP
#define HYPERTRIE_FMT_RAWDIAGONALPOSITIONS_HPP
#include <Dice/hypertrie/internal/raw/RawDiagonalPositions.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template<size_t depth> struct formatter<::hypertrie::internal::raw::RawKeyPositions<depth>> : hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::hypertrie::internal::raw::RawKeyPositions<depth> const &raw_key, FormatContext &ctx) {
			return ::hypertrie::internal::util::format_bitset(ctx.out(), raw_key);
		}
	};
}

#endif//HYPERTRIE_FMT_RAWDIAGONALPOSITIONS_HPP