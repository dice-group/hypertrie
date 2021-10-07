#ifndef HYPERTRIE_FMT_RAWHYPERTRIECONTEXT_HPP
#define HYPERTRIE_FMT_RAWHYPERTRIECONTEXT_HPP

#include <Dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SpecificNodeStorage.hpp>

#include <Dice/hypertrie/internal/container/fmt_TslMap.hpp>
#include <Dice/hypertrie/internal/container/fmt_TslSet.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>

#include <boost/hana.hpp>

namespace fmt {
	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri_t>
	struct formatter<::hypertrie::internal::raw::RawHypertrieContext<depth, tri_t>> : hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::hypertrie::internal::raw::RawHypertrieContext<depth, tri_t> const &rnc, FormatContext &ctx) {
			namespace hana = ::boost::hana;
			format_to(ctx.out(), "<RawHypertrieContext\n");
			hana::for_each(hana::range_c<size_t, 1, depth + 1>, [&](auto depth_) {
				constexpr auto height = depth + 1 - depth_;
				format_to(ctx.out(), "[{}] FN\n{}\n", height, rnc.node_storage_.template nodes<height, ::hypertrie::internal::raw::FullNode>());
				if constexpr (not(height == 1 and ::hypertrie::internal::raw::HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri_t>))
					format_to(ctx.out(), "[{}] SEN\n{}\n", height, rnc.node_storage_.template nodes<height, ::hypertrie::internal::raw::SingleEntryNode>());
			});
			return format_to(ctx.out(), ">");
		}
	};
}// namespace fmt

#endif//HYPERTRIE_FMT_RAWHYPERTRIECONTEXT_HPP
