#ifndef HYPERTRIE_HYPERTRIE_CORE_TRAIT_HPP
#define HYPERTRIE_HYPERTRIE_CORE_TRAIT_HPP

#include <Dice/hypertrie/internal/Hypertrie_trait.hpp>
#include <bitset>
#include <enumerate.hpp>
#include <map>
#include <zip.hpp>

namespace hypertrie::internal::raw {


	template<HypertrieTrait tr_t>
	struct Hypertrie_core_t {
		using tr = tr_t;
		/// public definitions
		using allocator_type = typename tr::allocator_type;

		template<typename PointedType>
		using allocator_pointer = typename std::pointer_traits<typename std::allocator_traits<allocator_type>::pointer>::template rebind<PointedType>;

		template<typename PointedType>
		using allocator_const_pointer = typename std::pointer_traits<typename std::allocator_traits<allocator_type>::const_pointer>::template rebind<PointedType>;

		template<typename T>
		using rebind_alloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<T>;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;

		template<typename key, typename value>
		using map_type = typename tr::template map_type<key, value>;
		template<typename key>
		using set_type = typename tr::template set_type<key>;

		/// internal definitions

		constexpr static bool is_bool_valued = tr::is_bool_valued;
		static constexpr const ssize_t key_part_tagging_bit = tr::key_part_tagging_bit;
		static constexpr const bool taggable_key_part = tr::taggable_key_part;

		template<typename other_allocator>
		using change_allocator = Hypertrie_core_t<Hypertrie_t<key_part_type,
															  value_type,
															  other_allocator,
															  tr::template map_type_arg,
															  tr::template set_type_arg,
															  key_part_tagging_bit>>;
		using with_std_allocator = change_allocator<std::allocator<std::byte>>;
	};

	namespace internal::hypertrie_internal_trait {
		template<typename T, template<typename> typename U>
		struct is_instance_impl : public std::false_type {};

		template<template<HypertrieTrait> typename U,
				 HypertrieTrait tr_t>
		struct is_instance_impl<U<tr_t>, U> : public std::true_type {
		};

		template<typename T, template<HypertrieTrait> typename U>
		using is_instance = is_instance_impl<std::decay_t<T>, U>;
	}// namespace internal::hypertrie_internal_trait


	template<class T>
	concept HypertrieCoreTrait = internal::hypertrie_internal_trait::is_instance<T, Hypertrie_core_t>::value;
};// namespace hypertrie::internal::raw

#endif//HYPERTRIE_HYPERTRIE_CORE_TRAIT_HPP
