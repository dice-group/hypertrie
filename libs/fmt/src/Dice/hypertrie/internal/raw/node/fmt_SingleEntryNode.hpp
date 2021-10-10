#ifndef HYPERTRIE_FMT_SINGLEENTRYNODE_HPP
#define HYPERTRIE_FMT_SINGLEENTRYNODE_HPP

#include <Dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <Dice/hypertrie/internal/util/fmt_utils.hpp>

namespace fmt {
	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri_t>
	struct formatter<::hypertrie::internal::raw::SingleEntryNode<depth, tri_t>> : hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::hypertrie::internal::raw::SingleEntryNode<depth, tri_t> const &sen, FormatContext &ctx) {
			return format_to(ctx.out(), "{{ [ref_count={}] <{}> -> {} }}", sen.ref_count(), fmt::join(sen.key(), ", "), sen.value());
		}
	};
}// namespace fmt
#endif//HYPERTRIE_FMT_SINGLEENTRYNODE_HPP
