#ifndef HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP
#define HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP

#include <algorithm>
#include <concepts>
#include <cstdint>

namespace hypertrie::internal::util {

	namespace detail_switch_cases {

		template<std::integral auto first, std::integral auto last>
		struct Range {
			using int_type = std::conditional_t < first < 0 or last<0, intmax_t, uintmax_t>;

		private:
			static constexpr int_type first_typed = first;
			static constexpr int_type last_typed = last;

		public:
			static constexpr int_type min = std::min(first_typed, last_typed);
			static constexpr int_type max = std::max(first_typed, last_typed);
		};

		template<std::integral auto i, std::integral auto max, class T, class F, std::integral auto min = i>
		constexpr auto execute_case(T value, F f) {
			if constexpr (i < ((max == min) ? max : max - 1))
				if (value != i)
					return execute_case<i + 1, max, T, F, min>(value, f);
			static_assert(i < max);
			return f(std::integral_constant<T, i>{});
		}
	}// namespace detail_switch_cases

	/**
   * Generates a switch-case function at compile-time which is evaluated at runtime. The switch is a lambda like: `[&](auto i_t){ ... }`.
   * `i_t` can be used as template parameter. It will be instantiated with the values between first and last excluding
   * the maximum value. If any of the <div>cases_function</div>s returns all of them must return. The types of the
   * returned objects must be compatible. This applies also to the default_function.
   * @tparam first first value of the integer range which is switched over. It doesn't matter if first or last is
   * larger. The upper bound is not excluded, i.e. [min,max)
   * @tparam last last value of the integer range which is switched over
   * @tparam F automatically deduced type of switch function
   * @tparam D automatically deduced type of default_function
   * @param condition the switch condition
   * @param cases_function the switch function template or lambda. See function description for details.
   * @param default_function the default fallback if condition is out of range [min,max) spanned b first, last
   * @return The value returned from the switch case
   */
	template<std::integral auto first, std::integral auto last, class F, class D>
	constexpr auto switch_cases(typename detail_switch_cases::Range<first, last>::int_type condition, F cases_function,
								D default_function) {
		using namespace detail_switch_cases;
		using range = Range<first, last>;
		if constexpr (range::min != range::max)
			if (range::min <= condition and condition < range::max)
				return execute_case<range::min, range::max>(condition, cases_function);
		return default_function();
	}

	/**
 * Generates a switch-case function at compile-time which is evaluated at runtime. The switch is a lambda like: `[&](auto i_t){ ... }`.
 * `i_t` can be used as template parameter. It will be instantiated with the values between first and last excluding
 * the maximum value. First is fixed to 0. If any of the <div>cases_function</div>s returns all of them must return. The types of the
 * returned objects must be compatible. This applies also to the default_function.
 * @tparam last last value of the integer range which is switched over
 * @tparam F automatically deduced type of switch function
 * @tparam D automatically deduced type of default_function
 * @param condition the switch condition
 * @param cases_function the switch function template or lambda. See function description for details.
 * @param default_function the default fallback if condition is out of range [min,max) spanned b first, last
 * @return The value returned from the switch case
 */
	template<std::integral auto last, class F, class D>
	constexpr auto switch_cases(typename detail_switch_cases::Range<0, last>::int_type condition, F cases_function,
								D default_function) {
		return switch_cases<0, last>(condition, cases_function, default_function);
	}

	/**
 * Generates a switch-case function at compile-time which is evaluated at runtime. The switch is a lambda like: `[&](auto i_t){ ... }`.
 * `i_t` can be used as template parameter. It will be instantiated with the values between first and last excluding
 * the maximum value. This overload does not allow to return a value from switch_cases and has no default_function.
 * @tparam first first value of the integer range which is switched over. It doesn't matter if first or last is
 * @tparam last last value of the integer range which is switched over
 * @tparam F automatically deduced type of switch function
 * @tparam D automatically deduced type of default_function
 * @param condition the switch condition
 * @param cases_function the switch function template or lambda. See function description for details.
 * @return The value returned from the switch case
 */
	template<std::integral auto first, std::integral auto last, class F>
	constexpr void switch_cases(typename detail_switch_cases::Range<first, last>::int_type x, F cases_function) {
		using namespace detail_switch_cases;
		using range = Range<first, last>;

		if (range::min <= x and x < range::max)
			execute_case<range::min, range::max>(x, cases_function);
	}

	/**
 * Generates a switch-case function at compile-time which is evaluated at runtime. The switch is a lambda like: `[&](auto i_t){ ... }`.
 * `i_t` can be used as template parameter. It will be instantiated with the values between first and last excluding
 * the maximum value. First is fixed to 0. This overload does not allow to return a value from switch_cases and has no default_function.
 * @tparam last last value of the integer range which is switched over
 * @tparam F automatically deduced type of switch function
 * @tparam D automatically deduced type of default_function
 * @param condition the switch condition
 * @param cases_function the switch function template or lambda. See function description for details.
 * @return The value returned from the switch case
 */
	template<std::integral auto last, class F>
	constexpr void switch_cases(typename detail_switch_cases::Range<0, last>::int_type x, F cases_function) {
		switch_cases<0, last>(x, cases_function);
	}
}// namespace hypertrie::internal::util

#endif//HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP
