#ifndef HYPERTRIE_FMT_UTILS_HPP
#define HYPERTRIE_FMT_UTILS_HPP

#include <fmt/format.h>
#include <boost/type_index.hpp>

namespace hypertrie::internal::util {
	// only allow "{}" and "{:}" in the formatting string for our own types
	struct SimpleParsing {
		constexpr auto parse(fmt::format_parse_context &ctx) {
			auto it = ctx.begin();
			if (it != ctx.end() && *it != '}') {
				throw fmt::format_error("invalid format");
			}
			return it;
		}
	};

	template<typename T>
	inline std::string name_of_type() {
		std::string string = boost::typeindex::type_id<T>().pretty_name();
		auto pos = string.find('<');
		return string.substr(0, pos);
	}
}// namespace hypertrie::internal::util

#endif//HYPERTRIE_FMT_UTILS_HPP