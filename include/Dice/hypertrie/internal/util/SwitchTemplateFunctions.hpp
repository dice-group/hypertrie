#ifndef HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP
#define HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP

#include <concepts>
#include <initializer_list>
#include <iostream>
#include <utility>

namespace hypertrie::internal {
	template<int max, int min = 0>
	struct compiled_switch {
	private:
		template<class T, T... Is, class F>
		static auto execute_switch(T i, std::integer_sequence<T, Is...>, F f) {
			using return_type = std::decay_t<std::common_type_t<decltype(f(std::integral_constant<T, Is + min>{}))...>>;
			if constexpr (std::is_same_v<return_type, void>){
				std::initializer_list<int>({(i == Is + min ? (f(std::integral_constant<T, Is + min>{})), 0 : 0)...});
			} else {
				return_type ret;
				std::initializer_list<int>({(i == Is + min ? (ret = f(std::integral_constant<T, Is + min>{})), 0 : 0)...});
				return ret;
			}
		}

		template<class T, T... Is, class F>
		static auto execute_switch_void([[maybe_unused]] T i, std::integer_sequence<T, Is...>, [[maybe_unused]] F f) {
				std::initializer_list<int>({(i == Is + min ? (f(std::integral_constant<T, Is + min>{})), 0 : 0)...});
		}

	public:
		template<class F, class D>
		static auto switch_(size_t x, F switch_function, D default_function) {
			if (x < min or max <= x)
				return default_function();
			else
				return execute_switch(x, std::make_index_sequence<max - min>{}, switch_function);
		}

		template<class F, class D>
		static void switch_void(size_t x, F switch_function, D default_function) {
			if (x < min or max <= x)
				default_function();
			else
				execute_switch(x, std::make_index_sequence<max - min>{}, switch_function);
		}

		template<class F>
		static void switch_void(size_t x, F switch_function) {
			if (x >= min or max > x)
				execute_switch_void(x, std::make_index_sequence<max - min>{}, switch_function);
		}
	};
}// namespace hypertrie::internal

#endif//HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP
