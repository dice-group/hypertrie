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
			return format_to(ctx.out(), "{{ {{ {} }}, {} }}", fmt::join(sen.key(), ", "), sen.value());
		}
	};
}// namespace fmt

namespace dice::hypertrie::internal::raw {
	template<size_t depth, HypertrieTrait htt_t>
	std::ostream &operator<<(std::ostream &os, SingleEntry<depth, htt_t> const &entry) {
		os << fmt::format("{}", entry);
		return os;
	}
} // namespace dice::hypertrie::internal::raw

namespace std {
	// technically not standard compliant, but doctest suggests doing this
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	ostream &operator<<(ostream &os, vector<::dice::hypertrie::internal::raw::SingleEntry<depth, htt_t>> const &entries) {
		os << ::fmt::format("{{{}}}", ::fmt::join(entries, ",\n"));
		return os;
	}
} // namespace std

#endif//HYPERTRIE_FMT_SINGLEENTRY_HPP
