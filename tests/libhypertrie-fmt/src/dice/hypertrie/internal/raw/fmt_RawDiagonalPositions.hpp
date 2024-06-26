#ifndef HYPERTRIE_FMT_RAWDIAGONALPOSITIONS_HPP
#define HYPERTRIE_FMT_RAWDIAGONALPOSITIONS_HPP
#include <dice/hypertrie/internal/raw/RawDiagonalPositions.hpp>
#include "dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {

	template<size_t depth> struct formatter<::dice::hypertrie::internal::raw::RawKeyPositions<depth>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::RawKeyPositions<depth> const &raw_key, FormatContext &ctx) {
			using namespace ::dice::hypertrie::internal;
			std::vector<pos_type> positions;
			for (pos_type i = 0; i < pos_type(depth); ++i)
				if (raw_key[i])
					positions.push_back(i);

			return format_to(ctx.out(), "[{}]", fmt::join(positions, ", "));
		}
	};
}

#endif//HYPERTRIE_FMT_RAWDIAGONALPOSITIONS_HPP