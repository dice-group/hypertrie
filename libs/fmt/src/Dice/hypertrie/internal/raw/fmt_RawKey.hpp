#ifndef HYPERTRIE_FMT_RAWKEY_HPP
#define HYPERTRIE_FMT_RAWKEY_HPP

#include "Dice/hypertrie/internal/util/fmt_utils.hpp"
#include <Dice/hypertrie/internal/raw/RawKey.hpp>

namespace fmt {
	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct formatter<::hypertrie::internal::raw::RawKey<depth, tri>> : hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::hypertrie::internal::raw::RawKey<depth, tri> const &raw_key, FormatContext &ctx) {
			return ::hypertrie::internal::util::format_array(ctx.out(), raw_key);
		}
	};

	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct formatter<::hypertrie::internal::raw::RawSliceKey<depth, tri>> : hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::hypertrie::internal::raw::RawSliceKey<depth, tri> const &raw_key, FormatContext &ctx) {
			format_to(ctx.out(), "[");
			for (const auto &fixed_key_part : raw_key)
				format_to(ctx.out(), "{} -> {}, ", fixed_key_part.pos, fixed_key_part.key_part);
			return format_to(ctx.out(), "]");
		}
	};
}// namespace fmt
#endif//HYPERTRIE_FMT_RAWKEY_HPP