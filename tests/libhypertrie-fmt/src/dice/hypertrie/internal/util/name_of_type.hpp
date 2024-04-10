#ifndef HYPERTRIE_NAME_OF_TYPE_HPP
#define HYPERTRIE_NAME_OF_TYPE_HPP

#include <boost/type_index.hpp>

namespace dice::hypertrie::internal::util {

	template<typename T>
	inline std::string name_of_type() {
		std::string string = boost::typeindex::type_id<T>().pretty_name();
		auto pos = string.find('<');
		return string.substr(0, pos);
	}

}// namespace dice::hypertrie::internal::util

#endif//HYPERTRIE_NAME_OF_TYPE_HPP
