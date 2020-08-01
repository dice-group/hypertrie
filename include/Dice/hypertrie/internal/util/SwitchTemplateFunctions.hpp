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
			using return_type = std::common_type_t<decltype(f(std::integral_constant<T, Is + min>{}))...>;
			return_type ret;
			std::initializer_list<int>({(i == Is + min ? (ret = f(std::integral_constant<T, Is + min>{})), 0 : 0)...});
			return ret;
		}

	public:
		template<class F, class D>
		static auto switch_(int x, F switch_function, D default_function) {
			if (x < min or max <= x)
				return default_function();
			else
				return execute_switch(x, std::make_index_sequence<max - min>{}, switch_function);
		}
	};
}// namespace hypertrie::internal

#endif//HYPERTRIE_SWITCHTEMPLATEFUNCTIONS_HPP
