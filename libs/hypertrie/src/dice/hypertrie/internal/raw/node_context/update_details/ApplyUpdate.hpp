#ifndef APPLYUPDATE_HPP
#define APPLYUPDATE_HPP

#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/container/AllContainer.hpp"
#include "dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "dice/hypertrie/internal/raw/node/Identifier.hpp"
#include "dice/hypertrie/internal/raw/node/NodeContainer.hpp"
#include "dice/hypertrie/internal/raw/node/NodeStorage.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"
#include "dice/hypertrie/internal/raw/node_context/update_details/EntrySubsetForPos.hpp"
#include "dice/hypertrie/internal/raw/node_context/update_details/UpdatePlan.hpp"
#include "dice/hypertrie/internal/raw/node_context/update_details/UpdateRequests.hpp"

#include <cstdint>
#include <robin_hood.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

namespace dice::hypertrie::internal::raw::node_context::update_details {

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t max_depth>
	struct ApplyUpdate;


	/**
	 * Execute the changes from update_requests on node_storage.
	 * @tparam depth
	 * @tparam htt_t
	 * @tparam allocator_type
	 * @tparam max_depth
	 * @param node_storage node storage to apply changes to
	 * @param update_requests updates that are requestd to be executed
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t max_depth>
	void apply_update(NodeStorage<max_depth, htt_t, allocator_type> &node_storage,
					  UpdateRequests<depth, htt_t, allocator_type, max_depth> &&update_requests) {
		ApplyUpdate<depth, htt_t, allocator_type, max_depth>{node_storage, std::move(update_requests)}
				.consume_and_execute();
	}

	/**
	 * Insert or erase entries into/from the node passed in via nodec. Depending on the change, nodec can be empty before or after the call.
	 * @tparam mode if entries should be inserted or erased
	 * @tparam depth
	 * @tparam htt_t
	 * @tparam allocator_type
	 * @tparam max_depth
	 * @param node_storage node storage that holds nodec (if its content is not inlined) and where changes will be applied
	 * @param nodec this will be updated and reflect the insertion or easure of entries
	 * @param entries the entries to be inserted or erased into/from nodec
	 */
	template<EntriesUpdateMode mode, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t max_depth>
	void apply_update(NodeStorage<max_depth, htt_t, allocator_type> &node_storage,
					  NodeContainer<depth, htt_t, allocator_type> &nodec,
					  std::vector<SingleEntry<depth, htt_t>> &&entries) {
		if (entries.empty()) {
			return;
		}

		UpdateRequests<depth, htt_t, allocator_type, max_depth> update_requests{node_storage};

		auto const target_id = [&]() {
			if constexpr (mode == EntriesUpdateMode::INSERT) {
				if (nodec.empty()) {
					return update_requests.add_node(std::move(entries));
				}
				return update_requests.insert_into_node(nodec.raw_identifier(), std::move(entries), true);
			} else {// mode == EntriesUpdateMode::ERASE
				return update_requests.remove_from_node(nodec.raw_identifier(), std::move(entries), true);
			}
		}();

		apply_update(node_storage, std::move(update_requests));

		if (target_id.empty()) {
			assert(mode == EntriesUpdateMode::ERASE);
			nodec = {};
			return;
		}

		if constexpr (HTHSIDepth1<depth, htt_t>) {
			if (target_id.is_sen()) {
				// value is inplace, pointer not used
				nodec = SENContainer<depth, htt_t, allocator_type>{target_id};
				assert(!nodec.raw_identifier().empty());
				return;
			}
			nodec = FNContainer<depth, htt_t, allocator_type>{target_id, node_storage.template lookup<depth, FullNode>(target_id)};
		} else {
			nodec = node_storage.template lookup<depth>(target_id);
		}
		if constexpr (mode == EntriesUpdateMode::INSERT) {
			assert(!nodec.is_null_ptr());
		}
	}

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t max_depth>
	void insert_entries_into_node(NodeStorage<max_depth, htt_t, allocator_type> &node_storage,
								  NodeContainer<depth, htt_t, allocator_type> &nodec,
								  std::vector<SingleEntry<depth, htt_t>> &&entries) {
		apply_update<EntriesUpdateMode::INSERT>(node_storage, nodec, std::move(entries));
	}

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t max_depth>
	void erase_entries_from_node(NodeStorage<max_depth, htt_t, allocator_type> &node_storage,
								 NodeContainer<depth, htt_t, allocator_type> &nodec,
								 std::vector<SingleEntry<depth, htt_t>> &&entries) {
		apply_update<EntriesUpdateMode::ERASE>(node_storage, nodec, std::move(entries));
	}

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t max_depth>
	struct ApplyUpdate {
	private:
		static constexpr bool ht_hsi_depth1 = HTHSIDepth1<depth, htt_t>;
		static constexpr bool ht_hsi_depth2 = HTHSIDepth2<depth, htt_t>;

	public:
		using UpdateRequests_t = UpdateRequests<depth, htt_t, allocator_type, max_depth>;
		using NodeStorage_t = NodeStorage<max_depth, htt_t, allocator_type>;

	private:
		enum struct NodeOrigin : uint8_t {
			Copied,
			Moved,
			JustCreated
		};

		using UpdatePlan_t = UpdatePlan<depth, htt_t, allocator_type, max_depth>;
		using ChildUpdateRequests_t = UpdateRequests<depth - 1, htt_t, allocator_type, max_depth>;
		using SENChange = typename UpdatePlan_t::SENChange;
		template<size_t depth2>
		using RawIdentifier_t = RawIdentifier<depth2, htt_t>;
		template<size_t depth2>
		using FNContainer_t = SpecificNodeContainer<depth2, htt_t, FullNode, allocator_type>;
		template<size_t depth2>
		using SingleEntry_t = SingleEntry<depth2, htt_t>;
		using key_part_type = typename htt_t::key_part_type;
		template<size_t depth2>
		using FNEntriesUpdate = typename UpdatePlan<depth2, htt_t, allocator_type, max_depth>::FNEntriesUpdate;
		template<size_t depth2>
		using FNCreation = typename UpdatePlan<depth2, htt_t, allocator_type, max_depth>::FNCreation;
		using FNPtr = typename FNContainer<depth, htt_t, allocator_type>::NodePtr;

		UpdatePlan_t update_plan_;
		ChildUpdateRequests_t child_update_requests_;
		NodeStorage_t &node_storage_;
		::robin_hood::unordered_set<RawIdentifier_t<depth>> done_fns_{};

		auto &fns() {
			return node_storage_.template nodes<depth, FullNode>().nodes();
		}

		auto &fn_lifecycle() {
			return node_storage_.template nodes<depth, FullNode>().node_lifecycle();
		}

		auto &sens()
			requires(!ht_hsi_depth1)
		{
			return node_storage_.template nodes<depth, SingleEntryNode>().nodes();
		}

		auto &sens_lifecycle()
			requires(!ht_hsi_depth1)
		{
			return node_storage_.template nodes<depth, SingleEntryNode>().node_lifecycle();
		}

	public:
		ApplyUpdate(NodeStorage_t &node_storage,
					UpdateRequests_t &&update_requests)
			: update_plan_{std::move(update_requests)},
			  child_update_requests_{node_storage},
			  node_storage_{node_storage} {}


