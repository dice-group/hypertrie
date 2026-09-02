#ifndef HYPERTRIE_RAWNODECONTEXT_INSERT_IMPL_LVCHANGES_HPP
#define HYPERTRIE_RAWNODECONTEXT_INSERT_IMPL_LVCHANGES_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/iteration/RawIterator.hpp"
#include "dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/CommonLvChanges.hpp"
#include "dice/hypertrie/internal/raw/node_context/insert_detail/UpwardsLvChanges.hpp"

#include <algorithm>

namespace dice::hypertrie::internal::raw::node_context::insert_detail {
	using namespace node_context::common_detail;

	template<size_t depth, size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct LvChanges : CommonLvChanges<depth, max_depth, htt_t, allocator_type> {
		using super_t = CommonLvChanges<depth, max_depth, htt_t, allocator_type>;

		using typename super_t::key_part_type;
		using typename super_t::SingleEntry_t;
		using typename super_t::RawIdentifier_t;
		using typename super_t::Change;
		using typename super_t::NodeStorage_t;
		using typename super_t::NodePtr_t;

		using Requester_t = Requester<max_depth, htt_t>;


		template<NodeBeforeRcPolicy rc_policy = NodeBeforeRcPolicy::Dec, IdentifierTag tag_hint = IdentifierTag::Indeterminate>
		NodePtr_t insert_into_node(NodePtr_t const node_before,
								   std::vector<SingleEntry_t> &&entries,
								   Requester_t const &requester,
								   NodeStorage_t &node_storage) noexcept {
			assert(!entries.empty());
			assert(requester.depth() > depth);

			auto id_before = node_before.identifier();
			assert(!id_before.is_indeterminate());
			assert(id_before.retag_as_indeterminate() != RawIdentifier_t{});

			if constexpr (rc_policy == NodeBeforeRcPolicy::Dec) {
				if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					if (!id_before.is_sen()) {
						this->dec_ref(id_before);
					}
				} else {
					this->dec_ref(id_before);
				}
			}

			auto &changes = this->node_changes[node_before];

			if constexpr (depth == 1) {
				// can only become FN
				static_assert(tag_hint == IdentifierTag::Indeterminate || tag_hint == IdentifierTag::FN);

				auto const id_after = id_before.combine_add(RawIdentifier_t{entries}, IdentifierTag::FN);
				this->inc_ref(id_after);

				if (auto fn_ptr = node_storage.template lookup<1, FullNode>(id_after); fn_ptr != nullptr) {
					return fn_ptr;
				}

				if (auto it = changes.find(id_after); it != changes.end()) {
					it->second.requesters.push_back(requester);
				} else {
					changes.emplace(id_after, Change{.entries = std::move(entries),
													 .requesters = {requester}});
				}

				return NodePtr_t{}; // will be replaced later
			} else {
				// can either become FN or XN
				auto const id_after = id_before.combine_add(RawIdentifier_t{entries}, tag_hint);

				if constexpr (tag_hint == IdentifierTag::Indeterminate || tag_hint == IdentifierTag::FN) {
					auto const fn_id_after = id_after.retag_as_fn();

					if (auto fn_ptr = node_storage.template lookup<depth, FullNode>(fn_id_after); fn_ptr != nullptr) {
						this->inc_ref(fn_id_after);
						return fn_ptr;
					}

					if (auto it = changes.find(fn_id_after); it != changes.end()) {
						this->inc_ref(fn_id_after);
						container::deref(it).requesters.push_back(requester);
						return NodePtr_t{}; // will be replaced later
					}
				}

				if constexpr (tag_hint == IdentifierTag::Indeterminate || tag_hint == IdentifierTag::XN) {
					auto const xn_id_after = id_after.retag_as_xn();

					if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(xn_id_after); xn_ptr != nullptr) {
						this->inc_ref(xn_id_after);
						return xn_ptr;
					}

					if (auto it = changes.find(xn_id_after); it != changes.end()) {
						this->inc_ref(xn_id_after);
						container::deref(it).requesters.push_back(requester);
						return NodePtr_t{}; // will be replaced later
					}
				}

				this->inc_ref(id_after);

				// still indeterminate, this means we need to message requester later
				// since we cannot calculate the correct id yet

				if (auto it = changes.find(id_after); it != changes.end()) {
					// exists but is currently indeterminate
					it->second.requesters.push_back(requester);
				} else {
					changes.emplace(id_after, Change{.entries = std::move(entries),
													 .requesters = {requester}});
				}

				return NodePtr_t{}; // will be replaced later
			}
		}
	};

	/**
	 * Is instantiated for simplicity but must never be used (therefore, has no members or methods).
	 */
	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	class LvChanges<0, max_depth, htt_t, allocator_type> {
	};
}// namespace dice::hypertrie::internal::raw::node_context::insert_detail

#endif//HYPERTRIE_RAWNODECONTEXT_INSERT_IMPL_LVCHANGES_HPP
