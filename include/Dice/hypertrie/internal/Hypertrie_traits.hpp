#ifndef HYPERTRIE_HYPERTRIE_TRAIT_HPP
#define HYPERTRIE_HYPERTRIE_TRAIT_HPP

#include <optional>
#include <vector>
#include <string>

#include <fmt/format.h>
#include <boost/type_index.hpp>

#include "Dice/hypertrie/internal/container/AllContainer.hpp"
#include "Dice/hypertrie/internal/util/Key.hpp"

namespace hypertrie {

	template<typename key_part_type_t = unsigned long,
			 typename value_type_t = bool,
			 template<typename, typename, typename> class map_type_t = hypertrie::internal::container::tsl_sparse_map,
			 template<typename, typename> class set_type_t = hypertrie::internal::container::tsl_sparse_set,
			 bool lsb_unused_v = false>
	struct Hypertrie_t {
		using key_part_type = key_part_type_t;
		using value_type = value_type_t;
		template<typename key, typename value, typename Allocator>
		using map_type = map_type_t<key, value, Allocator>;
		template<typename key, typename Allocator>
		using set_type = set_type_t<key, Allocator>;

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

			static auto value(IteratorEntry &entry) noexcept {
				if constexpr (is_bool_valued)
					return true;
				else
					return entry.second;
			}
		};

		using KeyPositions = std::vector<uint8_t>;

	private:
		template<typename T>
		static std::string nameOfType() {
			std::string string = boost::typeindex::type_id<T>().pretty_name();
			auto pos = string.find('<');
			return string.substr(0,pos);
		}

	public:
		static auto to_string() {
			return std::string{fmt::format(
					"< key_part = {}, value = {}, map = {}, set = {}, lsb_unused = {} >",
					nameOfType<key_part_type>(),
					nameOfType<value_type>(),
					nameOfType<map_type<key_part_type, value_type, std::allocator<std::pair<const key_part_type, value_type>>>>(),
					nameOfType<set_type<key_part_type, std::allocator<key_part_type>>>(),
					lsb_unused)};
		}
	};

	namespace internal::hypertrie_trait {
		template<typename T, template<typename,
									  typename,
									  template<typename, typename, typename> class,
									  template<typename, typename> class,
									  bool>
							 typename U>
		struct is_instance_impl : public std::false_type {
		};

		template<template<typename,
						  typename,
						  template<typename, typename, typename> class,
						  template<typename, typename> class,
						  bool>
				 typename U,
				 typename key_part_type_t,
				 typename value_type_t,
				 template<typename, typename, typename> class map_type_t,
				 template<typename, typename> class set_type_t,
				 bool lsb_unused_v>
		struct is_instance_impl<U<key_part_type_t, value_type_t, map_type_t, set_type_t, lsb_unused_v>, U> : public std::true_type {
		};

		template<typename T, template<typename,
									  typename,
									  template<typename, typename, typename> class,
									  template<typename, typename> class,
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
	using lsbunused_bool_Hypertrie_t = Hypertrie_t<unsigned long,
												   bool,
												   hypertrie::internal::container::tsl_sparse_map,
												   hypertrie::internal::container::tsl_sparse_set,
												   true>;

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