		/**
		 * This executes applying the changes. The object is consumed in the process.
		 */
		void consume_and_execute() && {
			// create new FNs from scratch or by promoting a SEN
			for (FNCreation<depth> &fn_creation_plan : update_plan_.fn_create) {
				create_fn(std::move(fn_creation_plan));
			}

			// create new FNs by copy-inserting into existing FNs
			for (FNEntriesUpdate<depth> &fn_entries_update : update_plan_.fn_update_copy) {
				copy_update_fn_to_fn(std::move(fn_entries_update));
			}


			// create new FNs by move-inserting into existing FNs that will no longer be used
			for (FNEntriesUpdate<depth> &fn_entries_update : update_plan_.fn_update_move) {
				move_update_fn_to_fn(std::move(fn_entries_update));
			}

			// delete FNs that are no longer referenced
			for (auto id : update_plan_.fn_delete) {
				delete_fn(id);
			}

			// update refcount of existing FNs that are not deleted. None of them will have a ref_count of 0 afterwards.
			for (auto const &[id, ref_count_delta] : update_plan_.fn_deltas) {
				if (auto [_, not_yet_updated] = done_fns_.insert(id); not_yet_updated) {
					assert(fns().contains(id));
					auto node_ptr = container::deref(fns().find(id));
					node_ptr->ref_count() += update_plan_.fn_deltas.at(id);
					assert(node_ptr->ref_count() > 0);
				}
			}

			// create or delete SENs; and update their ref_count
			if constexpr (!ht_hsi_depth1) {// ignore inplace SENs
				for (auto &&[node_id, update] : update_plan_.sen_changes) {
					apply_sen_update(node_id, std::move(update));
				}
			}

			if constexpr (depth > 1) {
				ApplyUpdate<depth - 1, htt_t, allocator_type, max_depth>{node_storage_, std::move(child_update_requests_)}
						.consume_and_execute();
			}
		}

	private:
		void create_fn(FNCreation<depth> &&creation_plan) noexcept {
			assert(!fns().contains(creation_plan.id));
			assert(update_plan_.fn_deltas.at(creation_plan.id) > 0);
			auto fn_ptr = fn_lifecycle().new_with_alloc(static_cast<size_t>(update_plan_.fn_deltas.at(creation_plan.id)));
			done_fns_.insert(creation_plan.id);
			fns().emplace(creation_plan.id, fn_ptr);

			apply_fn_entry_insertion<NodeOrigin::JustCreated>(
					FNEntriesUpdate<depth>{
							.source_id = {},
							.target_id = creation_plan.id,
							.entries = std::move(creation_plan.entries),
							.mode = EntriesUpdateMode::INSERT},
					fn_ptr);
		}


		void copy_update_fn_to_fn(FNEntriesUpdate<depth> &&update) {
			auto fn_ptr = [&]() {// copy the existing FN
				assert(fns().contains(update.source_id));
				assert(!fns().contains(update.target_id));
				auto source_node_ptr = fns()[update.source_id];
				auto fn_ptr = fn_lifecycle().new_(*source_node_ptr);
				fns().emplace(update.target_id, fn_ptr);
				return fn_ptr;
			}();

			done_fns_.insert(update.target_id);
			auto const ref_count = update_plan_.fn_deltas.at(update.target_id);

			switch (update.mode) {
				case EntriesUpdateMode::INSERT:
					apply_fn_entry_insertion<NodeOrigin::Copied>(std::move(update), fn_ptr);
					break;

				case EntriesUpdateMode::ERASE:
					apply_fn_entry_erasure<NodeOrigin::Copied>(std::move(update), fn_ptr);
					break;
				default:
					assert(false);
			}

			fn_ptr->ref_count() = ref_count;
		}

		void move_update_fn_to_fn(FNEntriesUpdate<depth> &&update) {
			auto fn_ptr = [&]() {// detach the existing FN from node storage
				assert(fns().contains(update.source_id));
				assert(!fns().contains(update.target_id));
				auto iter = fns().find(update.source_id);
				auto ret = container::deref(iter);
				fns().erase(iter);
				fns().emplace(update.target_id, ret);
				return ret;
			}();

			done_fns_.insert(update.source_id);
			done_fns_.insert(update.target_id);
			auto const ref_count = update_plan_.fn_deltas.at(update.target_id);

			switch (update.mode) {
				case EntriesUpdateMode::INSERT:
					apply_fn_entry_insertion<NodeOrigin::Moved>(std::move(update), fn_ptr);
					break;

				case EntriesUpdateMode::ERASE:
					apply_fn_entry_erasure<NodeOrigin::Moved>(std::move(update), fn_ptr);
					break;
				default:
					assert(false);
			}
			fn_ptr->ref_count() = ref_count;
		}

