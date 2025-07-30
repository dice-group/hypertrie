#ifndef HYPERTRIE_FMT_RAWHYPERTRIECONTEXT_HPP
#define HYPERTRIE_FMT_RAWHYPERTRIECONTEXT_HPP

#include <dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SpecificNodeStorage.hpp>

#include <dice/hypertrie/internal/container/fmt_SparseMap.hpp>
#include <dice/hypertrie/internal/container/fmt_SparseSet.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>

#include <dice/template-library/for.hpp>

namespace fmt {
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t, dice::hypertrie::ByteAllocator allocator_type>
	struct formatter<::dice::hypertrie::internal::raw::RawHypertrieContext<depth, htt_t, allocator_type>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::RawHypertrieContext<depth, htt_t, allocator_type> const &rnc, FormatContext &ctx) {
			format_to(ctx.out(), FMT_STRING("<RawHypertrieContext\n"));
			dice::template_library::for_range<1, depth + 1>([&](auto depth_) {
				constexpr auto height = depth + 1 - depth_;
				format_to(ctx.out(), FMT_STRING("[{}] FN\n{}\n"), height, rnc.node_storage_.template nodes<height, ::dice::hypertrie::internal::raw::FullNode>());
				if constexpr (not(height == 1 and ::dice::hypertrie::HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>))
					format_to(ctx.out(), FMT_STRING("[{}] SEN\n{}\n"), height, rnc.node_storage_.template nodes<height, ::dice::hypertrie::internal::raw::SingleEntryNode>());
			});
			return format_to(ctx.out(), FMT_STRING(">"));
		}
	};
}// namespace fmt

#endif//HYPERTRIE_FMT_RAWHYPERTRIECONTEXT_HPP
