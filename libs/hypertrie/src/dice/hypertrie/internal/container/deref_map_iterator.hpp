#ifndef HYPERTRIE_DEREF_MAP_ITERATOR_HPP
#define HYPERTRIE_DEREF_MAP_ITERATOR_HPP

/** @file
 * @brief Provide a unified way to modify entries of different map types.
 * The problematic map is the TslMap.
 * We can distinct the Tsl iterators from the other iterators by the .value() method.
 * So we defined concepts based on that.
 * The deref() template function can now use those concepts to provide different implementations
 * for the two different cases.
 */
namespace dice::hypertrie::internal::container {

	template<typename Iter>
	concept TslIter = requires(Iter iter) {
		iter.value();
	};

	template<typename Iter>
	concept BasicIter = not TslIter<Iter> && requires(Iter iter) {
		iter->second;
	};

	/**
	 * Different maps use different methods to provide access to non-constant mapped_type references. This method provides an abstraction.
	 * @param iter a valid iterator pointing to an entry
	 * @return a reference to the mapped_type
	 */
	template<BasicIter Iter>
	[[nodiscard]] auto &deref(Iter &&iter) {
		return iter->second;
	}

	/**
	 * Different maps use different methods to provide access to non-constant mapped_type references. This method provides an abstraction.
	 * @param iter a valid iterator pointing to an entry
	 * @return a reference to the mapped_type
	 */
	template<TslIter Iter>
	[[nodiscard]] auto &deref(Iter &&iter) {
		return iter.value();
	}
}// namespace dice::hypertrie::internal::container

#endif//HYPERTRIE_DEREF_MAP_ITERATOR_HPP
