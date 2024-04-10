#ifndef HYPERTRIE_EINSUMENTRY_HPP
#define HYPERTRIE_EINSUMENTRY_HPP

#include <dice/hypertrie/Hypertrie_trait.hpp>
#include <dice/hypertrie/Key.hpp>

namespace dice::einsum {

	namespace detail {
		using namespace dice::hypertrie;
		template<typename new_value_type,
				 typename key_part_type_o,
				 typename value_type_o,
				 template<typename, typename, typename> class map_type_o,
				 template<typename, typename> class set_type_o,
				 ssize_t key_part_tagging_bit_v_o>
		constexpr auto inject_value_type(Hypertrie_t<key_part_type_o, value_type_o, map_type_o, set_type_o, key_part_tagging_bit_v_o>) {
			return Hypertrie_t<key_part_type_o, new_value_type, map_type_o, set_type_o, key_part_tagging_bit_v_o>{};
		}


	}// namespace detail

	template<typename value_type, hypertrie::HypertrieTrait htt_t>
	using tri_with_value_type = decltype(detail::inject_value_type<value_type>(htt_t{}));

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t>
	using Entry = ::dice::hypertrie::Entry<tri_with_value_type<value_type, htt_t>>;

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t>
	using Key = ::dice::hypertrie::Key<tri_with_value_type<value_type, htt_t>>;
}// namespace dice::einsum
#endif//HYPERTRIE_EINSUMENTRY_HPP
