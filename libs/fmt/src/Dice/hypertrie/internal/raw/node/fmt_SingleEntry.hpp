#ifndef HYPERTRIE_FMT_SINGLEENTRY_HPP
#define HYPERTRIE_FMT_SINGLEENTRY_HPP

#include <Dice/hypertrie/internal/raw/node/SingleEntry.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <Dice/hypertrie/internal/util/fmt_utils.hpp>

namespace fmt {
	template<size_t depth, ::Dice::hypertrie::internal::raw::HypertrieCoreTrait tri_t>
	struct formatter<::Dice::hypertrie::internal::raw::SingleEntry<depth, tri_t>> : ::Dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::Dice::hypertrie::internal::raw::SingleEntry<depth, tri_t> const &sen, FormatContext &ctx) {
			return format_to(ctx.out(), "{{ {{ {} }}, {} }}", fmt::join(sen.key(), ", "), sen.value());
		}
	};
}// namespace fmt
#endif//HYPERTRIE_FMT_SINGLEENTRY_HPP
