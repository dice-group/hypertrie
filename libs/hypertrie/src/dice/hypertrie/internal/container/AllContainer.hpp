#ifndef HYPERTRIE_DEFAULT_HPP
#define HYPERTRIE_DEFAULT_HPP

/** @file
 * @brief Accumulation of all container headers (sets and maps) used.
 * Every set and every map follow the same structure for template parameters.
 * Every set has the form <Key_type, Allocator>.
 * Every map has the form <Key_type, Value_type, Allocator>.
 * The allocators will be rebound to the correct type, so you can pass it for any type.
 * A typical default (that this code follows) is to use `std::byte` if the type is irrelevant.
 *
 * The Tsl map has a special way to change its elements.
 * To unify it with the other maps use deref().
 */

#include "dice/hypertrie/internal/container/SparseMap.hpp"
#include "dice/hypertrie/internal/container/SparseSet.hpp"
#include "dice/hypertrie/internal/container/StdMap.hpp"
#include "dice/hypertrie/internal/container/StdSet.hpp"
#include "dice/hypertrie/internal/container/deref_map_iterator.hpp"

#endif//HYPERTRIE_DEFAULT_HPP
