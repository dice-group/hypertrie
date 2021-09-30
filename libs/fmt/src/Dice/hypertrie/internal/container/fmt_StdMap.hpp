#ifndef HYPERTRIE_FMT_STDMAP_HPP
#define HYPERTRIE_FMT_STDMAP_HPP

#include <Dice/hypertrie/internal/container/StdMap.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <typename Key, typename Value>
	struct formatter<::hypertrie::internal::container::std_map<Key, Value>> : public hypertrie::internal::util::SimpleParsing {
		template <typename FormatContext>
		auto format(::hypertrie::internal::container::std_map<Key, Value> const& map, FormatContext &ctx) {
			auto out = format_to(ctx.out(), "{{");
			if(map.size() != 0) {
				auto iter = map.begin(), end = map.end();
				{
					auto const &[key, value] = *(iter++);
					out = format_to(out, "({}, {})", key, value);
				}
				std::for_each(iter, end, [&out](auto val){
					auto const &[key, value] = val;
					out = format_to(out, ", ({}, {})", key, value);
				});
			}
			return format_to(out, "}}");
		}
	};
}

#endif//HYPERTRIE_FMT_STDMAP_HPP
