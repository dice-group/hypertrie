#ifndef HYPERTRIE_NAMEOFTYPE_HPP
#define HYPERTRIE_NAMEOFTYPE_HPP

#include <boost/type_index.hpp>

namespace hypertrie::tests::utils {
	template<typename T>
	auto nameOfType() {
		return std::string{boost::typeindex::type_id_with_cvr<T>().pretty_name()};
	}

}// namespace hypertrie::tests::utils
#endif//HYPERTRIE_NAMEOFTYPE_HPP
