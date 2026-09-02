#ifndef HYPERTRIE_RAWNODECONTEXT_REMOVE_IMPL_UPWARDSLVCHANGES_HPP
#define HYPERTRIE_RAWNODECONTEXT_REMOVE_IMPL_UPWARDSLVCHANGES_HPP

#include "dice/hypertrie/internal/raw/node_context/common_detail/CommonUpwardsLvChanges.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/Container.hpp"

namespace dice::hypertrie::internal::raw::node_context::remove_detail {
	using namespace node_context::common_detail;

	/**
	 * A request to replace a nodes child with an in-place single entry node
	 * answer to SENCheckOrigin
	 */
	template<HypertrieTrait htt_t>
	struct SENReplace {
		BoundPos<htt_t> child_to_replace;
		SingleEntry<1, htt_t> replacement;
	};

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct UpwardsLvChanges {
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;
		using EdgeReassign_t = EdgeReassign<htt_t, allocator_type>;
		using SENReplace_t = SENReplace<htt_t>;
		using SENPtr_t = SENPtr<depth, htt_t, allocator_type>;

		/**
		 * id -> [(pos, key_part, new_child)]
		 *
		 * request that a node replaces some of it's edges by the given pointers
		 */
		Map<RawIdentifier_t, std::vector<EdgeReassign_t>> edge_reassigns;

		/**
		 * sen id -> rc delta
		 *
		 * reference count deltas for sens that still need a sen check and thus don't exist in the node
		 * storage yet. These deltas must be applied late, i.e. in the upwards pass after the sens are inserted into the node storage
		 * instead of the usual place in the downwards pass.
		 */
		Map<RawIdentifier_t, ssize_t> sen_rc_deltas;

		/**
		 * sen id -> placeholder sen ptr
		 *
		 * placeholder sens that will each be filled in by one sen check
		 * the sen check itself is implicitly associated to the sen placeholder by
		 * having a pointer to the placeholders key(+offset) and value
		 *
		 * Lifetime: the sen ptrs here are consumed by apply_up and will be integrated into the node storage there
		 */
		Map<RawIdentifier_t, SENPtr_t> sen_buffers;

		/**
		 * node -> { (child pos, entry) }
		 *
		 * request that a node replaces some of it's edges by the given in-place SENs
		 */
		Map<RawIdentifier_t, std::vector<SENReplace_t>> child_sen_replacements;
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

		template<size_t parent_depth>
		void answer_sen_check(RawIdentifier<parent_depth, htt_t> const parent, BoundPos<htt_t> child_loc, SingleEntry<1, htt_t> const &se) noexcept requires (HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
			static_assert(parent_depth > 1);
			assert(parent.is_fn());

			auto &sen_replacements = this->template for_depth<parent_depth>().child_sen_replacements[parent];

			SENReplace<htt_t> sen_replace{
					.child_to_replace = child_loc,
					.replacement      = se};

			//assert(std::ranges::count(sen_replacements, sen_replace) == 0);
			sen_replacements.push_back(sen_replace);
		}

		template<size_t depth>
		void answer_edge_reassign(Requester<max_depth, htt_t> const &requester, NodePtr<depth, htt_t, allocator_type> ptr) noexcept {
			requester.who_asked.visit([&]<size_t req_depth>(RawIdentifier<req_depth, htt_t> const who_asked) noexcept {
				EdgeReassign<htt_t, allocator_type> const reassign{.edge = requester.who_asked_edge,
																   .new_child = ptr};

				auto &retags = this->template for_depth<req_depth>().edge_reassigns[who_asked];
				assert((std::ranges::find(retags, reassign) == retags.end()));
				retags.push_back(reassign);
			});
		}

		template<size_t depth>
		void answer_edge_reassigns(std::vector<Requester<max_depth, htt_t>> const &requesters, NodePtr<depth, htt_t, allocator_type> ptr) noexcept {
			for (auto const &requester : requesters) {
				answer_edge_reassign(requester, ptr);
			}
		}

		template<size_t depth>
		void inc_sen_ref(RawIdentifier<depth, htt_t> const id, size_t const n = 1) noexcept {
			assert(id.is_sen());
			assert((id.retag_as_indeterminate() != RawIdentifier<depth, htt_t>{}));

			this->template for_depth<depth>().sen_rc_deltas[id] += n;
		}

		template<size_t depth>
		std::pair<SENPtr<depth, htt_t, allocator_type>, bool> create_placeholder_sen(RawIdentifier<depth, htt_t> id, NodeStorage<max_depth, htt_t, allocator_type> &node_storage) noexcept {
			assert(id.is_sen());
			assert((id.retag_as_indeterminate() != RawIdentifier<depth, htt_t>{}));

			auto &buffers = this->template for_depth<depth>().sen_buffers;
			if (auto it = buffers.find(id); it != buffers.end()) {
				return std::make_pair(it->second, false);
			}

			auto &sen_lifecycle_ = node_storage.template nodes<depth, SingleEntryNode>().node_lifecycle();
			auto [it, _] = buffers.emplace(id, sen_lifecycle_.new_(0UL));
			return std::make_pair(it->second, true);
		}
	};

} // namespace dice::hypertrie::internal::raw::node_context::remove_detail

#endif // HYPERTRIE_RAWNODECONTEXT_REMOVE_IMPL_UPWARDSLVCHANGES_HPP
