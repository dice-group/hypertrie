#ifndef HYPERTRIE_HYPERTRIE_TRAIT_HPP
#define HYPERTRIE_HYPERTRIE_TRAIT_HPP

#include <optional>
#include <string>
#include <vector>

#include "dice/hash.hpp"

namespace dice::hypertrie {

	/**
	 *
	 * @tparam key_part_type_ the hypertries key part type
	 * @tparam value_type_ the hypertries value type
	 * @tparam map_type_ a `std::map` like type to be used as a map (this should typically be a hash map)
	 * @tparam set_type_ a `std::set` like type to be used as a set (this should typically be a hash set)
	 * @tparam taggable_key_part_ enables an optimization that saves space in depth-1 SENs
	 * 		enabling this requires the following:
	 * 		1.1. if your allocator uses raw pointers key_part_type_ cannot use its  2 most significant bits
	 * 		1.2. if your allocator uses offset pointers key_part_type_ cannot use its 3 most significant bits
	 * 		2. key_part_type must be able to be cast from and to 64-bit integers
	 */
	template<typename key_part_type_,
			 typename value_type_,
			 template<typename mkey_, typename mvalue_, typename mhash_, typename mequal_, typename malloc_> typename map_type_,
			 template<typename skey_, typename shash_, typename sequal_, typename salloc_> typename set_type_,
			 bool taggable_key_part_ = false>
	struct Hypertrie_t {
		using key_part_type = key_part_type_;
		using value_type = value_type_;

		template<typename key, typename value, typename allocator_type, typename hash = dice::hash::DiceHash<key, hash::Policies::wyhash>, typename equal = std::equal_to<key>>
		using map_type = map_type_<key, value, hash, equal, allocator_type>;

		template<typename key, typename allocator_type, typename hash = dice::hash::DiceHash<key, hash::Policies::wyhash>, typename equal = std::equal_to<key>>
		using set_type = set_type_<key, hash, equal, allocator_type>;

		static constexpr bool is_bool_valued = std::is_same_v<value_type, bool>;
		static constexpr bool taggable_key_part = taggable_key_part_;
	};

	namespace internal::hypertrie_trait {
		template<typename T,
				 template<typename key_part_type_,
						  typename value_type_,
						  template<typename mkey_, typename mvalue_, typename mhash_, typename mequal_, typename malloc_> typename map_type_,
						  template<typename skey_, typename shash_, typename sequal_, typename salloc_> typename set_type_,
						  bool taggable_key_part_> typename U>
		struct is_instance_impl : public std::false_type {
		};

		template<template<typename key_part_type_,
						  typename value_type_,
						  template<typename mkey_, typename mvalue_, typename mhash_, typename mequal_, typename malloc_> typename map_type_,
						  template<typename skey_, typename shash_, typename sequal_, typename salloc_> typename set_type_,
						  bool taggable_key_part_> typename U,
				 typename key_part_type_t,
				 typename value_type_t,
				 template<typename mkey_, typename mvalue_, typename mhash_, typename mequal_, typename malloc_> typename map_type_t,
				 template<typename skey_, typename shash_, typename sequal_, typename salloc_> typename set_type_t,
				 bool taggable_key_part_v>
		struct is_instance_impl<U<key_part_type_t, value_type_t, map_type_t, set_type_t, taggable_key_part_v>, U> : public std::true_type {
		};

		template<typename T,
				 template<typename key_part_type_,
						  typename value_type_,
						  template<typename mkey_, typename mvalue_, typename mhash_, typename mequal_, typename malloc_> typename map_type_,
						  template<typename skey_, typename shash_, typename sequal_, typename salloc_> typename set_type_,
						  bool taggable_key_part_> typename U>
		using is_instance = is_instance_impl<std::decay_t<T>, U>;
	}// namespace internal::hypertrie_trait


	template<typename T>
	concept HypertrieValueType = requires (T val1, T val2) {
		// additive identity element must exist
		T{};

		// multiplicative identity element must exist
		T{1};

		// hashable with dice_hash
		hash::dice_hash_templates<hash::Policies::wyhash>::dice_hash(val1);

		// spaceship
		{ val1 <=> val2 } -> std::convertible_to<std::partial_ordering>;
	};

	template<typename T>
	concept HypertrieTrait = internal::hypertrie_trait::is_instance<T, Hypertrie_t>::value && requires(T t) {
		typename T::key_part_type;
		requires HypertrieValueType<typename T::value_type>;
		{ T::is_bool_valued } -> std::convertible_to<bool>;
		{ T::taggable_key_part } -> std::convertible_to<bool>;
	};

	template<typename T>
	concept HypertrieTrait_bool_valued = HypertrieTrait<T> and T::is_bool_valued;

	template<typename T>
	concept HypertrieTrait_taggable_key_part = HypertrieTrait<T> and T::taggable_key_part;

	template<typename T>
	concept HypertrieTrait_bool_valued_and_taggable_key_part = HypertrieTrait_bool_valued<T> and T::taggable_key_part;
};// namespace dice::hypertrie

#endif//HYPERTRIE_HYPERTRIE_TRAIT_HPP
