#ifndef HYPERTRIE_UNSAFE_CAST_HPP
#define HYPERTRIE_UNSAFE_CAST_HPP
#include <memory>

namespace hypertrie::internal::util {

	template<typename out_type, typename in_type>
	inline constexpr out_type &unsafe_cast(in_type &in) {
		return *reinterpret_cast<out_type *>(std::addressof(in));
	}

}// namespace hypertrie::internal::util
#endif//HYPERTRIE_UNSAFE_CAST_HPP
