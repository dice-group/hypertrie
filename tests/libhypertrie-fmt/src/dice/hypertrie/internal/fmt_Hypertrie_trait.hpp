#ifndef HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP
#define HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP

#include "dice/hypertrie/internal/util/fmt_utils.hpp"
#include <dice/hypertrie/Hypertrie_trait.hpp>

namespace fmt {
	template<::dice::hypertrie::HypertrieTrait htt_t>
	struct formatter<htt_t> : public ::dice::hypertrie::internal::util::SimpleParsing {
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		template<typename T>
		static auto nameOfType() {
			return ::dice::hypertrie::internal::util::name_of_type<T>();
		}

		template<typename FormatContext>
		auto format(htt_t const &, FormatContext &ctx) {
			return format_to(ctx.out(),
							 "<key_part = {}, value = {}, map = {}, set = {}, key_part_tagging_bit = {}>",
							 nameOfType<key_part_type>(), nameOfType<value_type>(),
							 nameOfType<typename htt_t::template map_type<key_part_type, value_type, std::allocator<std::byte>>>(),
							 nameOfType<typename htt_t::template set_type<key_part_type, std::allocator<std::byte>>>(),
							 htt_t::key_part_tagging_bit);
		}
	};
}// namespace fmt

#endif//HYPERTRIE_HYPERTRIE_TRAITS_TOSTRING_HPP