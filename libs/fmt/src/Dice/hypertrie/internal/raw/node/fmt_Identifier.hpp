#ifndef HYPERTRIE_FMT_IDENTIFIER_HPP
#define HYPERTRIE_FMT_IDENTIFIER_HPP

#include <Dice/hypertrie/internal/raw/node/Identifier.hpp>

namespace fmt {
	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct formatter<::hypertrie::internal::raw::RawIdentifier<depth, tri>> : public hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::hypertrie::internal::raw::RawIdentifier<depth, tri> const &id, FormatContext &ctx) {

			if (id.empty()) {
				return format_to(ctx.out(), "empty-id");
			} else if (id.is_sen()) {
				if constexpr (depth == 1 and ::hypertrie::internal::raw::HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>)
					return format_to(ctx.out(), "<key_part:{}", id.get_entry().key()[0]);
				else
					return format_to(ctx.out(), "sen_{:X}", id.hash() & ~(1UL << ::hypertrie::internal::raw::RawIdentifier<depth, tri>::tag_pos)& 0xFFFFUL) ;
			} else {
				return format_to(ctx.out(), "fn_{:X}", id.hash() & ~(1UL << ::hypertrie::internal::raw::RawIdentifier<depth, tri>::tag_pos)& 0xFFFFUL);
			}
		}
	};
}// namespace fmt
#endif//HYPERTRIE_FMT_IDENTIFIER_HPP
