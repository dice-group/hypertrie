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
			format_to(ctx.out(), "{{ [size={},ref_count={}]\n", fn.size(), fn.ref_count());

			for (size_t pos = 0; pos < depth; ++pos) {
				std::string edges;

				if constexpr (depth > 1) {
					for (auto const &[key_part, child] : fn.edges(pos)) {
						if constexpr (depth - 1 == 1 && ::dice::hypertrie::HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
							if (child.is_sen()) {
								edges += fmt::format("{} -> inplace {}\n", key_part, child.decode_key_part());
								continue;
							}
						}

						edges += fmt::format("{} -> {}\n", key_part, child.identifier());
					}
				} else {
					if constexpr (::dice::hypertrie::HypertrieTrait_bool_valued<htt_t>) {
						for (auto const &key_part : fn.edges(pos)) {
							edges += fmt::format("{}, ", key_part);
						}
					} else {
						for (auto const &[key_part, value] : fn.edges(pos)) {
							edges += fmt::format("{} -> {}\n", key_part, value);
						}
					}
				}

				format_to(ctx.out(), "{}: [\n{}]\n", pos, edges);
			}
			return format_to(ctx.out(), " }}");
		}
	};
}// namespace fmt

namespace dice::hypertrie::internal::raw {
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	std::ostream &operator<<(std::ostream &os, FullNode<depth, htt_t, allocator_type> const &fn) {
		os << fmt::format("{}", fn);
		return os;
	}
} // namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_FMT_FULLNODE_HPP
