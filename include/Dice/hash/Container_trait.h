#ifndef CONTAINER_TRAIT_H_
#define CONTAINER_TRAIT_H_

#include <map>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

// unordered container
template <typename T> struct is_unordered_container : std::false_type {};

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
struct is_unordered_container<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>> : std::true_type {};

template <class Key, class Hash, class KeyEqual, class Allocator>
struct is_unordered_container<std::unordered_set<Key, Hash, KeyEqual, Allocator>> : std::true_type {};

template <typename T>
constexpr bool is_unordered_container_v = is_unordered_container<T>::value;

// ordered container
template <typename T> struct is_ordered_container : std::false_type {};

template <class Key, class T, class Compare, class Allocator>
struct is_ordered_container<std::map<Key, T, Compare, Allocator>> : std::true_type {};

template <class Key, class Compare, class Allocator>
struct is_ordered_container<std::set<Key, Compare, Allocator>> : std::true_type {};

template <typename T>
constexpr bool is_ordered_container_v = is_ordered_container<T>::value;

#endif