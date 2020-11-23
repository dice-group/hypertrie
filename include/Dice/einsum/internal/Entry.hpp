#ifndef HYPERTRIE_EINSUMENTRY_HPP
#define HYPERTRIE_EINSUMENTRY_HPP
//#define DEBUGEINSUM
#ifdef DEBUGEINSUM
#include <fmt/format.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>
constexpr bool _debugeinsum_ = true;
#else
constexpr bool _debugeinsum_ = false;
#endif

#include "Dice/hypertrie/hypertrie.hpp"
#include <Dice/hypertrie/internal/Hypertrie.hpp>
#include <boost/container_hash/hash.hpp>
#include <type_traits>


namespace einsum::internal {

	template<class T>
	concept HypertrieTrait = hypertrie::internal::hypertrie_trait::is_instance<T, hypertrie::Hypertrie_t>::value;

	template<HypertrieTrait tr>
	using const_Hypertrie = typename hypertrie::const_Hypertrie<tr>;
	template<HypertrieTrait tr>
	using Hypertrie = typename hypertrie::Hypertrie<tr>;

	template<typename key_part_type>
	using Key = std::vector<key_part_type>;

	template<typename T, typename =std::enable_if_t<(not std::is_same_v<std::decay_t<T>, bool>)>>
	struct KeyHash {
		std::size_t operator()(const ::einsum::internal::Key<T> &k) const {
			//return ::hypertrie::internal::robin_hood::rh_hash(k);
            return dice::hash::dice_hash(k);
		}
	};

	template<typename value_type_t, HypertrieTrait tr_t, typename = std::enable_if_t<(std::is_integral_v<value_type_t>)>>
	struct Entry {
		using tr = tr_t;
		using key_part_type = typename tr::key_part_type;
		using Key = typename tr::Key;
		using value_type = value_type_t;

		value_type value;
		Key key;
	};

}
#endif //HYPERTRIE_EINSUMENTRY_HPP

