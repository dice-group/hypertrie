#ifndef HYPERTRIE_FMT_FULLNODE_HPP
#define HYPERTRIE_FMT_FULLNODE_HPP

#include <dice/hypertrie/internal/raw/node/FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/util/fmt_utils.hpp>
#include <ranges>

namespace fmt {
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t, dice::hypertrie::ByteAllocator allocator_type>
	struct formatter<::dice::hypertrie::internal::raw::FullNode<depth, htt_t, allocator_type>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::FullNode<depth, htt_t, allocator_type> const &fn, FormatContext &ctx) {
			format_to(ctx.out(), FMT_STRING("{{ [size={},ref_count={}]\n"), fn.size(), fn.ref_count());
			for (size_t pos = 0; pos < depth; ++pos) {
				std::string edges;
				if constexpr (depth > 1 or not htt_t::is_bool_valued)
					for (auto const &[key_part, identifier] : fn.edges(pos))
						edges += fmt::format(FMT_STRING("{} -> {}\n"), key_part, identifier);
				else
					for (auto const &key_part : fn.edges(pos))
						edges += fmt::format(FMT_STRING("{}, "), key_part);
				format_to(ctx.out(), FMT_STRING("{}: [\n{}]\n"), pos, edges);
			}
			return format_to(ctx.out(), FMT_STRING(" }}"));
		}
	};
}// namespace fmt

#endif//HYPERTRIE_FMT_FULLNODE_HPP
