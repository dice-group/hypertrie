#ifndef HYPERTRIE_GENERATOR_HPP
#define HYPERTRIE_GENERATOR_HPP

#if __cpp_lib_generator >= 202207L

#include <generator>

#else

#define DICE_TEMPLATELIBRARY_GENERATOR_STD_COMPAT 1
#include <dice/template-library/generator.hpp>

#endif

namespace std {
    using ::std::ranges::elements_of;
} // namespace std

#endif // HYPERTRIE_GENERATOR_HPP
