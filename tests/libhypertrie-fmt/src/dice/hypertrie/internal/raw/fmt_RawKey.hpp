#ifndef HYPERTRIE_FMT_RAWKEY_HPP
#define HYPERTRIE_FMT_RAWKEY_HPP

#include "dice/hypertrie/internal/util/fmt_utils.hpp"
#include <dice/hypertrie/internal/raw/RawKey.hpp>

namespace fmt {
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct formatter<::dice::hypertrie::internal::raw::RawKey<depth, htt_t>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::RawKey<depth, htt_t> const &raw_key, FormatContext &ctx) {
			return ::dice::hypertrie::internal::util::format_array(ctx.out(), raw_key);
		}
	};

	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct formatter<::dice::hypertrie::internal::raw::RawSliceKey<depth, htt_t>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::RawSliceKey<depth, htt_t> const &raw_key, FormatContext &ctx) {
			std::vector<std::string> entries;
			for (const auto &fixed_key_part : raw_key)
				entries.push_back(fmt::format("{} -> {}", fixed_key_part.pos, fixed_key_part.key_part));
			return format_to(ctx.out(), "[{}]", fmt::join(entries, ", "));
		}
	};
}// namespace fmt
#endif//HYPERTRIE_FMT_RAWKEY_HPP