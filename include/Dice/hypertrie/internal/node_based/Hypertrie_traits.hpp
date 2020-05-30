#ifndef HYPERTRIE_HYPERTRIE_TRAIT_HPP
#define HYPERTRIE_HYPERTRIE_TRAIT_HPP

#include <vector>
#include <optional>
#include "Dice/hypertrie/internal/container/AllContainer.hpp"
#include "Dice/hypertrie/internal/util/Key.hpp"

namespace hypertrie::internal::node_based {

	template<typename key_part_type_t,
			typename value_type_t,
			template<typename, typename> class map_type_t,
			template<typename> class set_type_t>
	struct Hypertrie_t {
		using key_part_type = key_part_type_t;
		using value_type = value_type_t;
		template<typename key, typename value>
		using map_type = map_type_t<key, value>;
		template<typename key>
		using set_type = set_type_t<key>;

		using SliceKey = ::hypertrie::SliceKey<key_part_type>;
		using Key = ::hypertrie::Key<key_part_type>;
	};

	namespace internal::hypertrie_trait {
		template<typename T, template<typename,
				typename,
				template<typename, typename> class,
				template<typename> class> typename U>
		struct is_instance_impl : public std::false_type {
		};

		template<template<typename,
				typename,
				template<typename, typename> class,
				template<typename> class> typename U,
				typename key_part_type_t,
				typename value_type_t,
				template<typename, typename> class map_type_t,
				template<typename> class set_type_t>
		struct is_instance_impl<U<key_part_type_t, value_type_t, map_type_t, set_type_t>, U> : public std::true_type {
		};

		template<typename T, template<typename,
				typename,
				template<typename, typename> class,
				template<typename> class> typename U>
		using is_instance = is_instance_impl<std::decay_t<T>, U>;
	}


	template<class T>
	concept HypertrieTrait = internal::hypertrie_trait::is_instance<T, Hypertrie_t>::value;

	using default_bool_Hypertrie_t = Hypertrie_t<unsigned long,
			bool,
			hypertrie::internal::container::std_map,
			hypertrie::internal::container::std_set>;

	using default_long_Hypertrie_t = Hypertrie_t<unsigned long,
			long,
			hypertrie::internal::container::std_map,
			hypertrie::internal::container::std_set>;

	using default_double_Hypertrie_t = Hypertrie_t<unsigned long,
			double,
			hypertrie::internal::container::std_map,
			hypertrie::internal::container::std_set>;

};

#endif //HYPERTRIE_HYPERTRIE_TRAIT_HPP
