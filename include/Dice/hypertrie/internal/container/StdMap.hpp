#ifndef HYPERTRIE_STDMAP_HPP
#define HYPERTRIE_STDMAP_HPP

#include <map>
#include <fmt/format.h>

namespace hypertrie::internal::container {
    template<typename key_type, typename value>
    using std_map = std::map
            <
                    key_type,
                    value
            >;
}


template<typename Key, typename T>
std::ostream &operator<<(std::ostream &os, const hypertrie::internal::container::std_map<Key, T> &map) {
	return os << fmt::format("{}", map);
}

template<typename K, typename V>
struct fmt::formatter<hypertrie::internal::container::std_map<K, V>> {
private:
	using map_type = hypertrie::internal::container::std_map<K, V>;

public:
	auto parse(format_parse_context &ctx) {
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const map_type &map, FormatContext &ctx) {

		bool first = true;
		fmt::format_to(ctx.out(), "{{ ");
		for (const auto &entry : map) {
			if (first) {
				first = false;
			} else {
				fmt::format_to(ctx.out(), "\n  ");
			}
			fmt::format_to(ctx.out(), "{} -> {} ", entry.first, entry.second);
		}
		return fmt::format_to(ctx.out(), "}}");
	}
};

template<typename K, typename V>
struct fmt::formatter<hypertrie::internal::container::std_map<K, V*>> {
private:
	using map_type = hypertrie::internal::container::std_map<K, V*>;

public:
	auto parse(format_parse_context &ctx) {
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const map_type &map, FormatContext &ctx) {

		bool first = true;
		fmt::format_to(ctx.out(), "{{ ");
		for (const auto &entry : map) {
			if (first) {
				first = false;
			} else {
				fmt::format_to(ctx.out(), "\n  ");
			}
			fmt::format_to(ctx.out(), "{} -> {} ", entry.first, *entry.second);
		}
		return fmt::format_to(ctx.out(), "}}");
	}
};

#endif //HYPERTRIE_HYPERTRIE_STDMAP_HPPBOOSTFLATMAP_HPP
