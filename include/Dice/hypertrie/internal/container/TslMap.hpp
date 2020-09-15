#ifndef HYPERTRIE_TSLMAP_HPP
#define HYPERTRIE_TSLMAP_HPP

#include <absl/hash/hash.h>
#include <fmt/format.h>
#include <tsl/sparse_map.h>


namespace hypertrie::internal::container {

	template<typename Key, typename T>
	using tsl_sparse_map = tsl::sparse_map<Key,
										   T,
										   absl::Hash<Key>,
										   std::equal_to<Key>,
										   std::allocator<std::pair<Key, T>>,
										   tsl::sh::power_of_two_growth_policy<2>,
										   tsl::sh::exception_safety::basic,
										   tsl::sh::sparsity::high>;
}

template<typename Key, typename T>
std::ostream &operator<<(std::ostream &os, const hypertrie::internal::container::tsl_sparse_map<Key, T> &map) {
	return os << fmt::format("{}", map);
}

template<typename K, typename V>
struct fmt::formatter<hypertrie::internal::container::tsl_sparse_map<K, V>> {
private:
	using map_type = hypertrie::internal::container::tsl_sparse_map<K, V>;

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
struct fmt::formatter<hypertrie::internal::container::tsl_sparse_map<K, V*>> {
private:
	using map_type = hypertrie::internal::container::tsl_sparse_map<K, V*>;

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
			fmt::format_to(ctx.out(), "{} -> {} ", entry.first, *	entry.second);
		}
		return fmt::format_to(ctx.out(), "}}");
	}
};
#endif//HYPERTRIE_TSLMAP_HPP
