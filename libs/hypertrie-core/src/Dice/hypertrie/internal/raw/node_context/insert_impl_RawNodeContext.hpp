#ifndef HYPERTRIE_INSERT_IMPL_RAWNODECONTEXT_HPP
#define HYPERTRIE_INSERT_IMPL_RAWNODECONTEXT_HPP

#include <Dice/hypertrie/internal/raw/node/NodeStorage.hpp>
#include <Dice/hypertrie/internal/raw/node_context/ContextLevelChanges.hpp>

namespace hypertrie::internal::raw {

	template<size_t max_depth, HypertrieCoreTrait tri_t>
	struct insert_impl_RawNodeContext {
		using tri = tri_t;

		template<size_t depth>
		static void exec(NodeStorage<max_depth, tri> &node_storage,
						 NodeContainer<depth, tri> &nodec,
						 std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries) {
			ContextLevelChanges<depth, tri> changes;

			if (entries.empty())
				return;

			if (nodec.empty()) {
				nodec.identifier() = changes.add_node(std::move(entries));
			} else {
				nodec.identifier() = changes.insert_into_node(nodec.identifier(), std::move(entries), true);
			}

			apply<depth>(node_storage, changes);

			assert(not nodec.identifier().empty());
			if constexpr (not(depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>)) {
				nodec = node_storage.template lookup<depth>(nodec.identifier());
				assert(not nodec.identifier().empty());
			} else {
				if (nodec.identifier().is_sen()) {
					nodec.void_node_ptr() = {};
					return;
				} else {
					nodec = FNContainer<depth, tri>{nodec.identifier(),
													node_storage.template lookup<depth, FullNode>(nodec.identifier())};
					assert(not nodec.identifier().empty());
				}
			}
		}

		template<size_t depth>
		static inline void update_ref_count(size_t &node_ref_count, Identifier<depth, tri> node_id, tsl::sparse_map<Identifier<depth, tri>, ssize_t> &fn_deltas){
			auto new_ref_count = ssize_t(node_ref_count) + fn_deltas[node_id];
			node_ref_count = (new_ref_count > 0) ? size_t(new_ref_count) : 0UL;
		}

