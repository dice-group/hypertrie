#ifndef HYPERTRIE_FMT_STDSET_HPP
#define HYPERTRIE_FMT_STDSET_HPP

#include <dice/hypertrie/internal/container/StdSet.hpp>
#include "dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <typename Key>
	struct formatter<::dice::hypertrie::internal::container::std_set<Key>> : public ::dice::hypertrie::internal::util::SimpleParsing {
		template <typename FormatContext>
		auto format(::dice::hypertrie::internal::container::std_set<Key> const& set, FormatContext &ctx) {
			return ::dice::hypertrie::internal::util::format_set(set, ctx.out());
		}
	};
}

#endif//HYPERTRIE_FMT_STDSET_HPP
