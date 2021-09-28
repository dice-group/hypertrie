#ifndef HYPERTRIE_FMT_RAWKEY_HPP
#define HYPERTRIE_FMT_RAWKEY_HPP

#include <Dice/hypertrie/internal/raw/RawKey.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct formatter<::hypertrie::internal::raw::RawKey<depth, tri>> : hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::hypertrie::internal::raw::RawKey<depth, tri> const &raw_key, FormatContext &ctx) {
			return ::hypertrie::internal::util::format_array(ctx.out(), raw_key);
		}
	};

	template <size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct formatter<::hypertrie::internal::raw::RawSliceKey<depth, tri>> : hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::hypertrie::internal::raw::RawSliceKey<depth, tri> const &raw_key, FormatContext &ctx) {
			return ::hypertrie::internal::util::format_array(ctx.out(), raw_key);
		}
	};
}
#endif//HYPERTRIE_FMT_RAWKEY_HPP