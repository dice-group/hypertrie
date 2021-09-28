#ifndef HYPERTRIE_HYPERTRIE_CORE_TRAITS_TOSTRING_HPP
#define HYPERTRIE_HYPERTRIE_CORE_TRAITS_TOSTRING_HPP

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template<hypertrie::internal::raw::HypertrieCoreTrait trait>
	struct formatter<trait> : public hypertrie::internal::util::SimpleParsing {
		using key_part_type = typename trait::key_part_type;
		using value_type = typename trait::value_type;
		template<typename T>
		static auto nameOfType() {
			return ::hypertrie::internal::util::name_of_type<T>();
		}

		template<typename FormatContext>
		auto format(trait const &, FormatContext &ctx) {
			return format_to(ctx.out(),
							 "<key_part = {}, value = {}, allocator = {}, map = {}, set = {}, key_part_tagging_bit = {}>",
							 nameOfType<key_part_type>(), nameOfType<value_type>(),
							 nameOfType<typename trait::allocator_type>(),
							 nameOfType<typename trait::template map_type<key_part_type, value_type>>(),
							 nameOfType<typename trait::template set_type<key_part_type>>(),
							 trait::key_part_tagging_bit);
		}
	};
}// namespace fmt

#endif//HYPERTRIE_HYPERTRIE_CORE_TRAITS_TOSTRING_HPP