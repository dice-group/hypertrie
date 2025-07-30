#ifndef HYPERTRIE_FMT_SINGLEENTRY_HPP
#define HYPERTRIE_FMT_SINGLEENTRY_HPP

#include <dice/hypertrie/internal/raw/node/SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/util/fmt_utils.hpp>

namespace fmt {
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct formatter<::dice::hypertrie::internal::raw::SingleEntry<depth, htt_t>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::SingleEntry<depth, htt_t> const &sen, FormatContext &ctx) {
			return format_to(ctx.out(), FMT_STRING("{{ {{ {} }}, {} }}"), fmt::join(sen.key(), ", "), sen.value());
		}
	};
}// namespace fmt
#endif//HYPERTRIE_FMT_SINGLEENTRY_HPP
