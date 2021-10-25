#ifndef HYPERTRIE_EINSUMENTRY_HPP
#define HYPERTRIE_EINSUMENTRY_HPP

#include <Dice/hypertrie/Hypertrie_trait.hpp>
#include <Dice/hypertrie/Key.hpp>
//#define DEBUGEINSUM
#ifdef DEBUGEINSUM
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/printf.h>
#include <fmt/ranges.h>
constexpr bool _debugeinsum_ = true;
#else
constexpr bool _debugeinsum_ = false;
#endif

namespace Dice::einsum::internal {

	namespace detail {
		using namespace Dice::hypertrie;
		template<typename new_value_type,
				 typename key_part_type_o,
				 typename value_type_o,
				 typename Allocator_o,
				 template<typename, typename, typename> class map_type_o,
				 template<typename, typename> class set_type_o,
				 ssize_t key_part_tagging_bit_v_o>
		constexpr auto inject_value_type(Hypertrie_t<key_part_type_o, value_type_o, Allocator_o, map_type_o, set_type_o, key_part_tagging_bit_v_o>) {
			return Hypertrie_t<key_part_type_o, new_value_type, Allocator_o, map_type_o, set_type_o, key_part_tagging_bit_v_o>{};
		}


	}// namespace detail

	template<typename value_type, hypertrie::HypertrieTrait tr>
	using tri_with_value_type = decltype(detail::inject_value_type<value_type>(tr{}));

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued tr>
	using Entry = ::Dice::hypertrie::NonZeroEntry<tri_with_value_type<value_type, tr>>;

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued tr>
	using Key = ::Dice::hypertrie::Key<tri_with_value_type<value_type, tr>>;


}// namespace Dice::einsum::internal
#endif//HYPERTRIE_EINSUMENTRY_HPP
