#ifndef HYPERTRIE_DEREF_MAP_ITERATOR_HPP
#define HYPERTRIE_DEREF_MAP_ITERATOR_HPP
namespace Dice::hypertrie::internal::container {
	/**
	 * Different maps use different methods to provide access to non-constant mapped_type references. This method provides an abstraction.
	 * @tparam K key_type of the map
	 * @tparam V mapped_type of the map
	 * @param map_it a valid iterator pointing to an entry
	 * @return a reference to the mapped_type
	 */
	template<template<typename, typename, typename> typename map_type, typename K, typename V, typename Allocator>
	inline V &deref(typename map_type<K, V, Allocator>::iterator &map_it) {
		return map_it->second;
	}
}// namespace Dice::hypertrie::internal::container

#endif//HYPERTRIE_DEREF_MAP_ITERATOR_HPP