		void delete_fn(RawIdentifier_t<depth> const id) {

			auto node = [&]() {// retrieve and detach node from node storage
				assert(fns().contains(id));
				auto iter = fns().find(id);
				auto ret = container::deref(iter);
				fns().erase(iter);// detach node
				return ret;
			}();

			if constexpr (depth > 1) {
				// decrement refcount of all children, except inplace SENs
				for (size_t pos = 0; pos < depth; ++pos) {
					for (auto &[child_mapping_key_part, id_child] : node->edges(pos)) {
						if constexpr (ht_hsi_depth2)
							if (id_child.is_sen())
								continue;
						child_update_requests_.apply_ref_count_delta(id_child, -1);
					}
				}
			}

			fn_lifecycle().delete_(node);
		};

		template<NodeOrigin node_origin>
		void apply_fn_entry_insertion(FNEntriesUpdate<depth> &&update, FNPtr fn_ptr) {
			assert(update.mode == EntriesUpdateMode::INSERT);

			if constexpr (depth == 1) {
				// depth 1 fullnode contains values as children => directly assign

				for (const auto &entry : update.entries)
					fn_ptr->insert_or_assign(entry.key(), entry.value());
			} else {
				fn_ptr->size() += update.entries.size();
				for (size_t pos = 0; pos < depth; ++pos) {
					// foreach keypos

					// populate newly_inserted_children
					auto newly_inserted_children = entry_subset_for_pos(update.entries, pos);

					// If a full_node is copied, some child mappings are altered some are added.
					// For child mappings that stay the same the childs ref_count must be increased.
					// Obviously, there is now a new full_node (this one) which references them.
					if (node_origin == NodeOrigin::Copied) {
						for (auto const &[child_mapping_key_part, id_child] : fn_ptr->edges(pos)) {
							if (not newly_inserted_children.contains(child_mapping_key_part)) {
								// child was here before

								if constexpr (ht_hsi_depth2)
									if (id_child.is_sen())
										continue;// ignore inplace children

								child_update_requests_.apply_ref_count_delta(id_child, 1);
							}
						}
					}

					for (auto &[key_part, child_inserted_entries] : newly_inserted_children) {
						assert(child_inserted_entries.size() > 0);
						if constexpr (node_origin != NodeOrigin::JustCreated) {
							auto [child_exists, child_it] = fn_ptr->find(pos, key_part);
							if (child_exists) {
								// queue insert of child's children and set identifier
								container::deref(child_it) = child_update_requests_.insert_into_node(container::deref(child_it), std::move(child_inserted_entries), node_origin == NodeOrigin::Moved);
								continue;// child exists, no need to create new one further down
							}
						}

						// if node does not exist yet ...

						auto &edges = fn_ptr->edges(pos);
						if constexpr (ht_hsi_depth2) {
							if (child_inserted_entries.size() == 1) {
								// if child can be inplace and there is only one entry to be inserted
								// => the data fits inplace, so put it there

								edges[key_part] = RawIdentifier_t<depth - 1>{child_inserted_entries[0]};
								continue;// inplace, no need to add node further down
							}
						}

						// if the child can not be inplace and there is more than 1 entry to be inserted => create node
						edges[key_part] = child_update_requests_.add_node(std::move(child_inserted_entries));
					}
				}
			}
		}


