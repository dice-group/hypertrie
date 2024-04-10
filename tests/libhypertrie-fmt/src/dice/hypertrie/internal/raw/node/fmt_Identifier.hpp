#ifndef HYPERTRIE_FMT_IDENTIFIER_HPP
#define HYPERTRIE_FMT_IDENTIFIER_HPP

#include <dice/hypertrie/internal/raw/node/Identifier.hpp>
#include <dice/hypertrie/internal/util/fmt_utils.hpp>

namespace fmt {
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct formatter<::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t>> : public ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t> const &id, FormatContext &ctx) {

			if (id.empty()) {
				return format_to(ctx.out(), "empty-id");
			} else if (id.is_sen()) {
				if constexpr (depth == 1 and ::dice::hypertrie::HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
					return format_to(ctx.out(), "<key_part:{}", id.get_entry().key()[0]);
				else
					return format_to(ctx.out(), "sen_{:X}", id.hash() & ~(1UL << ::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t>::tag_pos) & 0xFFFFUL);
			} else {
				return format_to(ctx.out(), "fn_{:X}", id.hash() & ~(1UL << ::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t>::tag_pos) & 0xFFFFUL);
			}
		}
	};
}// namespace fmt
#endif//HYPERTRIE_FMT_IDENTIFIER_HPP
