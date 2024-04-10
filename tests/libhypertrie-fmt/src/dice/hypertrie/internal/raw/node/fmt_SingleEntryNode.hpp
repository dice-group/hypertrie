#ifndef HYPERTRIE_FMT_SINGLEENTRYNODE_HPP
#define HYPERTRIE_FMT_SINGLEENTRYNODE_HPP

#include <dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/util/fmt_utils.hpp>

namespace fmt {
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct formatter<::dice::hypertrie::internal::raw::SingleEntryNode<depth, htt_t>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::SingleEntryNode<depth, htt_t> const &sen, FormatContext &ctx) {
			return format_to(ctx.out(), "{{ [ref_count={}] <{}> -> {} }}", sen.ref_count(), fmt::join(sen.key(), ", "), sen.value());
		}
	};
}// namespace fmt
#endif//HYPERTRIE_FMT_SINGLEENTRYNODE_HPP
