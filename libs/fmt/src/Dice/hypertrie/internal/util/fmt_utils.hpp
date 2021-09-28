#ifndef HYPERTRIE_FMT_UTILS_HPP
#define HYPERTRIE_FMT_UTILS_HPP

#include <fmt/format.h>
#include <boost/type_index.hpp>
#include <array>
#include <bitset>
#include <optional>

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

	template <typename OutItter, size_t depth>
	auto format_bitset(OutItter out, std::bitset<depth> const& set) {
		out = ::fmt::format_to(out, "[{:d}", set[0]);
		for (size_t i = 1; i < depth; ++i) {
			out = ::fmt::format_to(out, ", {:d}", set[i]);
		}
		return ::fmt::format_to(out, "]");
	}

	template <typename OutItter, typename T, size_t depth>
	auto format_array(OutItter out, std::array<T, depth> const& arr) {
		out = ::fmt::format_to(out, "[{}", arr[0]);
		for (size_t i = 1; i < depth; ++i) {
			out = ::fmt::format_to(out, ", {}", arr[i]);
		}
		return ::fmt::format_to(out, "]");
	}

	template <typename OutItter, typename T, size_t depth>
	auto format_array(OutItter out, std::array<std::optional<T>, depth> const& arr) {
		if(arr[0])
			out = ::fmt::format_to(out, "[{}", *arr[0]);
		else
			out = ::fmt::format_to(out, "[{}", "-");
		for (size_t i = 1; i < depth; ++i) {
			if(arr[i])
				out = ::fmt::format_to(out, ", {}", *arr[i]);
			else
				out = ::fmt::format_to(out, ", {}", "-");
		}
		return ::fmt::format_to(out, "]");
	}

}// namespace hypertrie::internal::util
#endif//HYPERTRIE_FMT_UTILS_HPP