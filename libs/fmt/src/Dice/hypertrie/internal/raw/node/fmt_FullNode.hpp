#ifndef HYPERTRIE_FMT_FULLNODE_HPP
#define HYPERTRIE_FMT_FULLNODE_HPP

#include <Dice/hypertrie/internal/raw/node/FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <Dice/hypertrie/internal/util/fmt_utils.hpp>

namespace fmt {
	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri_t>
	struct formatter<::hypertrie::internal::raw::FullNode<depth, tri_t>> : hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::hypertrie::internal::raw::FullNode<depth, tri_t> const &fn, FormatContext &ctx) {
			format_to(ctx.out(), "{{ [size={},ref_count={}] ", fn.size(), fn.ref_count());
			for (const auto &pos : iter::range(depth)) {
				format_to(ctx.out(), "{}: {}\n", pos, fn.edges(pos));
			}
			return format_to(ctx.out(), " }}");
		}
	};
}// namespace fmt

#endif//HYPERTRIE_FMT_FULLNODE_HPP
