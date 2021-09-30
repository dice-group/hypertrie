#ifndef HYPERTRIE_FMT_TSLMAP_HPP
#define HYPERTRIE_FMT_TSLMAP_HPP

#include <Dice/hypertrie/internal/container/TslMap.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <typename Key, typename Value>
	struct formatter<::hypertrie::internal::container::tsl_sparse_map<Key, Value>> : public hypertrie::internal::util::SimpleParsing {
		template <typename FormatContext>
		auto format(::hypertrie::internal::container::tsl_sparse_map<Key, Value> const& map, FormatContext &ctx) {
			return ::hypertrie::internal::util::format_map(map, ctx.out());
		}
	};
}
#endif//HYPERTRIE_FMT_TSLMAP_HPP
