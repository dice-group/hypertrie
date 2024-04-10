#ifndef HYPERTRIE_CONFIGHYPERTRIEDEPTHLIMIT_HPP
#define HYPERTRIE_CONFIGHYPERTRIEDEPTHLIMIT_HPP

#include <cctype>

namespace dice::hypertrie {
	// TODO: add option to override this at startup
	static constexpr std::size_t hypertrie_max_depth = 5;
	static_assert(hypertrie_max_depth >= 1);
}// namespace dice::hypertrie
#endif//HYPERTRIE_CONFIGHYPERTRIEDEPTHLIMIT_HPP