		template<NodeOrigin node_origin>
		void apply_fn_entry_erasure(FNEntriesUpdate<depth> &&update, FNPtr fn_ptr) {
			assert(update.mode == EntriesUpdateMode::ERASE);

			if constexpr (depth > 1) {
				// update size
				fn_ptr->size() -= update.entries.size();// ok because exec invariant 1
				assert(fn_ptr->size() > 1);

				// apply changes to copy
				for (size_t pos = 0; pos < depth; ++pos) {
					auto &edges = fn_ptr->edges(pos);

					if constexpr (node_origin == NodeOrigin::Copied) {
						auto changes = entry_subset_for_pos(update.entries, pos);

						for (auto edges_iter = edges.begin(); edges_iter != edges.end();) {
							const key_part_type key_part = edges_iter->first;
							auto &child_id = container::deref(edges_iter);
							if (auto changes_iter = changes.find(key_part); changes_iter != changes.end()) {
								child_id = child_update_requests_.remove_from_node(child_id, std::move(changes_iter->second), false);
								if (child_id.empty()) {
									edges_iter = edges.erase(edges_iter);
									continue;
								}
							} else {
								child_update_requests_.apply_ref_count_delta(child_id, 1);
							}
							++edges_iter;
						}
					} else {// NodeOrigin::Moved
						for (auto &&[key_part, subset] : entry_subset_for_pos(update.entries, pos)) {
							assert(edges.contains(key_part));
							auto edges_iter = edges.find(key_part);
							auto &child_id = container::deref(edges_iter);
							child_id = child_update_requests_.remove_from_node(child_id, std::move(subset), true);
							if (child_id.empty())
								edges.erase(edges_iter);
						}
					}
				}
			} else {// depth == 1
				// nodes with size_after == 0 don't need to be enqueued
				// nodes with size_after == 1 will be enqueued as SEN checks instead of normal changes
				assert(fn_ptr->size() > update.entries.size() + 1);
				// copy->size() is tracked automatically since we are at depth 1

				for (auto const &entry : update.entries) {
					fn_ptr->edges().erase(entry.key()[0]);
				}
			}
		}

		void apply_sen_update(RawIdentifier_t<depth> node_id, SENChange &&update) noexcept {
			assert(
					[&]() {
						if (!update.entry.has_value())
							return true;
						if (node_id != RawIdentifier_t<depth>{*update.entry})
							return false;
						if (std::ranges::any_of((*update.entry).key(),
												[](auto &key_part) {
													return key_part == typename htt_t::key_part_type{};
												}))
							return false;
						if ((*update.entry).value() == typename htt_t::value_type{})
							return false;
						return true;
					}());
			// if this is just a ref_count update of an existing node, e.g., because the parent node was copied, the entry is not set.
			if (update.ref_count_delta == 0)
				return;

			auto sens_iter = sens().find(node_id);
			if (update.ref_count_delta < 0 || sens_iter != sens().end()) {
				auto sen_ptr = container::deref(sens_iter);
				assert(static_cast<ssize_t>(sen_ptr->ref_count()) + update.ref_count_delta >= 0);
				sen_ptr->ref_count() += update.ref_count_delta;
				if (sen_ptr->ref_count() == 0) {
					sens().erase(sens_iter);
					sens_lifecycle().delete_(sen_ptr);
				}
			} else {
				assert(
						[&]() {
							if (!update.entry.has_value())
								return false;
							if (node_id != RawIdentifier_t<depth>{*update.entry})
								return false;
							if (std::ranges::any_of((*update.entry).key(),
													[](auto &key_part) {
														return key_part == typename htt_t::key_part_type{};
													}))
								return false;
							if ((*update.entry).value() == typename htt_t::value_type{})
								return false;
							return true;
						}());
				auto sen_ptr = sens_lifecycle().new_(update.entry.value(), update.ref_count_delta);
				sens().emplace(node_id, sen_ptr);
			}
		}
	};

}// namespace dice::hypertrie::internal::raw::node_context::update_details


#endif//APPLYUPDATE_HPP
