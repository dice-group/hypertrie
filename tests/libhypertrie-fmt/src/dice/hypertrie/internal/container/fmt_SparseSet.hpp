#ifndef HYPERTRIE_FMT_SPARSESET_HPP
#define HYPERTRIE_FMT_SPARSESET_HPP

#include "dice/hypertrie/internal/util/fmt_utils.hpp"
#include <dice/hypertrie/internal/container/SparseSet.hpp>

namespace fmt {
	template <typename Key>
	struct formatter<::dice::hypertrie::internal::container::dice_sparse_set<Key>> : public ::dice::hypertrie::internal::util::SimpleParsing {
		template <typename FormatContext>
		auto format(::dice::hypertrie::internal::container::dice_sparse_set<Key> const& set, FormatContext &ctx) {
			return ::dice::hypertrie::internal::util::format_set(set, ctx.out());
		}
	};
}

#endif//HYPERTRIE_FMT_SPARSESET_HPP
