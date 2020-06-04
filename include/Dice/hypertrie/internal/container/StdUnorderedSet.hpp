#ifndef HYPERTRIE_STDUNORDEREDSET_HPP
#define HYPERTRIE_STDUNORDEREDSET_HPP

#include <unordered_set>

namespace hypertrie::internal::container {
	template<typename key_type>
	using std_unordered_set = std::unordered_set<
			key_type>;
}

#endif//HYPERTRIE_STDUNORDEREDSET_HPP
