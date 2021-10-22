#ifndef HYPERTRIE_FMT_UTILS_HPP
#define HYPERTRIE_FMT_UTILS_HPP

#include <fmt/format.h>
#include <boost/type_index.hpp>
#include <array>
#include <bitset>
#include <optional>
#include <vector>

// TODO: refactor and make the parameter format consistent!
namespace Dice::hypertrie::internal::util {
	// only allow "{}" and "{:}" in the formatting string for our own types
	struct SimpleParsing {
		static constexpr auto parse(fmt::format_parse_context &ctx) {
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

	template <typename OuterItter, typename Container>
	auto format_container_optional(OuterItter out, Container const& container, std::string fallback = "-") {
		out = ::fmt::format_to(out, "{{");
		if(!container.empty()) {
			auto iter = container.begin(), end = container.end();
			if (*iter) {
				out = ::fmt::format_to(out, "{}", **iter);
			} else {
				out = ::fmt::format_to(out, "{}", fallback);
			}
			for_each(++iter, end, [&out, &fallback](auto opt_val){
				if (opt_val) {
					out = ::fmt::format_to(out, ", {}", *opt_val);
				} else {
					out = ::fmt::format_to(out, ", {}", fallback);
				}
			});
		}
		return ::fmt::format_to(out, "}}");
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

	template <typename Set, typename OutIter>
	auto format_set(Set const& set, OutIter out) {
		out = format_to(out, "{{");
		if(set.size() != 0) {
			auto set_iter = set.begin(), set_end = set.end();
			out = format_to(out, "{}", *(set_iter++));
			std::for_each(set_iter, set_end, [&out](auto val){out = format_to(out, ", {}", val);});
		}
		return format_to(out, "}}");
	}

	template <typename Map, typename OutIter>
	auto format_map_ptr_val(Map const& map, OutIter out) {
		out = format_to(out, "{{");
		if(map.size() != 0) {
			auto map_iter = map.begin(), map_end = map.end();
			// TODO: @Lukas: why do we need this?
			{
				auto const &[key, value] = *(map_iter++);
				out = format_to(out, "({}, {})", key, *value);
			}
			std::for_each(map_iter, map_end, [&out](auto val){
				auto const &[key, value] = val;
				out = format_to(out, ", ({}, {})", key, *value);
			});
		}
		return format_to(out, "}}");
	}

	template <typename Map, typename OutIter>
	auto format_map(Map const& map, OutIter out) {
		out = format_to(out, "{{");
		if(map.size() != 0) {
			auto map_iter = map.begin(), map_end = map.end();
			{
				auto const &[key, value] = *(map_iter++);
				out = format_to(out, "({}, {})", key, value);
			}
			std::for_each(map_iter, map_end, [&out](auto val){
			  auto const &[key, value] = val;
			  out = format_to(out, ", ({}, {})", key, value);
			});
		}
		return format_to(out, "}}");
	}

	template <typename T, typename OutIter>
	auto format_vector(std::vector<T> const& vec, OutIter out) {
		out = format_to(out, "{{");
		if(!vec.empty()) {
			auto iter = vec.begin(), end = vec.end();
			out = format_to(out, "{}", *(iter++));
			std::for_each(iter, end, [&out](auto val){out = format_to(out, ", {}", val);});
		}
		return format_to(out, "}}");
	}

	template <typename T, typename OutIter>
	auto format_vector(std::vector<std::optional<T>> const& vec, OutIter out) {
		return format_container_optional(out, vec);
	}

}// namespace Dice::hypertrie::internal::util
#endif//HYPERTRIE_FMT_UTILS_HPP