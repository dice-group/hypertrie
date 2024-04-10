#ifndef HYPERTRIE_FMT_SPARSEMAP_HPP
#define HYPERTRIE_FMT_SPARSEMAP_HPP

#include "dice/hypertrie/internal/util/fmt_utils.hpp"
#include <dice/hypertrie/internal/container/SparseMap.hpp>

namespace fmt {
	template <typename Key, typename Value>
	struct formatter<::dice::hypertrie::internal::container::dice_sparse_map<Key, Value>> : public ::dice::hypertrie::internal::util::SimpleParsing {
		template <typename FormatContext>
		auto format(::dice::hypertrie::internal::container::dice_sparse_map<Key, Value> const& map, FormatContext &ctx) {
			return ::dice::hypertrie::internal::util::format_map(map, ctx.out());
		}
	};
}
#endif//HYPERTRIE_FMT_SPARSEMAP_HPP
