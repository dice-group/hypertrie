#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include <Dice/hypertrie/internal/raw/node/ContextLevelChanges.hpp>
#include <Dice/hypertrie/internal/raw/node/NodeStorage.hpp>


namespace hypertrie::internal::raw {

	template<size_t max_depth, HypertrieCoreTrait_bool_valued tri_t>
	struct update_node_in_context;


	template<size_t max_depth, HypertrieCoreTrait_bool_valued tri_t>
	struct RawHypertrieContext {
		using tri = tri_t;

		NodeStorage<max_depth, tri_t> node_storage_;

		explicit RawHypertrieContext(const typename tri::allocator_type &alloc) : node_storage_(alloc) {}

		template<size_t depth>
		void insert(NodeContainer<depth, tri> &nodec,
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries) {
			update_node_in_context<max_depth, tri>::exec(node_storage_, nodec, entries);
		}
	};

	template<size_t max_depth, HypertrieCoreTrait_bool_valued tri_t>
	struct update_node_in_context {
		using tri = tri_t;

		template<size_t depth>
		static void exec(NodeStorage<max_depth, tri> &node_storage,
						 NodeContainer<depth, tri> &nodec,
						 std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries) {
			ContextLevelChanges<depth, tri> changes;
			//			using Entry = SingleEntry<depth, tri>;

			if (entries.empty())
				return;


			if (nodec.empty()) {
				nodec.identifier() = changes.add_node(entries);
			} else {
				nodec.identifier() = changes.insert_into_node(nodec.identifier(), entries);
			}

			apply<depth>(node_storage, changes);

			if constexpr (not(depth == 1 and tri::taggable_key_part))
				nodec = node_storage.template lookup<depth>(nodec.identifier());
			else
				nodec.void_node_ptr() = {};
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

				for (const auto &id_after : ids_after)
					if (not lv_changes.done_fns.contains(id_after)) {
						last_id_after = id_after;
						break;
					}

				// TODO: look up only if needed
				auto node_before = full_nodes_.find(id_before).value();

				for (const auto &[id_after, entries] : changes) {
					if (last_id_after.has_value() and id_after == last_id_after.value())
						continue;
					if (lv_changes.done_fns.contains(id_after))
						continue;

					{
						if (not ids_after.contains(id_after)) {// we know it exists already, and we only need to update the ref_count
							full_nodes_.find(id_after).value()->ref_count() += lv_changes.fn_deltas[id_after];
						} else {// we know that it does not already exist (and thus do not need to check that

							auto copied_node = full_nodes_lifecycle.new_(*node_before);
							copied_node->ref_count() += lv_changes.fn_deltas[id_after];

							assert(not full_nodes_.contains(id_after));
							full_nodes_.insert({id_after, copied_node});

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
										auto [key_part_exists, iter] = copied_node->find(pos, key_part);

										if (key_part_exists) {
											iter.value() = next_level_changes.insert_into_node(iter->second, child_inserted_entries);
										} else {
											if constexpr (depth == 2 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) {
												iter.value() = next_level_changes.add_node(child_inserted_entries);
											} else {
												if (child_inserted_entries.size() == 1)
													iter.value() = Identifier<depth - 1, tri>{child_inserted_entries[0]};
												else
													iter.value() = next_level_changes.insert_into_node(iter->second, child_inserted_entries);
											}
										}
									}
								}
							}
						}
					}
					// entries

					lv_changes.done_fns.insert(id_after);
				}

				// reuse
				if (last_id_after.has_value()) {
					// TODO: move id_before node and insert entries
					lv_changes.done_fns.insert(last_id_after.value());
				} else {// or delete
						// TODO: remove id_before
				}
				lv_changes.done_fns.insert(id_before);
			}

			for (const auto &[id_before, changes] : lv_changes.FN_changes) {
				for (const auto &[id_after, entries] : lv_changes.FN_changes) {
					if (lv_changes.done_fns.contains(id_after))
						continue;
					// TODO: update node after with id_before, id_after, entries
					//					node_storage.tempalte update_fn<depth>(id_before, id_after, entries)
				}

				if (not lv_changes.done_fns.contains(id_before)) {
					// TODO: update or delete id_before
				}
			}

			for (auto it = lv_changes.FN_new_ones.begin(); it != lv_changes.FN_new_ones.end(); ++it) {
				Identifier_t id_after = it->first;
				if (not lv_changes.done_fns.contains(id_after)) {
					typename ContextLevelChanges<depth, tri>::FN_New &change = it.value();
					Identifier_t id_before = change.sen_node_before;


					if (id_before.is_sen()) {
						change.entries.push_back({lv_changes.SEN_new_ones[id_before].entry});
					}
					// TODO: new node with
					//  Identifier: id_after
					//  entries: change.entries
					lv_changes.done_fns.insert(id_after);
				}
			}
		}
	};


}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_NODECONTEXT_HPP
