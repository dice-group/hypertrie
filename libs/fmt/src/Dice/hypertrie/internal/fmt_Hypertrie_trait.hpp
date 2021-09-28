#ifndef HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP
#define HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP

#include <Dice/hypertrie/internal/Hypertrie_trait.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template<hypertrie::internal::HypertrieTrait hypertrie_trait>
	struct formatter<hypertrie_trait> : public hypertrie::internal::util::SimpleParsing {
		using key_part_type = typename hypertrie_trait::key_part_type;
		using value_type = typename hypertrie_trait::value_type;
		template<typename T>
		static auto nameOfType() {
			return ::hypertrie::internal::util::name_of_type<T>();
		}

		template<typename FormatContext>
		auto format(hypertrie_trait const &, FormatContext &ctx) {
			return format_to(ctx.out(),
							 "<key_part = {}, value = {}, allocator = {}, map = {}, set = {}, key_part_tagging_bit = {}>",
							 nameOfType<key_part_type>(), nameOfType<value_type>(),
							 nameOfType<typename hypertrie_trait::allocator_type>(),
							 nameOfType<typename hypertrie_trait::template map_type<key_part_type, value_type>>(),
							 nameOfType<typename hypertrie_trait::template set_type<key_part_type>>(),
							 hypertrie_trait::key_part_tagging_bit);
		}
	};
}// namespace fmt

#endif//HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP