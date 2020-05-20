#ifndef HYPERTRIE_HYPERTRIE_INTERNAL_TRAITS_HPP
#define HYPERTRIE_HYPERTRIE_INTERNAL_TRAITS_HPP

#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"

#include "Dice/hypertrie/internal/util/RawKey.hpp"

namespace hypertrie::internal::node_based {



	template<HypertrieTrait tr_t = default_bool_Hypertrie_t>
	struct Hypertrie_internal_t {
		using tr = tr_t;
		/// public definitions
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		template<typename key, typename value>
		using map_type = typename tr::template map_type<key, value>;
		template<typename key>
		using set_type = typename tr::template set_type<key>;

		using SliceKey = typename tr::SliceKey;
		using Key = typename tr::Key;
		/// internal definitions
		template<size_t depth>
		using RawKey = hypertrie::internal::RawKey<depth, typename tr::key_part_type>;

		template<size_t depth>
		using RawSliceKey = hypertrie::internal::RawSliceKey<depth, typename tr::key_part_type>;

		constexpr static bool is_bool_valued = std::is_same_v<value_type, bool>;

		template<size_t depth>
		static auto subkey(const RawKey<depth> &key, size_t remove_pos) -> RawKey<depth - 1> {
			RawKey<depth - 1> sub_key;
			for (size_t i = 0, j = 0; i < depth; ++i)
				if (i != remove_pos) sub_key[j++] = key[i];
			return sub_key;
		}
	};

	namespace internal::hypertrie_internal_trait {
		template<typename T, template<HypertrieTrait> typename U>
		struct is_instance_impl : public std::false_type {
		};

		template<template <HypertrieTrait> typename U,
		HypertrieTrait tr_t>
		struct is_instance_impl<U<tr_t>, U> : public std::true_type {
		};

		template<typename T, template<HypertrieTrait> typename U>
		using is_instance = is_instance_impl<std::decay_t<T>, U>;
	}


	template<class T>
	concept HypertrieInternalTrait = internal::hypertrie_internal_trait::is_instance<T, Hypertrie_internal_t>::value;

	using default_bool_Hypertrie_internal_t = Hypertrie_internal_t<default_bool_Hypertrie_t>;

	using default_long_Hypertrie_internal_t = Hypertrie_internal_t<default_long_Hypertrie_t>;

	using default_double_Hypertrie_internal_t = Hypertrie_internal_t<default_double_Hypertrie_t>;
};

#endif //HYPERTRIE_HYPERTRIE_INTERNAL_TRAITS_HPP
