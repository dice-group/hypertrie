#ifndef HYPERTRIE_FMT_IDENTIFIER_HPP
#define HYPERTRIE_FMT_IDENTIFIER_HPP

#include <dice/hypertrie/internal/raw/node/RawIdentifier.hpp>
#include <dice/hypertrie/internal/util/fmt_utils.hpp>

namespace fmt {
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct formatter<::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t>> : public ::dice::hypertrie::internal::util::IdentifierFmtParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t> const &id, FormatContext &ctx) {
			using namespace ::dice::hypertrie;
			using namespace internal::raw;

			if (id.empty()) {
				return format_to(ctx.out(), "empty-id");
			}

			switch (id.tag()) {
				case IdentifierTag::FN: {
					return format_to(ctx.out(), "fn_{:X}", id.hash());
				}
				case IdentifierTag::SEN: {
					return format_to(ctx.out(), "sen_{:X}", id.hash());
				}
				case IdentifierTag::XN: {
					return format_to(ctx.out(), "xn_{:X}", id.hash());
				}
				case IdentifierTag::Indeterminate: {
					return format_to(ctx.out(), "?_{:X}", id.hash());
				}
			}
		}
	};
}// namespace fmt

namespace dice::hypertrie::internal::raw {
	template<size_t depth, HypertrieTrait htt_t>
	std::ostream &operator<<(std::ostream &os, RawIdentifier<depth, htt_t> const &id) {
		os << fmt::format("{}", id);
		return os;
	}
} // namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_FMT_IDENTIFIER_HPP
