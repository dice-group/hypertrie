#ifndef HYPERTRIE_FMT_STDSET_HPP
#define HYPERTRIE_FMT_STDSET_HPP

#include <Dice/hypertrie/internal/container/StdSet.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <typename Key>
	struct formatter<::hypertrie::internal::container::std_set<Key>> : public hypertrie::internal::util::SimpleParsing {
		template <typename FormatContext>
		auto format(::hypertrie::internal::container::std_set<Key> const& set, FormatContext &ctx) {
			auto out = format_to(ctx.out(), "{{");
			if(set.size() != 0) {
				auto iter = set.begin(), end = set.end();
				out = format_to(out, "{}", *(iter++));
				std::for_each(iter, end, [&out](auto val){out = format_to(out, ", {}", val);});
			}
			return format_to(out, "}}");
		}
	};
}

#endif//HYPERTRIE_FMT_STDSET_HPP
