#ifndef HYPERTRIE_HYPERTRIE_TRAIT_HPP
#define HYPERTRIE_HYPERTRIE_TRAIT_HPP

#include <optional>
#include <string>
#include <vector>

namespace dice::hypertrie {

	template<typename key_part_type_t,
			 typename value_type_t,
			 template<typename, typename, typename> class map_type_t,
			 template<typename, typename> class set_type_t,
			 ssize_t key_part_tagging_bit_v = -1>
	struct Hypertrie_t {
		using key_part_type = key_part_type_t;
		using value_type = value_type_t;


		template<typename key, typename value, typename allocator_type>
		using map_type = map_type_t<key, value, allocator_type>;
		template<typename key, typename allocator_type>
		using set_type = set_type_t<key, allocator_type>;

		static constexpr bool is_bool_valued = std::is_same_v<value_type, bool>;
		static constexpr ssize_t key_part_tagging_bit = key_part_tagging_bit_v;
		static constexpr bool taggable_key_part = key_part_tagging_bit != -1;
	};

	namespace internal::hypertrie_trait {
		template<typename T, template<typename,
									  typename,
									  template<typename, typename, typename> class,
									  template<typename, typename> class,
									  ssize_t>
							 typename U>
		struct is_instance_impl : public std::false_type {
		};

		template<template<typename,
						  typename,
						  template<typename, typename, typename> class,
						  template<typename, typename> class,
						  ssize_t>
				 typename U,
				 typename key_part_type_t,
				 typename value_type_t,
				 template<typename, typename, typename> class map_type_t,
				 template<typename, typename> class set_type_t,
				 ssize_t value_type_tagging_bit_v>
		struct is_instance_impl<U<key_part_type_t, value_type_t, map_type_t, set_type_t, value_type_tagging_bit_v>, U> : public std::true_type {
		};

		template<typename T, template<typename,
									  typename,
									  template<typename, typename, typename> class,
									  template<typename, typename> class,
									  ssize_t>
							 typename U>
		using is_instance = is_instance_impl<std::decay_t<T>, U>;
	}// namespace internal::hypertrie_trait


	template<class T>
	concept HypertrieTrait = internal::hypertrie_trait::is_instance<T, Hypertrie_t>::value && requires(T t) {
		typename T::key_part_type;
		typename T::value_type;
		{ T::is_bool_valued } -> std::convertible_to<bool>;
		{ T::key_part_tagging_bit } -> std::convertible_to<ssize_t>;
		{ T::taggable_key_part } -> std::convertible_to<bool>;
	};

	template<class T>
	concept HypertrieTrait_bool_valued = HypertrieTrait<T> and T::is_bool_valued;

	template<class T>
	concept HypertrieTrait_bool_valued_and_taggable_key_part = HypertrieTrait_bool_valued<T> and T::taggable_key_part;
};// namespace dice::hypertrie

#endif//HYPERTRIE_HYPERTRIE_TRAIT_HPP
