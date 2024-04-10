#ifndef HYPERTRIE_FMT_STDMAP_HPP
#define HYPERTRIE_FMT_STDMAP_HPP

#include <dice/hypertrie/internal/container/StdMap.hpp>
#include "dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <typename Key, typename Value>
	struct formatter<::dice::hypertrie::internal::container::std_map<Key, Value>> : public ::dice::hypertrie::internal::util::SimpleParsing {
		template <typename FormatContext>
		auto format(::dice::hypertrie::internal::container::std_map<Key, Value> const& map, FormatContext &ctx) {
			return ::dice::hypertrie::internal::util::format_map(map, ctx.out());
		}
	};
}

#endif//HYPERTRIE_FMT_STDMAP_HPP
