#ifndef HYPERTRIE_UNSAFE_CAST_HPP
#define HYPERTRIE_UNSAFE_CAST_HPP
#include <memory>

namespace hypertrie::internal::util {

	template<typename out_type, typename in_type>
	inline constexpr out_type &unsafe_cast(in_type &in) {
		return *reinterpret_cast<out_type *>(std::to_address(in));
	}

//	template<typename out_type, size_t out_N , typename in_type, size_t in_N>
//	inline constexpr out_type[out_N] &unsafe_cast(in_type &in[in_N]) & {
//		return *reinterpret_cast<out_type[out_N] *>(&in);
//	}
}// namespace hypertrie::internal::util
#endif//HYPERTRIE_UNSAFE_CAST_HPP
