#ifndef HYPERTRIE_METALL_MUTE_WARNINGS_HPP
#define HYPERTRIE_METALL_MUTE_WARNINGS_HPP

//Needed to silence the metall allocator warnings generated.
//Works for gcc AND clang.
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wnull-pointer-arithmetic"
#endif
#include <metall/metall.hpp>
#ifdef __clang__
#pragma GCC diagnostic pop
#endif

#endif//HYPERTRIE_METALL_MUTE_WARNINGS_HPP
