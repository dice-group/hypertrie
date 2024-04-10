#ifndef HYPERTRIE_UNSAFE_CAST_HPP
#define HYPERTRIE_UNSAFE_CAST_HPP
#include <memory>

namespace dice::hypertrie::internal::util {

	template<typename out_type, typename in_type>
	inline constexpr out_type &unsafe_cast(in_type &in) {
		static_assert(sizeof(in_type) == sizeof(out_type));
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		return *reinterpret_cast<out_type *>(std::addressof(in));
	}

	template<typename out_type, typename in_type>
	inline constexpr out_type unsafe_copy_cast(in_type in) {
		static_assert(sizeof(in_type) == sizeof(out_type));
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		return *reinterpret_cast<out_type *>(std::addressof(in));
	}

}// namespace dice::hypertrie::internal::util
#endif//HYPERTRIE_UNSAFE_CAST_HPP
