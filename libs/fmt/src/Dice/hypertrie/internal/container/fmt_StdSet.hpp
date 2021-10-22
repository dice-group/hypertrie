#ifndef HYPERTRIE_FMT_STDSET_HPP
#define HYPERTRIE_FMT_STDSET_HPP

#include <Dice/hypertrie/internal/container/StdSet.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <typename Key>
	struct formatter<::Dice::hypertrie::internal::container::std_set<Key>> : public ::Dice::hypertrie::internal::util::SimpleParsing {
		template <typename FormatContext>
		auto format(::Dice::hypertrie::internal::container::std_set<Key> const& set, FormatContext &ctx) {
			return ::Dice::hypertrie::internal::util::format_set(set, ctx.out());
		}
	};
}

#endif//HYPERTRIE_FMT_STDSET_HPP
