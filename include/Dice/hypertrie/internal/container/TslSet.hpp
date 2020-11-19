#ifndef HYPERTRIE_TSLSET_HPP
#define HYPERTRIE_TSLSET_HPP

#include <fmt/format.h>
#include <tsl/sparse_set.h>
#include <vector>

namespace hypertrie::internal::container {

	template<typename Key>
	using tsl_sparse_set = tsl::sparse_set<
			Key,
			hypertrie::internal::robin_hood::hash<Key>,
			std::equal_to<Key>,
			std::allocator<Key>,
			tsl::sh::power_of_two_growth_policy<2>,
			tsl::sh::exception_safety::basic,
			tsl::sh::sparsity::high>;

}

template<typename Key>
std::ostream &operator<<(std::ostream &os, const hypertrie::internal::container::tsl_sparse_set<Key> &set) {
	return os << fmt::format("{}", set);
}


template<typename K>
struct fmt::formatter<hypertrie::internal::container::tsl_sparse_set<K>> {
private:
	using set_type = hypertrie::internal::container::tsl_sparse_set<K>;

public:
	auto parse(format_parse_context &ctx) {
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const set_type &set, FormatContext &ctx) {

		if (set.size() == 0)
			return fmt::format_to(ctx.out(), "{{ }}");
		else {
			fmt::format_to(ctx.out(), "{{ ");
			fmt::format_to(ctx.out(), "{}", fmt::join(std::vector<K>{set.begin(), set.end()}, ", "));
			return fmt::format_to(ctx.out(), " }}");
		}
	}
};
#endif//HYPERTRIE_TSLSET_HPP
