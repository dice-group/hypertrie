#ifndef HYPERTRIE_INTEGRALTEMPLATEDTUPLE_HPP
#define HYPERTRIE_INTEGRALTEMPLATEDTUPLE_HPP

#include "Dice/hypertrie/internal/util/NTuple.hpp"
#include <cmath>
#include <concepts>
#include <cstdint>
#include <functional>
#include <tuple>

namespace hypertrie::internal::util {

	namespace count_tuple_internal {

		template<template<auto X> typename T, std::uintptr_t N, std::intptr_t MAX, std::intptr_t MIN, bool count_up, bool use_signed,
				 template<typename...> class TT>
		struct repeat {
			static constexpr const std::intptr_t I = (count_up) ? MAX - N + 1 : MIN + N - 1;
			using type = typename append_to_type_seq<
					std::conditional_t<(use_signed), T<I>, T<(I < 0) ? 0u : unsigned(I)>>,
					typename repeat<T, N - 1, MAX, MIN, count_up, use_signed, TT>::type>::type;
		};

		template<template<auto X> typename T, std::intptr_t MAX, std::intptr_t MIN, bool count_up, bool use_signed,
				 template<typename...> class TT>
		struct repeat<T, 0, MAX, MIN, count_up, use_signed, TT> {
			using type = TT<>;
		};

		template<template<auto I> typename T, auto FIRST, auto LAST>
		struct x {
			static constexpr const bool count_up = (std::intptr_t(FIRST) < std::intptr_t(LAST));
			static constexpr const auto MIN = (count_up) ? std::intptr_t(FIRST) : std::intptr_t(LAST);
			static constexpr const auto MAX = (count_up) ? std::intptr_t(LAST) : std::intptr_t(FIRST);
			static constexpr const auto N = MAX - MIN + 1L;
			static constexpr const bool use_signed = (MAX < 0 or MIN < 0);
			using type = typename repeat<T, N, MAX, MIN, count_up, use_signed, std::tuple>::type;
		};
	}// namespace count_tuple_internal


	/**
	 * This class is a wrapper around a tuple std::tuple<T<FIRST> .. T<LAST>>. It allows access to the elements via get<i>() -> T<I>. Elements are memory aligned from FIRST to LAST which means you can reinterpret a IntegralTemplatedTuple<T,1,5> as a IntegralTemplatedTuple<T,1,3> and will still be able to access the elements 1-3.
	 * @tparam T
	 * @tparam FIRST
	 * @tparam LAST
	 */
	template<template<std::integral auto> typename T, auto FIRST = 0, auto LAST = 0>
	class IntegralTemplatedTuple {
		typename count_tuple_internal::x<T, FIRST, LAST>::type count_up_tuple_;

		constexpr static auto abs(std::intptr_t a, std::intptr_t b) {
			if (a < b) return b - a;
			else
				return a - b;
		}

	public:
		template<auto I>
		constexpr auto &get() noexcept {
			constexpr const auto pos = (FIRST <= LAST) ? LAST - I : I - LAST;
			static_assert(pos >= 0 and pos <= abs(LAST, FIRST));
			return std::get<pos>(count_up_tuple_);
		}

		template<auto I>
		constexpr const auto &get() const noexcept {
			constexpr const auto pos = (FIRST <= LAST) ? LAST - I : I - LAST;
			static_assert(pos >= 0 and pos <= abs(LAST, FIRST));
			return std::get<pos>(count_up_tuple_);
		}
	};
}// namespace hypertrie::internal::util


#endif//HYPERTRIE_INTEGRALTEMPLATEDTUPLE_HPP
