#ifndef HYPERTRIE_RAWNODECONTEXT_INSERT_IMPL_UPWARDSLVCHANGES_HPP
#define HYPERTRIE_RAWNODECONTEXT_INSERT_IMPL_UPWARDSLVCHANGES_HPP

#include "dice/hypertrie/internal/raw/node_context/common_detail/CommonUpwardsLvChanges.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/Container.hpp"

namespace dice::hypertrie::internal::raw::node_context::insert_detail {
	using namespace node_context::common_detail;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct UpwardsLvChanges {
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;
		using EdgeReassign_t = EdgeReassign<htt_t, allocator_type>;

		Map<RawIdentifier_t, std::vector<EdgeReassign_t>> edge_reassigns;
	};

	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct AllUpwardsLvChanges {
		template<size_t depth>
		using UpwardsLvChanges_t = UpwardsLvChanges<depth, htt_t, allocator_type>;

		template_library::integral_template_tuple<1UL, max_depth + 1, UpwardsLvChanges_t> changes;

		template<size_t depth>
		UpwardsLvChanges_t<depth> const &for_depth() const noexcept {
			return changes.template get<depth>();
		}

		template<size_t depth>
		UpwardsLvChanges_t<depth> &for_depth() noexcept {
			return changes.template get<depth>();
		}

		template<size_t depth>
		void answer_edge_reassign(Requester<max_depth, htt_t> const &requester, NodePtr<depth, htt_t, allocator_type> ptr) noexcept {
			requester.who_asked.visit([&]<size_t req_depth>(RawIdentifier<req_depth, htt_t> const who_asked) noexcept {
				EdgeReassign<htt_t, allocator_type> const reassign{.edge = requester.who_asked_edge,
																   .new_child = ptr};

				auto &reassigns = this->template for_depth<req_depth>().edge_reassigns[who_asked];
				assert((std::ranges::find(reassigns, reassign) == reassigns.end()));
				reassigns.push_back(reassign);
			});
		}

		template<size_t depth>
		void answer_edge_reassigns(std::vector<Requester<max_depth, htt_t>> const &requesters, NodePtr<depth, htt_t, allocator_type> ptr) noexcept {
			for (auto const &requester : requesters) {
				answer_edge_reassign(requester, ptr);
			}
		}
	};

} // namespace dice::hypertrie::internal::raw::node_context::insert_detail

#endif  //HYPERTRIE_RAWNODECONTEXT_INSERT_IMPL_UPWARDSLVCHANGES_HPP
