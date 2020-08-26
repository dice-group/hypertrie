#ifndef HYPERTRIE_HYPERTRIE_TRAIT_HPP
#define HYPERTRIE_HYPERTRIE_TRAIT_HPP

#include <optional>
#include <vector>

#include "Dice/hypertrie/internal/container/AllContainer.hpp"
#include "Dice/hypertrie/internal/util/Key.hpp"

namespace hypertrie {

	template<typename key_part_type_t = unsigned long,
			 typename value_type_t = bool,
			 template<typename, typename> class map_type_t = hypertrie::internal::container::tsl_sparse_map,
			 template<typename> class set_type_t = hypertrie::internal::container::tsl_sparse_set,
			 bool lsb_unused_v = false>
	struct Hypertrie_t {
		using key_part_type = key_part_type_t;
		using value_type = value_type_t;
		template<typename key, typename value>
		using map_type = map_type_t<key, value>;
		template<typename key>
		using set_type = set_type_t<key>;

		using SliceKey = ::hypertrie::SliceKey<key_part_type>;
		using Key = ::hypertrie::Key<key_part_type>;

		static constexpr const bool is_bool_valued = std::is_same_v<value_type, bool>;
		static constexpr const bool lsb_unused = lsb_unused_v;

		using IteratorEntry = std::conditional_t<(is_bool_valued), Key, std::pair<Key, value_type>>;

		struct iterator_entry{
			static Key &key(IteratorEntry &entry) noexcept {
				if constexpr (is_bool_valued) return entry;
				else
					return entry.first;
			}

			template<typename = std::enable_if_t<(is_bool_valued)>>
			static value_type &value(IteratorEntry &entry) noexcept { return entry.second; }
		};

		using KeyPositions = std::vector<uint8_t>;
	};

	namespace internal::hypertrie_trait {
		template<typename T, template<typename,
									  typename,
									  template<typename, typename> class,
									  template<typename> class,
									  bool>
							 typename U>
		struct is_instance_impl : public std::false_type {
		};

		template<template<typename,
						  typename,
						  template<typename, typename> class,
						  template<typename> class,
						  bool>
				 typename U,
				 typename key_part_type_t,
				 typename value_type_t,
				 template<typename, typename> class map_type_t,
				 template<typename> class set_type_t,
				bool lsb_unused_v>
		struct is_instance_impl<U<key_part_type_t, value_type_t, map_type_t, set_type_t, lsb_unused_v>, U> : public std::true_type {
		};

		template<typename T, template<typename,
									  typename,
									  template<typename, typename> class,
									  template<typename> class,
									  bool>
							 typename U>
		using is_instance = is_instance_impl<std::decay_t<T>, U>;
	}// namespace internal::hypertrie_trait


	template<class T>
	concept HypertrieTrait = internal::hypertrie_trait::is_instance<T, Hypertrie_t>::value;

	using default_bool_Hypertrie_t = Hypertrie_t<unsigned long,
												 bool,
												 hypertrie::internal::container::tsl_sparse_map ,
												 hypertrie::internal::container::tsl_sparse_set >;

	using default_long_Hypertrie_t = Hypertrie_t<unsigned long,
												 long,
												 hypertrie::internal::container::tsl_sparse_map,
												 hypertrie::internal::container::tsl_sparse_set>;

	using default_double_Hypertrie_t = Hypertrie_t<unsigned long,
												   double,
												   hypertrie::internal::container::tsl_sparse_map,
												   hypertrie::internal::container::tsl_sparse_set>;

};// namespace hypertrie

#endif//HYPERTRIE_HYPERTRIE_TRAIT_HPP
