#ifndef HYPERTRIE_COUNTDOWNNTUPLE_HPP
#define HYPERTRIE_COUNTDOWNNTUPLE_HPP

#include "Dice/hypertrie/internal/util/NTuple.hpp"
#include <cmath>
#include <concepts>
#include <functional>
#include <tuple>

namespace hypertrie::internal::util {

	namespace count_tuple_internal {

		template<template<auto X> typename T, int N, int MAX, int MIN, bool count_up,
				 template<typename...> class TT>
		struct repeat {
			static constexpr const bool use_signed = (MAX < 0 or MIN < 0);
			static constexpr const int I = (count_up) ? MIN + N : MAX - N;
			using type = typename append_to_type_seq<
					std::conditional_t<(use_signed), T<I>, T<(I < 0) ? 0u : unsigned(I)>>,
					typename repeat<T, N + 1, MAX, MIN, count_up, TT>::type>::type;
		};

		template<template<auto I> typename T, int MAX, int MIN, bool count_up,
				 template<typename...> class TT>
		struct repeat<T, MAX - MIN + 1, MAX, MIN, count_up, TT> {
			using type = TT<>;
		};

		template<template<auto I> typename T, auto FIRST, auto LAST>
		using CUT = typename repeat<T, 0, (FIRST < LAST) ? LAST : FIRST, (FIRST < LAST) ? FIRST : LAST, (FIRST < LAST), std::tuple>::type;
	}// namespace count_tuple_internal


	/**
	 * This class is a wrapper around a tuple std::tuple<T<FIRST> .. T<LAST>>. It allows access to the elements via get<i>() -> T<I>. Elements are memory aligned from FIRST to LAST which means you can reinterpret a IntegralTemplatedTuple<T,1,5> as a IntegralTemplatedTuple<T,1,3> and will still be able to access the elements 1-3.
	 * @tparam T
	 * @tparam FIRST
	 * @tparam LAST
	 */
	template<template<std::integral auto> typename T, auto FIRST = 0, auto LAST = 0>
	class IntegralTemplatedTuple {
		count_tuple_internal::CUT<T, FIRST, LAST> count_up_tuple_;

		template<std::integral I>
		constexpr static I abs(I a, I b) {
			if (a < b)
				return b - a;
			else
				return a - b;
		}

	public:
		template<auto I>
		auto &get() {

			static constexpr const auto pos = (FIRST <= LAST) ? LAST - I : I - LAST;
			static_assert(pos >= 0 and pos <= abs(LAST, FIRST));
			return std::get<pos>(count_up_tuple_);
		}
	};
}// namespace hypertrie::internal::util


#endif//HYPERTRIE_COUNTDOWNNTUPLE_HPP
