#ifndef HYPERTRIE_FMT_ALLOCATENODE_HPP
#define HYPERTRIE_FMT_ALLOCATENODE_HPP

#include <Dice/hypertrie/internal/raw/node/AllocateNode.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	// TODO: find a way to represent node_type_t.
	template <size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri_t, template<size_t, typename> typename node_type_t>
	struct formatter<::hypertrie::internal::raw::AllocateNode<depth, tri_t, node_type_t>> : hypertrie::internal::util::SimpleParsing {
		template<typename T>
		static auto nameOfType() {
			return ::hypertrie::internal::util::name_of_type<T>();
		}
		template <typename FormatContext>
		auto format(::hypertrie::internal::raw::AllocateNode<depth, tri_t, node_type_t> const&, FormatContext &ctx) {
			return format_to(ctx.out(),
					"<depth = {}, trait = {} ({})>",
				 	depth, nameOfType<tri_t>(), tri_t{}
					);
		}
	};
}

#endif//HYPERTRIE_FMT_ALLOCATENODE_HPP
