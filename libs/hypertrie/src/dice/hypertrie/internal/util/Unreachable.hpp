#ifndef HYPERTRIE_UNREACHABLE_HPP
#define HYPERTRIE_UNREACHABLE_HPP

#include <cassert>

#define HYPERTRIE_UNREACHABLE \
	assert(false);   			\
	__builtin_unreachable();

#endif//HYPERTRIE_UNREACHABLE_HPP
