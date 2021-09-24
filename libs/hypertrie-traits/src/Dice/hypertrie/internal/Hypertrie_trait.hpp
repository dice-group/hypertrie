#ifndef HYPERTRIE_HYPERTRIE_TRAIT_HPP
#define HYPERTRIE_HYPERTRIE_TRAIT_HPP

#include <optional>
#include <string>
#include <vector>

namespace hypertrie::internal {

	template<typename key_part_type_t,
			 typename value_type_t,
			 typename Allocator,
			 template<typename, typename, typename> class map_type_t,
			 template<typename, typename> class set_type_t,
			 ssize_t key_part_tagging_bit_v = -1>
	struct Hypertrie_t {
		using key_part_type = key_part_type_t;
		using value_type = value_type_t;
		using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<std::byte>;

		template<typename key, typename value>
		using map_type = map_type_t<key, value, allocator_type>;
		template<typename key>
		using set_type = set_type_t<key, allocator_type>;

		template<typename key, typename value, typename allocator_type>
		using map_type_arg = map_type_t<key, value, allocator_type>;
		template<typename key, typename allocator_type>
		using set_type_arg = set_type_t<key, allocator_type>;

		static constexpr const bool is_bool_valued = std::is_same_v<value_type, bool>;
		static constexpr const ssize_t key_part_tagging_bit = key_part_tagging_bit_v;
		static constexpr const bool taggable_key_part = key_part_tagging_bit != -1;
	};

	namespace internal::hypertrie_trait {
		template<typename T, template<typename,
									  typename,
									  typename,
									  template<typename, typename, typename> class,
									  template<typename, typename> class,
									  ssize_t>
							 typename U>
		struct is_instance_impl : public std::false_type {
		};

		template<template<typename,
						  typename,
						  typename,
						  template<typename, typename, typename> class,
						  template<typename, typename> class,
						  ssize_t>
				 typename U,
				 typename key_part_type_t,
				 typename value_type_t,
				 typename allocator_t,
				 template<typename, typename, typename> class map_type_t,
				 template<typename, typename> class set_type_t,
				 ssize_t value_type_tagging_bit_v>
		struct is_instance_impl<U<key_part_type_t, value_type_t, allocator_t, map_type_t, set_type_t, value_type_tagging_bit_v>, U> : public std::true_type {
		};

		template<typename T, template<typename,
									  typename,
									  typename,
									  template<typename, typename, typename> class,
									  template<typename, typename> class,
									  ssize_t>
							 typename U>
		using is_instance = is_instance_impl<std::decay_t<T>, U>;
	}// namespace internal::hypertrie_trait


	template<class T>
	concept HypertrieTrait = internal::hypertrie_trait::is_instance<T, Hypertrie_t>::value;
};// namespace hypertrie::internal

#endif//HYPERTRIE_HYPERTRIE_TRAIT_HPP