		template<size_t depth>
		static void apply(NodeStorage<max_depth, tri> &node_storage,
						  ContextLevelChanges<depth, tri> &lv_changes) {
			ContextLevelChanges<depth - 1, tri> next_level_changes;
			using Identifier_t = Identifier<depth, tri>;

			auto &full_nodes_storage_ = node_storage.template nodes<depth, FullNode>();
			auto &full_nodes_ = full_nodes_storage_.nodes();
			auto &full_nodes_lifecycle = full_nodes_storage_.node_lifecycle();
			lv_changes.calc_moveables(full_nodes_storage_);

			if constexpr (not(depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>))
				// create, delete or update ref_count for Single Entry Nodes
				for (auto it = lv_changes.SEN_new_ones.begin(); it != lv_changes.SEN_new_ones.end(); ++it) {
					Identifier_t id_after = it->first;
					auto &change = it.value();
					// if a new node is created, this update change.entry accordingly.
					// change.entry might be used for creating FullNodes that were formed by inserting entries into a SingleEntryNode
					node_storage.template update_or_create_sen<depth>(id_after, change.entry, change.ref_count_delta);
				}

			// insert entries into existing full nodes
			// here, only full nodes that will be deleted and can potentially be reused are considered.
			// If possible, the node will be reused in the last
			for (const auto &[id_before, ids_after] : lv_changes.moveables_fns) {
				auto &changes = lv_changes.FN_changes[id_before];

				std::optional<Identifier_t> last_id_after;

				// find an id_after which is not yet done (= in done_fns)
				for (const auto &id_after : ids_after)
					if (not lv_changes.done_fns.contains(id_after)) {
						last_id_after = id_after;
						break;
					}

				// lookup node_before and detach it from its SpecificNodeStorage
				auto node_before_it = full_nodes_.find(id_before);
				auto node_before = node_before_it.value();
				full_nodes_.erase(node_before_it);

				// the node_before is gone after we reused only once.
				// So we need to process all other changes which require node_before first
				for (const auto &[id_after, entries] : changes) {
					assert(id_before != id_after);
					if (last_id_after.has_value() and id_after == last_id_after.value())
						continue;
					if (lv_changes.done_fns.contains(id_after))
						continue;

					if (not ids_after.contains(id_after)) {// we know it exists already, and we only need to update the ref_count
						full_nodes_.find(id_after).value()->ref_count() += lv_changes.fn_deltas[id_after];
					} else {// we know that it does not already exist (and thus do not need to check that

						auto copied_node = full_nodes_lifecycle.new_(*node_before);
						copied_node->ref_count() = lv_changes.fn_deltas[id_after];

						insert_into_full_node<depth>(next_level_changes, full_nodes_, id_after, copied_node, std::move(entries));
					}
					// entries

					lv_changes.done_fns.insert(id_after);
				}

				// reuse (or delete) node_before
				if (last_id_after.has_value()) {

					auto id_after = last_id_after.value();
					assert(id_before != id_after);
					assert(ssize_t(node_before->ref_count()) + lv_changes.fn_deltas[id_before] <= 0);
					node_before->ref_count() = lv_changes.fn_deltas[id_after];

					auto entries = std::move(changes[id_after]);

					insert_into_full_node<depth, false, true>(next_level_changes, full_nodes_, id_after, node_before, std::move(entries));
					lv_changes.done_fns.insert(id_after);
				} else {// or delete
					full_nodes_lifecycle.delete_(node_before);
				}
				lv_changes.done_fns.insert(id_before);
			}

			for (const auto &[id_before, changes] : lv_changes.FN_changes) {
				if (lv_changes.done_fns.contains(id_before))
					continue;
				// lookup node_before
				assert(full_nodes_.find(id_before) != full_nodes_.end());
				auto node_before = full_nodes_.find(id_before).value();

				for (const auto &[id_after, entries] : changes) {
					assert(id_before != id_after);
					if (lv_changes.done_fns.contains(id_after))
						continue;
					assert(not entries.empty());

					if (auto found = full_nodes_.find(id_after); found != full_nodes_.end()) {// node already exists: we only need to update the ref_count
						found.value()->ref_count() += lv_changes.fn_deltas[id_after];
					} else {// node doesn't exist so far: we copy the node before and insert the new entries

						auto copied_node = full_nodes_lifecycle.new_(*node_before);
						copied_node->ref_count() = lv_changes.fn_deltas[id_after];

						insert_into_full_node<depth>(next_level_changes, full_nodes_, id_after, copied_node, std::move(entries));
					}

					lv_changes.done_fns.insert(id_after);
				}

				// update the ref_count of node_before
				update_ref_count(node_before->ref_count(), id_before, lv_changes.fn_deltas);

				// if the ref_count reaches 0, remove the node
				if (node_before->ref_count() == 0) {
					full_nodes_.erase(id_before);
					full_nodes_lifecycle.delete_(node_before);
				}
				lv_changes.done_fns.insert(id_before);
			}

			for (auto it = lv_changes.FN_new_ones.begin(); it != lv_changes.FN_new_ones.end(); ++it) {
				Identifier_t id_after = it->first;
				if (not lv_changes.done_fns.contains(id_after)) {
					typename ContextLevelChanges<depth, tri>::FN_New &change = it.value();
					Identifier_t id_before = change.sen_node_before;


					if constexpr (not (depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>)) {
						auto &se_nodes_ =  node_storage.template nodes<depth, SingleEntryNode>().nodes();
						if (id_before.is_sen()) {
							if (auto found = lv_changes.SEN_new_ones.find(id_before); found != lv_changes.SEN_new_ones.end()) {
								assert(found->second.entry != decltype(found->second.entry){});
								change.entries.emplace_back(found->second.entry);
							} else {
								assert(se_nodes_.contains(id_before));
								auto &sen_node_before = se_nodes_[id_before];
								change.entries.emplace_back(*sen_node_before);
							}
						}
					}
					auto fn_node_ptr = node_storage.template lookup<depth, FullNode>(id_after);
					if (fn_node_ptr) {
						assert(ssize_t(fn_node_ptr->ref_count()) + lv_changes.fn_deltas[id_after] > 0);
						fn_node_ptr->ref_count() += lv_changes.fn_deltas[id_after];
					} else {
						auto new_node = full_nodes_lifecycle.new_with_alloc(lv_changes.fn_deltas[id_after]);
						insert_into_full_node<depth, true>(next_level_changes, full_nodes_, id_after, new_node, std::move(change.entries));
					}
					lv_changes.done_fns.insert(id_after);
				}
			}
			assert(lv_changes.done_fns.size() == lv_changes.fn_deltas.size());

			if constexpr (depth > 1) {
				apply(node_storage, next_level_changes);
			}
		}
		template<size_t depth, bool node_is_empty = false, bool reused_node = false>
		static void insert_into_full_node(ContextLevelChanges<depth - 1, tri> &next_level_changes,
										  typename SpecificNodeStorage<depth, tri, FullNode>::Map_t &full_nodes_,
										  Identifier<depth, tri> id_after,
										  typename FNContainer<depth, tri>::NodePtr copied_node,
										  std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries) {
			assert(not full_nodes_.contains(id_after));
			full_nodes_.insert({id_after, copied_node});
			// TODO: this does not cover value changes (for non-boolean-valued hypertries) yet, I think ...
			if constexpr (depth == 1) {
				for (const auto &entry : entries)
					copied_node->insert_or_assign(entry.key(), entry.value());
			} else {
				copied_node->size() += entries.size();
				for (const size_t pos : iter::range(depth)) {
					tsl::sparse_map<typename tri::key_part_type, std::vector<SingleEntry<depth - 1, tri_with_stl_alloc<tri>>>> children_inserted_keys{};

					// populate children_inserted_keys
					for (const auto &entry : entries)
						children_inserted_keys[entry.key()[pos]].emplace_back(entry.key().subkey(pos), entry.value());

					for (auto &[key_part, child_inserted_entries] : children_inserted_keys) {
						assert(child_inserted_entries.size() > 0);
						if constexpr (not node_is_empty) {
							auto [key_part_exists, iter] = copied_node->find(pos, key_part);
							if (key_part_exists) {
								iter.value() = next_level_changes.insert_into_node(iter->second, child_inserted_entries, reused_node);
								continue;
							}
						}

						auto &edges = copied_node->edges(pos);
						if constexpr (depth == 2 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) {
							if (child_inserted_entries.size() == 1) {
								edges[key_part] = Identifier<depth - 1, tri>{child_inserted_entries[0]};
								continue;
							}
						}
						edges[key_part] = next_level_changes.add_node(child_inserted_entries);
					}
				}
			}
		}
	};
}// namespace hypertrie::internal::raw
#endif//HYPERTRIE_INSERT_IMPL_RAWNODECONTEXT_HPP
