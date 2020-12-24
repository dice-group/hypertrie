#ifndef CONTAINER_TRAIT_H_
#define CONTAINER_TRAIT_H_

/** @file Home of custom type traits for the diceHash.
 * If you want to add another class to the trait, the safest place to do so is in this file.
 * Why is there no namespace?
 */

#include <map>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

/** Typetrait for checking if a type T is an unordered container.
 * An example would be std::unordered_map and std::unordered_set.
 * The general version is always false, so it inherits from false_type.
 * @tparam T The type to check.
 */
template <typename T> struct is_unordered_container : std::false_type {};

/** Specialization for std::unordered_map.
 * Adds std::unordered_map to the is_unordered_container trait.
 * @tparam Key Template parameter from std::unordered_map.
 * @tparam T Template parameter from std::unordered_map.
 * @tparam Hash Template parameter from std::unordered_map.
 * @tparam KeyEqual Template parameter from std::unordered_map.
 * @tparam Allocator Template parameter from std::unordered_map.
 */
template <class Key, class T, class Hash, class KeyEqual, class Allocator>
struct is_unordered_container<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>> : std::true_type {};

/** Specialization for std::unordered_set.
 * Adds std::unordered_set to the is_unordered_container trait.
 * @tparam Key Template parameter from std::unordered_set.
 * @tparam Hash Template parameter from std::unordered_set.
 * @tparam KeyEqual Template parameter from std::unordered_set.
 * @tparam Allocator Template parameter from std::unordered_set.
 */
template <class Key, class Hash, class KeyEqual, class Allocator>
struct is_unordered_container<std::unordered_set<Key, Hash, KeyEqual, Allocator>> : std::true_type {};

/** Helper definition.
 * Enables the *_v usage of is_unordered_container.
 * @tparam T The type to check.
 */
template <typename T>
constexpr bool is_unordered_container_v = is_unordered_container<T>::value;

/** Typetrait for checking if a type T is an ordered container.
 * An example would be std::map and std::set.
 * The general version is always false, so it inherits from false_type.
 * @tparam T The type to check.
 */
template <typename T> struct is_ordered_container : std::false_type {};

/** Specialization for std::map.
 * Adds std::map to the is_ordered_container trait.
 * @tparam Key Template parameter from std::map.
 * @tparam T Template parameter from std::map.
 * @tparam Compare Template parameter from std::map.
 * @tparam Allocator Template parameter from std::map.
 */
template <class Key, class T, class Compare, class Allocator>
struct is_ordered_container<std::map<Key, T, Compare, Allocator>> : std::true_type {};

/** Specialization for std::set.
  * Adds std::set to the is_ordered_container trait.
  * @tparam Key Template parameter from std::set.
  * @tparam Compare Template parameter from std::set.
  * @tparam Allocator Template parameter from std::set.
  */
template <class Key, class Compare, class Allocator>
struct is_ordered_container<std::set<Key, Compare, Allocator>> : std::true_type {};

/** Helper definition.
 * Enables the *_v usage of is_ordered_container.
 * @tparam T The type to check.
 */
template <typename T>
constexpr bool is_ordered_container_v = is_ordered_container<T>::value;

#endif