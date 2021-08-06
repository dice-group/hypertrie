#ifndef HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP
#define HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP

#include <algorithm>
#include <concepts>
#include <cstdint>

namespace hypertrie::internal {
	template<std::integral auto first, std::integral auto last = 0>
	struct compiled_switch {
		static constexpr bool USE_SIGNED = first < 0 or last < 0;
		using integral_type = std::conditional_t<USE_SIGNED, intmax_t, uintmax_t>;
		static constexpr integral_type min = std::min(first, last);
		static constexpr integral_type max = std::max(first, last);

	private:
		template<class T, class F, integral_type i = min>
		static auto execute_switch(T value, F f) {
			if constexpr (i < max) {
				if (value == i)
					return f(std::integral_constant<T, i>{});
				else
					return execute_switch<T, F, i + 1>(value, f);
			} else {
				return f(std::integral_constant<T, i>{});
			}
		}

		template<class T, class F, integral_type i = min>
		static void execute_void_switch(T value, F f) {
			if constexpr (i < max) {
				if (value == i)
					f(std::integral_constant<T, i>{});
				else
					execute_switch<T, F, i + 1>(value, f);
			} else {
				f(std::integral_constant<T, i>{});
			}
		}

		template<class T, class F, T... Is>
		static auto execute_switch(T i, F f, std::integer_sequence<T, Is...>) {
			// extract common return type
			using return_type = std::decay_t<std::common_type_t<decltype(f(std::integral_constant<T, Is + min>{}))...>>;
			// check if the return type is void.
			if constexpr (std::is_same_v<return_type, void>) {
				execute_void_switch(i, f);
			} else {
				return execute_switch(i, f);
			}
		}

	public:
		template<class F, class D>
		static auto switch_(integral_type x, F switch_function, D default_function) {
			if (x < min or max <= x)
				return default_function();
			else
				return execute_switch(x, switch_function, std::make_index_sequence<max - min>{});
		}

		template<class F, class D>
		static void switch_void(integral_type x, F switch_function, D default_function) {
			if (x < min or max <= x)
				default_function();
			else
				execute_void_switch(x, switch_function);
		}

		template<class F>
		static void switch_void(integral_type x, F switch_function) {
			if (x >= min or max > x)
				execute_void_switch(x, switch_function);
		}
	};
}// namespace hypertrie::internal

#endif//HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP
