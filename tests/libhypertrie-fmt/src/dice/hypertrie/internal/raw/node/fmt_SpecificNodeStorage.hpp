#ifndef HYPERTRIE_FMT_SPECIFICNODESTORAGE_HPP
#define HYPERTRIE_FMT_SPECIFICNODESTORAGE_HPP

#include <dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp>
#include <dice/hypertrie/internal/util/fmt_utils.hpp>

namespace fmt {
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t, template<size_t, typename, typename...> typename node_type, dice::hypertrie::ByteAllocator allocator_type>
	struct formatter<::dice::hypertrie::internal::raw::SpecificNodeStorage<depth, htt_t, node_type, allocator_type>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::SpecificNodeStorage<depth, htt_t, node_type, allocator_type> const &sns, FormatContext &ctx) {
			return ::dice::hypertrie::internal::util::format_map_ptr_val(sns.nodes(), ctx.out());
		}
	};
}// namespace fmt

#endif//HYPERTRIE_FMT_SPECIFICNODESTORAGE_HPP
