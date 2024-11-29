#ifndef HYPERTRIE_FMT_ALLOCATENODE_HPP
#define HYPERTRIE_FMT_ALLOCATENODE_HPP

#include "dice/hypertrie/internal/util/fmt_utils.hpp"
#include <dice/hypertrie/internal/raw/node/AllocateNode.hpp>

namespace fmt {
	// TODO: find a way to represent node_type_t.
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t, template<size_t, typename, typename> typename node_type_t, dice::hypertrie::ByteAllocator allocator_type>
	struct formatter<::dice::hypertrie::internal::raw::AllocateNode<depth, htt_t, node_type_t, allocator_type>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename T>
		static auto nameOfType() {
			return ::dice::hypertrie::internal::util::name_of_type<T>();
		}
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::AllocateNode<depth, htt_t, node_type_t, allocator_type> const &, FormatContext &ctx) {
			return format_to(ctx.out(),
							 "<depth = {}, trait = {} ({})>",
							 depth, nameOfType<htt_t>(), htt_t{});
		}
	};
}// namespace fmt

#endif//HYPERTRIE_FMT_ALLOCATENODE_HPP
