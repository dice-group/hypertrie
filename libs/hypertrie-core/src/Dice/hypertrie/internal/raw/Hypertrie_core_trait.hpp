#ifndef HYPERTRIE_HYPERTRIE_CORE_TRAIT_HPP
#define HYPERTRIE_HYPERTRIE_CORE_TRAIT_HPP

#include <Dice/hypertrie/Hypertrie_trait.hpp>
#include <bitset>
#include <enumerate.hpp>
#include <map>
#include <zip.hpp>

namespace Dice::hypertrie::internal::raw {


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
	concept HypertrieCoreTrait = internal::hypertrie_internal_trait::is_instance<T, Hypertrie_core_t>::value && requires(T t) {
		typename T::tr;
		typename T::allocator_type;
		typename T::key_part_type;
		typename T::value_type;
		{ T::is_bool_valued } -> std::convertible_to<bool>;
		{ T::key_part_tagging_bit } -> std::convertible_to<ssize_t>;
		{ T::taggable_key_part } -> std::convertible_to<bool>;
	};

	template<class T>
	concept HypertrieCoreTrait_bool_valued = HypertrieCoreTrait<T> and T::is_bool_valued;

	template<class T>
	concept HypertrieCoreTrait_bool_valued_and_taggable_key_part = HypertrieCoreTrait_bool_valued<T> and T::taggable_key_part;


	namespace detail {
		template<typename NewAllocator,
				 typename key_part_type_o,
				 typename value_type_o,
				 typename Allocator_o,
				 template<typename, typename, typename> class map_type_o,
				 template<typename, typename> class set_type_o,
				 ssize_t key_part_tagging_bit_v_o>
		constexpr auto inject_allocator_type(Hypertrie_core_t<Hypertrie_t<key_part_type_o, value_type_o, Allocator_o, map_type_o, set_type_o, key_part_tagging_bit_v_o>>) {
			return Hypertrie_core_t<Hypertrie_t<key_part_type_o, value_type_o, NewAllocator, map_type_o, set_type_o, key_part_tagging_bit_v_o>>{};
		}
	}// namespace detail

	template<HypertrieCoreTrait tri>
	using tri_with_stl_alloc = decltype(detail::inject_allocator_type<std::allocator<std::byte>>(tri{}));

};// namespace Dice::hypertrie::internal::raw

#endif//HYPERTRIE_HYPERTRIE_CORE_TRAIT_HPP
