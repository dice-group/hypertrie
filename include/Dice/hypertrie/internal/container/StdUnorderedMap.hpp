#ifndef HYPERTRIE_STDUNORDEREDMAP_HPP
#define HYPERTRIE_STDUNORDEREDMAP_HPP
#include <unordered_map>

namespace hypertrie::internal::container {
	template<typename key_type, typename value>
	using std_unordered_map = std::unordered_map
			<
					key_type,
					value
			>;
}
#endif//HYPERTRIE_STDUNORDEREDMAP_HPP
