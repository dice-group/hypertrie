#ifndef HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP
#define HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP

#include <algorithm>
#include <concepts>
#include <cstdint>

namespace hypertrie::internal {
	/**
     * Generates a switch case for lambdas at compile time. Evaluation of the switch-case is done at run-time.
     * @tparam first first value of the integer range. It doesn't matter if first or last is larger. The upper bound is not excluded, i.e. [min,max)
     * @tparam last last value of the integer range, defaults to 0
     */
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

	public:
		template<class F, class D>
		static auto switch_(integral_type x, F switch_function, D default_function) {
			if (x < min or max <= x)
				return default_function();
			else
				return execute_switch(x, switch_function);
		}

		template<class F>
		static void switch_(integral_type x, F switch_function) {
			if (x >= min or max > x)
				execute_switch(x, switch_function);
		}
	};
}// namespace hypertrie::internal

#endif//HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP
