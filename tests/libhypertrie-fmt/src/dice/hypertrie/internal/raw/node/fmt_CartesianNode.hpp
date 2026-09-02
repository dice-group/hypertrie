#ifndef HYPERTRIE_FMT_CARTESIANNODE_HPP
#define HYPERTRIE_FMT_CARTESIANNODE_HPP

#include <dice/hypertrie/internal/raw/node/CartesianNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>

namespace fmt {
	template<size_t depth>
	struct formatter<::dice::hypertrie::internal::raw::CartesianDiscriminant<depth>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::CartesianDiscriminant<depth> const &d, FormatContext &ctx) {
			auto it = ctx.out();
			it = format_to(it, "{}", d[0]);
			for (size_t ix = 1; ix < depth; ++ix) {
				it = format_to(it, " ⨯ {}", d[ix]);
			}

			return it;
		}
	};

	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t, ::dice::hypertrie::ByteAllocator allocator_type>
	struct formatter<::dice::hypertrie::internal::raw::CartesianNode<depth, htt_t, allocator_type>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::dice::hypertrie::internal::raw::CartesianNode<depth, htt_t, allocator_type> const &xn, FormatContext &ctx) {
			auto it = ctx.out();

			it = format_to(it, "{{ [ref_count={}] ", xn.ref_count());

			xn.for_each_operand([&]<size_t ix, size_t operand_depth>(::dice::hypertrie::internal::raw::NodePtr<operand_depth, htt_t, allocator_type> operand) noexcept {
				if constexpr (ix == 0) {
					if constexpr (operand_depth == 0) {
						it = format_to(it, "empty-operand@D0");
					} else {
						if constexpr (operand_depth == 1 && ::dice::hypertrie::HypertrieTrait_taggable_key_part<htt_t>) {
							if (operand.is_sen()) {
								it = format_to(it, "inplace {}", operand.decode_key_part());
								return;
							}
						}

						it = format_to(it, "{:#}@D{}", operand.identifier(), operand_depth);
					}
				} else {
					if constexpr (operand_depth == 0) {
						it = format_to(it, " ⨯ empty-operand@D0");
					} else {
						if constexpr (operand_depth == 1 && ::dice::hypertrie::HypertrieTrait_taggable_key_part<htt_t>) {
							if (operand.is_sen()) {
								it = format_to(it, " ⨯ inplace {}", operand.decode_key_part());
								return;
							}
						}

						it = format_to(it, " ⨯ {:#}@D{}", operand.identifier(), operand_depth);
					}
				}
			});

			return format_to(it, " }}");
		}
	};
}// namespace fmt

namespace dice::hypertrie::internal::raw {
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	std::ostream &operator<<(std::ostream &os, CartesianNode<depth, htt_t, allocator_type> const &xn) {
		os << fmt::format("{}", xn);
		return os;
	}
} // namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_FMT_CARTESIANNODE_HPP
