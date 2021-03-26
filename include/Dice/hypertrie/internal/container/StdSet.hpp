#ifndef HYPERTRIE_STDSET_HPP
#define HYPERTRIE_STDSET_HPP

#include <set>
#include <fmt/format.h>

namespace hypertrie::internal::container {
    template<typename key_type>
    using std_set = std::set
            <
                    key_type
            >;
}



template<typename Key>
std::ostream &operator<<(std::ostream &os, const hypertrie::internal::container::std_set<Key> &set) {
	return os << fmt::format("{}", set);
}


template<typename K>
struct fmt::formatter<hypertrie::internal::container::std_set<K>> {
private:
	using set_type = hypertrie::internal::container::std_set<K>;

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
#endif //HYPERTRIE_STDSET_HPP
