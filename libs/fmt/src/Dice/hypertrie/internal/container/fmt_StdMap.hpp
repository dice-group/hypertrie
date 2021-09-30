#ifndef HYPERTRIE_FMT_STDMAP_HPP
#define HYPERTRIE_FMT_STDMAP_HPP

#include <Dice/hypertrie/internal/container/StdMap.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <typename Key, typename Value>
	struct formatter<::hypertrie::internal::container::std_map<Key, Value>> : public hypertrie::internal::util::SimpleParsing {
		template <typename FormatContext>
		auto format(::hypertrie::internal::container::std_map<Key, Value> const& map, FormatContext &ctx) {
			return ::hypertrie::internal::util::format_map(map, ctx.out());
		}
	};
}

#endif//HYPERTRIE_FMT_STDMAP_HPP
