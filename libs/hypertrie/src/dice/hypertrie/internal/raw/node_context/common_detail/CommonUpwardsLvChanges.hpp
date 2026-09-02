#ifndef HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_COMMONUPWARDSLVCHANGES_HPP
#define HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_COMMONUPWARDSLVCHANGES_HPP

#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/RawIdentifier.hpp"
#include "dice/template-library/integral_template_variant.hpp"

namespace dice::hypertrie::internal::raw::node_context::common_detail {

	template<HypertrieTrait htt_t>
	struct BoundPos {
		size_t pos;
		typename htt_t::key_part_type key_part;

		bool operator==(BoundPos const &) const = default;
	};

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct EdgeReassign {
		BoundPos<htt_t> edge;
		RawNodePtr<htt_t, allocator_type> new_child;

		bool operator==(EdgeReassign const &other) const noexcept = default;
	};

	template<size_t max_depth, HypertrieTrait htt_t>
	struct Requester {
		template<size_t depth>
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;

		template_library::integral_template_variant<2UL, max_depth + 1, RawIdentifier_t> who_asked;
		BoundPos<htt_t> who_asked_edge;

		[[nodiscard]] constexpr size_t depth() const noexcept {
			return who_asked.index();
		}
	};

} // namespace dice::hypertrie::internal::raw::node_context::common_detail

#endif//HYPERTRIE_COMMONUPWARDSLVCHANGES_HPP
