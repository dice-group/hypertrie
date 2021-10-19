#ifndef HYPERTRIE_CONFIGHYPERTRIEDEPTHLIMIT_HPP
#define HYPERTRIE_CONFIGHYPERTRIEDEPTHLIMIT_HPP
#include <cctype>

namespace hypertrie {
	// TODO: add option to override this at startup
	static constexpr const std::size_t hypertrie_max_depth = 3;
	static_assert(hypertrie_max_depth >= 1);
}// namespace hypertrie
#endif//HYPERTRIE_CONFIGHYPERTRIEDEPTHLIMIT_HPP
