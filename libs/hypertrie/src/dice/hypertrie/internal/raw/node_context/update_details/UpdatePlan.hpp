#ifndef UPDATESPLAN_HPP
#define UPDATESPLAN_HPP


#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/Identifier.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"
#include "dice/hypertrie/internal/raw/node_context/update_details/UpdateRequests.hpp"

#include <robin_hood.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>


namespace dice::hypertrie::internal::raw::node_context::update_details {


	/**
	 * Plans concrete changes for updates requested via UpdateRequest.
	 * @tparam depth
	 * @tparam htt_t
	 * @tparam allocator_type
	 * @tparam max_depth
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t max_depth>
	struct UpdatePlan {
		static constexpr bool ht_hsi_depth1 = HTHSIDepth1<depth, htt_t>;

		using UpdateRequests_t = UpdateRequests<depth, htt_t, allocator_type, max_depth>;

		using SingleEntry_t = SingleEntry<depth, htt_t>;
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;
		using NodeStorage_t = NodeStorage<max_depth, htt_t, allocator_type>;
		using SENChange = typename UpdateRequests_t::SENChange;
		using SENChanges = typename UpdateRequests_t::SENChanges;
		using FNEntriesUpdateReq = typename UpdateRequests_t::FNEntriesUpdateData;

		struct FNCreation {
			std::vector<SingleEntry_t> entries;
			RawIdentifier_t id;
		};

		struct FNEntriesUpdate {
			RawIdentifier_t source_id;
			RawIdentifier_t target_id;
			std::vector<SingleEntry_t> entries;
			EntriesUpdateMode mode;
		};

		struct FNEntriesInsertion : FNEntriesUpdate {};
		struct FNEntriesEarsure : FNEntriesUpdate {};

		std::vector<FNCreation> fn_create{};                   ///< FN that are newly created
		std::vector<FNEntriesUpdate> fn_update_copy{};         ///< FN that are created from copies of existing FNs
		std::vector<FNEntriesUpdate> fn_update_move{};         ///< FN that are created from moved existing FNs that would otherwise be deleted because their reference count reached 0
		robin_hood::unordered_set<RawIdentifier_t> fn_delete{};///< FN that are deleted because their reference count reached 0 (already excluding those that are moved for reusal)
		/**
		 * The deltas for all reference count changes are listed here. For new node the delta is relative to 0.<br/>
		 * Changes must be applied to fn_create, fn_update_copy (source_id if present, target_id), fn_update_move (not source_id, target_id).<br/>
		 * Also, fn_deltas might contain additional FN node ids. The deltas must also be applied to these nodes.<br/>
		 * @warning Keep track of the nodes where the deltas were applied already. Applying them twice will render the NodeContext inconsistent and might raise UB.
		 **/
		robin_hood::unordered_map<RawIdentifier_t, ssize_t> fn_deltas{};
		SENChanges sen_changes;///< Changes to SENs. Includes creating or deleting SENs as well as changes to reference counts.

		explicit UpdatePlan(UpdateRequests_t &&ur) {
			auto const &fns = ur.fns();

			/*
			 * FNs that are not yet in nodes_stprage and must be created during the update of the level depth.
			 * As soon as they have been assigned to a specific changes set (fn_create, fn_update_copy, fn_update_move) they are removed from here.
			 */
			robin_hood::unordered_set<RawIdentifier_t> fn_to_be_created = [&]() {
				robin_hood::unordered_set<RawIdentifier_t> ret;
				for (const auto &id : ur.fn_potentially_new_) {
					if (!fns.count(id))
						ret.insert(id);
				}
				return ret;
			}();

			// populate deleted_fns, remove deleted FNs and FNs with delta == 0 from fn_deltas
			{
				for (const auto &source_fn : ur.fn_deletion_candidates_) {
					assert(fns.contains(source_fn));
					if (auto fn_delta_iter = ur.fn_deltas_.find(source_fn);
						fn_delta_iter != ur.fn_deltas_.end()) {
						assert(static_cast<ssize_t>(fns.at(source_fn)->ref_count()) + fn_delta_iter->second >= 0);
						if (fn_delta_iter->second == 0) {// short circuit before access to NodeStorage
							// delta can only be 0 if at least one ref_count decrease was requested
							// ref_count decreases require adding the identifier to ur.fn_deletion_candidates_
							// so this removes all ref_count deltas that are 0
							ur.fn_deltas_.erase(fn_delta_iter);
							continue;
						}
						if (static_cast<ssize_t>(fns.at(source_fn)->ref_count()) + fn_delta_iter->second == 0) {
							fn_delete.insert(source_fn);
							ur.fn_deltas_.erase(fn_delta_iter);// remove nodes in deleted_fns from fn_deltas
						}
					}
				}
			}

			// populate fn_update_copy and fn_update_move
			{
				for (auto &[source_id, targets] : ur.fn_update_entries_) {
					// Idea: A source node that is planned for deletion can be reused. Because reusing is destructive it can only be done once per source node.
					bool move_one_source = fn_delete.contains(source_id);
					for (auto &[target_id, update_req] : targets) {
						// check if target_id is in created_fns. So, we still need to create it.
						// if it is still there we remove it because it will be created by the FNEntriesUpdate that is planned here
						if ([&, target_id = target_id]() {// true if the creation of th FN for target_id is already covered
								auto iter = fn_to_be_created.find(target_id);
								if (iter == fn_to_be_created.end())
									return true;
								fn_to_be_created.erase(iter);
								return false;
							}())
							continue;

						FNEntriesUpdate fn_entries_update{.source_id = source_id,
														  .target_id = target_id,
														  .entries = std::move(update_req.entries),
														  .mode = update_req.mode};
						if (move_one_source) {
							// source is listed to be deleted and target is listed to be created. So, source can be reused to create target
							fn_update_move.emplace_back(std::move(fn_entries_update));
							fn_delete.erase(source_id);
							move_one_source = false;
						} else {
							fn_update_copy.emplace_back(std::move(fn_entries_update));
						}
					}
				}
			}


			// For the remaining nodes that should be created (fn_to_be_created) add the creation content to fn_create_
			{
				fn_create.reserve(fn_to_be_created.size());
				for (auto const &id : fn_to_be_created) {
					assert(ur.fn_creations_.contains(id));
					fn_create.emplace_back(FNCreation{.entries = std::move(ur.fn_creations_[id]), .id = id});
				}
			}

			// move fn_deltas from RemovalChangeRequest
			{
				fn_deltas = std::move(ur.fn_deltas_);
			}

			// move the SEN changes from RemovalChangeRequest and filter out the ones that don't change the ref_count_delta
			if constexpr (!ht_hsi_depth1) {
				sen_changes = std::move(ur.sen_changes_);
				for (auto iter = sen_changes.begin(); iter != sen_changes.end();) {
					if (iter->second.ref_count_delta == 0) {
						iter = sen_changes.erase(iter);
					} else {
						++iter;
					}
				}
			}
		}
	};

	template<HypertrieTrait htt_t, ByteAllocator allocator_type, size_t max_depth>
	struct UpdatePlan<0, htt_t, allocator_type, max_depth> {
	};
}// namespace dice::hypertrie::internal::raw::node_context::update_details
#endif//UPDATESPLAN_HPP
