#ifndef HYPERTRIE_FMT_TSLSET_HPP
#define HYPERTRIE_FMT_TSLSET_HPP

#include <Dice/hypertrie/internal/container/TslSet.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <typename Key>
	struct formatter<::hypertrie::internal::container::tsl_sparse_set<Key>> : public hypertrie::internal::util::SimpleParsing {
		template <typename FormatContext>
		auto format(::hypertrie::internal::container::tsl_sparse_set<Key> const& set, FormatContext &ctx) {
			return ::hypertrie::internal::util::format_set(set, ctx.out());
		}
	};
}

#endif//HYPERTRIE_FMT_TSLSET_HPP
