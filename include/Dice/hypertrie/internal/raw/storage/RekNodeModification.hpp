#ifndef HYPERTRIE_REKNODEMODIFICATION_HPP
#define HYPERTRIE_REKNODEMODIFICATION_HPP

#include "Dice/hypertrie/internal/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/raw/node/NodeContainer.hpp"
#include "Dice/hypertrie/internal/raw/node/TensorHash.hpp"
#include "Dice/hypertrie/internal/raw/storage/Entry.hpp"
#include "Dice/hypertrie/internal/raw/storage/NodeModificationPlan.hpp"
#include "Dice/hypertrie/internal/raw/storage/NodeStorage.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp"

#include <robin_hood.h>
#include <tsl/hopscotch_map.h>

namespace hypertrie::internal::raw {

	template<size_t node_storage_depth,
			 size_t update_depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			 typename = typename std::enable_if_t<(node_storage_depth >= 1)>>
	class RekNodeModification {
	public:
		using tri = tri_t;
		/// public definitions
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

	private:
		static const constexpr long INC_COUNT_DIFF_AFTER = 1;
		static const constexpr long DEC_COUNT_DIFF_AFTER = -1;
		static const constexpr long INC_COUNT_DIFF_BEFORE = -1;
		static const constexpr long DEC_COUNT_DIFF_BEFORE = 1;

		template<size_t depth>
		using NodeStorage_t = NodeStorage<depth, tri>;

		template<size_t depth>
		using Modification_t = NodeModificationPlan<depth, tri>;

		template<size_t depth>
		using LevelModifications_t = robin_hood::unordered_node_set<Modification_t<depth>, absl::Hash<Modification_t<depth>>>;

		using PlannedModifications = util::IntegralTemplatedTuple<LevelModifications_t, 1, update_depth>;


		template <size_t depth>
		struct CountDiffAndNodePtr{ long count_diff; NodePtr<depth, tri> node_ptr; };

		template <size_t depth>
		using LevelRefChanges = tsl::sparse_map<TensorHash, CountDiffAndNodePtr<depth>>;

		using RefChanges = util::IntegralTemplatedTuple<LevelRefChanges, 1, update_depth>;

		template <size_t depth>
		using re = RawEntry_t<depth, tri>;

		template <size_t depth>
		using Entry = typename re<depth>::RawEntry;


	public:
		NodeStorage_t<node_storage_depth> &node_storage;

		NodeContainer<update_depth, tri> &nodec;

		PlannedModifications planned_multi_updates{};

		RefChanges ref_changes{};

		template<size_t updates_depth>
		auto getRefChanges()
				-> LevelRefChanges<updates_depth> & {
			return ref_changes.template get<updates_depth>();
		}

		// extract a change from count_changes
		template<size_t updates_depth>
		void planChangeCount(const TensorHash hash, const long count_change) {
			LevelRefChanges<updates_depth> &count_changes = getRefChanges<updates_depth>();
			count_changes[hash].count_diff += count_change;
		};


		template<size_t updates_depth>
		auto getPlannedModifications()
				-> LevelModifications_t<updates_depth> & {
			return planned_multi_updates.template get<updates_depth>();
		}

		template<size_t updates_depth>
		void planUpdate(Modification_t<updates_depth> planned_update, const long count_diff) {
			auto &planned_updates = getPlannedModifications<updates_depth>();
			if (not planned_update.hashBefore().empty())
				planChangeCount<updates_depth>(planned_update.hashBefore(), -1 * count_diff);
			planChangeCount<updates_depth>(planned_update.hashAfter(), count_diff);
			planned_updates.insert(std::move(planned_update));
		}


		RekNodeModification(NodeStorage_t<node_storage_depth> &nodeStorage, NodeContainer<update_depth, tri> &nodec)
			: node_storage(nodeStorage), nodec{nodec} {}

		void apply_decrement_ref_count(const size_t decrement = 1) {
			planChangeCount<update_depth>(nodec.hash(), decrement * DEC_COUNT_DIFF_AFTER);
			apply_update_rek<update_depth>();
		}

		auto apply_update(std::vector<RawKey<update_depth>> keys) {
			Modification_t<update_depth> update{};
			if (keys.empty())
				return;
			else if (nodec.empty())
				if (keys.size() == 1)
					update.modOp() = ModificationOperations::NEW_COMPRESSED_NODE;
				else
					update.modOp() = ModificationOperations::NEW_UNCOMPRESSED_NODE;
			else if (nodec.isCompressed())
				update.modOp() = ModificationOperations::INSERT_INTO_COMPRESSED_NODE;
			else
				update.modOp() = ModificationOperations::INSERT_INTO_UNCOMPRESSED_NODE;

			update.hashBefore() = nodec.hash();
			update.entries() = std::move(keys);

			planUpdate(std::move(update), INC_COUNT_DIFF_AFTER);

			apply_update_rek<update_depth>();
		}

		void apply_update(const RawKey<update_depth> &key, const value_type value, const value_type old_value) {
			if (value == old_value)
				return;

			bool value_deleted = value == value_type{};

			bool value_changes = old_value != value_type{};

			Modification_t<update_depth> update{};
			update.hashBefore() = nodec;
			update.addEntry(key, value);
			if (value_deleted) {
				update.modOp() = ModificationOperations::REMOVE_FROM_UC;
				throw std::logic_error{"deleting values from hypertrie is not yet implemented. "};
			} else if (value_changes) {
				update.modOp() = ModificationOperations::CHANGE_VALUE;
				update.oldValue() = old_value;
			} else {// new entry
				if (nodec.empty())
					update.modOp() = ModificationOperations::NEW_COMPRESSED_NODE;
				else if (nodec.isCompressed())
					update.modOp() = ModificationOperations::INSERT_INTO_COMPRESSED_NODE;
				else
					update.modOp() = ModificationOperations::INSERT_INTO_UNCOMPRESSED_NODE;
			}

			if(not update.hashAfter().empty()) {
				auto nc_after = node_storage.template getNode<update_depth>(update.hashAfter());
				if (not nc_after.empty()) {
					planChangeCount<update_depth>(update.hashAfter(), INC_COUNT_DIFF_AFTER);
					if(not update.hashBefore().empty()) {
						planChangeCount<update_depth>(update.hashBefore(), DEC_COUNT_DIFF_AFTER);
					}
					apply_update_rek<update_depth>();
					nodec = nc_after;
					return;
				}
			}

			planUpdate(std::move(update), INC_COUNT_DIFF_AFTER);
			apply_update_rek<update_depth>();
		}

		template<size_t depth>
		void apply_update_rek() {

			LevelModifications_t<depth> &multi_updates = getPlannedModifications<depth>();

			// nodes_before that are have ref_count 0 afterwards -> those could be reused/moved for nodes after
			tsl::sparse_map<TensorHash, NodePtr<depth, tri_t>> unreferenced_nodes_before{}; // TODO: store the pointer alongside to safe one resolve

			tsl::sparse_map<TensorHash, size_t> new_after_nodes{};

			// extract a change from count_changes
			auto pop_after_count_change = [&](TensorHash hash) -> std::tuple<bool, size_t> {
				if (auto changed = new_after_nodes.find(hash); changed != new_after_nodes.end()) {
					auto diff = changed->second;
					new_after_nodes.erase(changed);
				  return {diff != 0, diff};
			  } else
				  return {false, 0};
			};


			// check if a hash is in count_changes
			auto peak_after_count_change = [&](TensorHash hash) -> bool {
			  if (auto changed = new_after_nodes.find(hash); changed != new_after_nodes.end()) {
				  auto diff = changed->second;
				  return diff != 0;
			  } else
				  return false;
			};

			// process reference changes to nodes_before
			for (const auto &[hash, diff_u_ptr] : getRefChanges<depth>()) {
				assert(not hash.empty());
				if (diff_u_ptr.count_diff == 0)
					continue;
				auto nodec = node_storage.template getNode<depth>(hash);
				if (not nodec.null()){
					size_t &ref_count = nodec.ref_count();
					ref_count += diff_u_ptr.count_diff;

					if (ref_count == 0) {
						unreferenced_nodes_before.insert({hash, nodec.node()});
					}
				} else {
					assert(diff_u_ptr.count_diff > 0);
					new_after_nodes.insert({hash, (size_t) diff_u_ptr.count_diff});
				}
			}

			static std::vector<std::pair<Modification_t<depth>, size_t>> moveable_multi_updates{};
			moveable_multi_updates.clear();
			moveable_multi_updates.reserve(multi_updates.size());
			// extract movables
			for (const Modification_t<depth> &update : multi_updates) {
				// skip if it cannot be a movable
				if (update.modOp() == ModificationOperations::NEW_UNCOMPRESSED_NODE or update.modOp() == ModificationOperations::NEW_COMPRESSED_NODE or update.modOp() == ModificationOperations::INSERT_INTO_COMPRESSED_NODE)
					continue;
				// check if it is a moveable. then save it and skip to the next iteration
				auto unref_before = unreferenced_nodes_before.find(update.hashBefore());
				if (unref_before != unreferenced_nodes_before.end()) {
					if (peak_after_count_change(update.hashAfter())) {
						unreferenced_nodes_before.erase(unref_before);
						auto [changes, count] = pop_after_count_change(update.hashAfter());
						// TODO: move the keys here
						moveable_multi_updates.emplace_back(update, count);
					}
				}
			}

			static std::vector<std::pair<Modification_t<depth>, size_t>> unmoveable_multi_updates{};
			unmoveable_multi_updates.clear();
			unmoveable_multi_updates.reserve(multi_updates.size());
			// extract unmovables
			for (const Modification_t<depth> &update : multi_updates) {
				// check if it is a moveable. then save it and skip to the next iteration
				auto [changes, after_count_change] = pop_after_count_change(update.hashAfter());
				if (changes) {
					// TODO: move the keys here
					unmoveable_multi_updates.emplace_back(update, after_count_change);
				}
			}

			assert(new_after_nodes.empty());

			tsl::sparse_map<TensorHash, long> node_before_children_count_diffs{};

			// do unmoveables
			for (auto &[update, count] : unmoveable_multi_updates) {
				assert (count > 0);
				const long node_before_children_count_diff = processUpdate<depth, false>(update, (size_t) count);

				if (node_before_children_count_diff != 0)
					node_before_children_count_diffs[update.hashBefore()] += node_before_children_count_diff;
			}

			// remove remaining unreferenced_nodes_before and update the references of their children
			for (const auto &[hash_before, node_ptr] : unreferenced_nodes_before) {
				if (hash_before.isCompressed()){
					removeUnreferencedNode<depth, NodeCompression::compressed>(hash_before);
				} else {
					auto children_count_diff_it = node_before_children_count_diffs.find(hash_before);
					long children_count_diff = DEC_COUNT_DIFF_BEFORE;
					if (children_count_diff_it != node_before_children_count_diffs.end()){
						children_count_diff += children_count_diff_it->second;
						node_before_children_count_diffs.erase(children_count_diff_it);
					}

					removeUnreferencedNode<depth, NodeCompression::uncompressed>(hash_before, node_ptr, children_count_diff);
				}
			}

			// update the references of children not covered above (children of nodes that were not removed)
			for (const auto &[hash_before, children_count_diff] : node_before_children_count_diffs) {
				updateChildrenCountDiff<depth>(TensorHash(hash_before), children_count_diff);
			}

			// do moveables
			for (auto &[update, count] : moveable_multi_updates) {
				processUpdate<depth, true>(update, (size_t) count);
			}

			if constexpr (depth > 1)
				apply_update_rek<depth - 1>();
		}

		template<size_t depth>
		void updateChildrenCountDiff(const TensorHash &hash, const long &children_count_diff){
			if (children_count_diff != 0) {
				auto node = node_storage.template getUncompressedNode<depth>(hash).uncompressed_node();
				this->template updateChildrenCountDiff<depth>(node, children_count_diff);
			}
		}

		template<size_t depth>
		void updateChildrenCountDiff(UncompressedNode<depth, tri> * const node, const long &children_count_diff){
			if constexpr (depth > 1){
				for (size_t pos : iter::range(depth)) {
					for (const auto &[_, child_hash] : node->edges(pos)) {
						if constexpr (not (depth == 2 and tri::is_bool_valued and tri::is_lsb_unused))
							planChangeCount<depth -1 >(child_hash, -1*children_count_diff);
						else {
							if (child_hash.isUncompressed())
								planChangeCount<depth -1 >(TensorHash(child_hash), -1*children_count_diff);
						}
					}
				}
			}
		}

		template<size_t depth, NodeCompression compression>
		void removeUnreferencedNode(const TensorHash hash, Node<depth, compression, tri> *node = nullptr, long children_count_diff = 0){
			if constexpr(compression == NodeCompression::uncompressed)
				if (children_count_diff != 0)
					this->template updateChildrenCountDiff<depth>(node, children_count_diff);

			node_storage.template deleteNode<depth>(hash);
			if constexpr (depth == update_depth)
				if (this->nodec.hash() == hash)
					this->nodec = {};
		}

		template<size_t depth, bool reuse_node_before = false>
		long processUpdate(Modification_t<depth> &update, const size_t after_count_diff) {
			long node_before_children_count_diff = 0;

			switch (update.modOp()) {
				case ModificationOperations::CHANGE_VALUE:
					if constexpr (not tri::is_bool_valued) {
						if (update.hashBefore().isCompressed())
							changeValue<depth, NodeCompression::compressed, reuse_node_before>(update, after_count_diff);
						else
							node_before_children_count_diff = changeValue<depth, NodeCompression::uncompressed, reuse_node_before>(update, after_count_diff);
					}
					break;
				case ModificationOperations::NEW_COMPRESSED_NODE:
					if constexpr(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued))
						insertCompressedNode<depth>(update, after_count_diff);
					break;
				case ModificationOperations::INSERT_INTO_UNCOMPRESSED_NODE:
					node_before_children_count_diff = insertBulkIntoUC<depth, reuse_node_before>(update, after_count_diff);
					break;
				case ModificationOperations::INSERT_INTO_COMPRESSED_NODE:
					if constexpr (not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued))
						insertBulkIntoC<depth>(update, after_count_diff);
					break;
				case ModificationOperations::NEW_UNCOMPRESSED_NODE:
					newUncompressedBulk<depth>(update, after_count_diff);
					break;
				case ModificationOperations::REMOVE_FROM_UC:
					assert(false);// not yet implemented
					break;
				default:
					assert(false);
			}
			return node_before_children_count_diff;
		}

		template<size_t depth>
		void insertCompressedNode(const Modification_t<depth> &update, const size_t after_count_diff) {

			auto nodec_after = node_storage.template newCompressedNode<depth>(
					update.firstKey(), update.firstValue(), after_count_diff, update.hashAfter());
			if constexpr (depth == update_depth)
				this->nodec = nodec_after;
		}

		template<size_t depth, NodeCompression compression, bool reuse_node_before = false>
		long changeValue(const Modification_t<depth> &update, const size_t after_count_diff) {
			long node_before_children_count_diff = 0;
			SpecificNodeContainer<depth, compression, tri> nc_before = node_storage.template getNode<depth, compression>(update.hashBefore());
			assert(not nc_before.null());
			Node<depth, compression, tri> *node_before = nc_before.template specific_node<compression>();

			SpecificNodeContainer<depth, compression, tri> nc_after;

			// reusing the node_before
			if constexpr (reuse_node_before) {// node before ref_count is zero -> maybe reused

				// create updates for sub node
				if constexpr (compression == NodeCompression::uncompressed) {
					if constexpr (depth > 1) {
						// change the values of children recursively


						static constexpr const auto subkey = &tri::template subkey<depth>;
						for (const size_t pos : iter::range(depth)) {
							auto sub_key = subkey(update.firstKey(), pos);
							auto key_part = update.firstKey()[pos];

							Modification_t<depth - 1> child_update{};
							child_update.modOp() = ModificationOperations::CHANGE_VALUE;
							child_update.addEntry(sub_key, update.firstValue());
							child_update.oldValue() = update.oldValue();

							child_update.hashBefore() = node_before->child(pos, key_part);
							assert(not child_update.hashBefore().empty());

							planUpdate(std::move(child_update), INC_COUNT_DIFF_AFTER);
						}
					}
				}

				// update the node_before with the after_count and value
				nc_after = node_storage.template changeNodeValue<depth, compression, false>(
						nc_before, update.firstKey(), update.oldValue(), update.firstValue(), after_count_diff, update.hashAfter());

			} else {// leaving the node_before untouched
				if constexpr (compression == NodeCompression::compressed)
					node_storage.template newCompressedNode<depth>( // TODO: use firstKey
							node_before->key(), update.firstValue(), after_count_diff, update.hashAfter());
				else {
					if constexpr (depth > 1) {
						node_before_children_count_diff = INC_COUNT_DIFF_BEFORE;
					}

					if constexpr (compression == NodeCompression::uncompressed and depth > 1) {

						static constexpr const auto subkey = &tri::template subkey<depth>;
						for (const size_t pos : iter::range(depth)) {
							auto sub_key = subkey(update.firstKey(), pos);
							auto key_part = update.firstKey()[pos];

							Modification_t<depth - 1> child_update{};
							child_update.modOp() = ModificationOperations::CHANGE_VALUE;
							child_update.addEntry(sub_key, update.firstValue());
							child_update.oldValue() = update.oldValue();

							child_update.hashBefore() = node_before->child(pos, key_part);
							assert(not child_update.hashBefore().empty());

							planUpdate(std::move(child_update), INC_COUNT_DIFF_AFTER);
						}
					}

					nc_after = node_storage.template changeNodeValue<depth, compression, true>(
							nc_before, update.firstKey(), update.oldValue(), update.firstValue(), after_count_diff, update.hashAfter());
				}
			}

			if constexpr (depth == update_depth)
				this->nodec = nc_after;

			return node_before_children_count_diff;
		}

		template<size_t depth>
		void newUncompressedBulk(const Modification_t<depth> &update, const size_t after_count_diff) {
			// TODO: implement for valued
			static constexpr const auto subkey = &tri::template subkey<depth>;
			using red = re<depth>;
			// move or copy the node from old_hash to new_hash
			auto &storage = node_storage.template getNodeStorage<depth, NodeCompression::uncompressed>();
			// make sure everything is set correctly
			assert(update.hashBefore().empty());
			assert(storage.find(update.hashAfter()) == storage.end());

			// create node and insert it into the storage
			UncompressedNode<depth, tri> *const node = new UncompressedNode<depth, tri>{(size_t) after_count_diff};
			storage.insert({update.hashAfter(), node});

			if constexpr (depth > 1)
				node->size_ = update.entries().size();

			// populate the new node
			for (const size_t pos : iter::range(depth)) {
				if constexpr (depth == 1) {
					for (const Entry<depth> &entry : update.entries()){
						if constexpr (tri_t::is_bool_valued)
							node->edges().insert(red::key(entry)[0]);
						else
							node->edges().emplace(red::key(entry)[0], red::value(entry));
					}
				} else {
					// # group the subkeys by the key part at pos

					// maps key parts to the keys to be inserted for that child
					robin_hood::unordered_map<key_part_type, std::vector<Entry<depth - 1>>> children_inserted_keys{};

					// populate children_inserted_keys
					for (const Entry<depth> &entry : update.entries())
						children_inserted_keys[red::key(entry)[pos]]
								.push_back(re<depth-1>::make_Entry(subkey(red::key(entry), pos), red::value(entry)));

					// process the changes to the node at pos and plan the updates to the sub nodes
					for (auto &[key_part, child_inserted_entries] : children_inserted_keys) {
						assert(child_inserted_entries.size() > 0);

						// plan the new subnodes and insert references
						Modification_t<depth - 1> child_update{};
						if (child_inserted_entries.size() == 1){
							if constexpr (not (depth == 2 and tri::is_bool_valued and tri::is_lsb_unused)) {
								child_update.modOp() = ModificationOperations::NEW_COMPRESSED_NODE;
							} else {
								node->edges(pos)[key_part] = TaggedTensorHash<tri>{child_inserted_entries[0][0]};
								continue;
							}
						} else
							child_update.modOp() = ModificationOperations::NEW_UNCOMPRESSED_NODE;
						child_update.entries() = std::move(child_inserted_entries);

						// insert reference to subnode
						node->edges(pos)[key_part] = child_update.hashAfter();
						// submit subnode plan
						planUpdate(std::move(child_update), INC_COUNT_DIFF_AFTER);
					}
				}
			}
			if constexpr (depth == update_depth)
				this->nodec = {update.hashAfter(), node};
		}

		template<size_t depth>
		void insertBulkIntoC(Modification_t<depth> &update, const long after_count_diff) {
			auto &storage = node_storage.template getNodeStorage<depth, NodeCompression::compressed>();
			assert(storage.find(update.hashBefore()) != storage.end());
			assert(storage.find(update.hashAfter()) == storage.end());

			CompressedNode<depth, tri> const *const node_before = storage[update.hashBefore()];

			update.modOp() = ModificationOperations::NEW_UNCOMPRESSED_NODE;
			update.addEntry(node_before->key(), node_before->value());
			update.hashBefore() = {};
			newUncompressedBulk<depth>(update, after_count_diff);
		}

		template<size_t depth, bool reuse_node_before = false>
		long insertBulkIntoUC(const Modification_t<depth> &update, const long after_count_diff) {
				static constexpr const auto subkey = &tri::template subkey<depth>;
				using red = re<depth>;
				const long node_before_children_count_diff =
						(not reuse_node_before and depth > 1) ? INC_COUNT_DIFF_BEFORE : 0;

				// move or copy the node from old_hash to new_hash
				auto &storage = node_storage.template getNodeStorage<depth, NodeCompression::uncompressed>();
				auto node_it = storage.find(update.hashBefore());
				assert(node_it != storage.end());
				UncompressedNode<depth, tri> *node = node_it->second;
				if constexpr (reuse_node_before) {// node before ref_count is zero -> maybe reused
					storage.erase(node_it);
				} else {
					node = new UncompressedNode<depth, tri>{*node};
					node->ref_count() = 0;
				}
				assert(storage.find(update.hashAfter()) == storage.end());
				storage[update.hashAfter()] = node;

				// update the node count
				node->ref_count() += after_count_diff;
				if constexpr (depth > 1)
					node->size_ += update.entries().size();

				// update the node (new_hash)
				for (const size_t pos : iter::range(depth)) {
					if constexpr (depth == 1) {
						for (const Entry <depth> &entry : update.entries()){
							if constexpr (tri_t::is_bool_valued)
								node->edges().insert(red::key(entry)[0]);
							else
								node->edges().emplace(red::key(entry)[0], red::value(entry));
						}
					} else {
						// # group the subkeys by the key part at pos

						// maps key parts to the keys to be inserted for that child
						robin_hood::unordered_map<key_part_type, std::vector<Entry<depth - 1>>> children_inserted_keys{};

						// populate children_inserted_keys
						for (const Entry<depth> &entry : update.entries())
							children_inserted_keys[red::key(entry)[pos]]
									.push_back(re<depth-1>::make_Entry(subkey(red::key(entry), pos), red::value(entry)));

						// process the changes to the node at pos and plan the updates to the sub nodes
						for (auto &[key_part, child_inserted_entries] : children_inserted_keys) {
							assert(child_inserted_entries.size() > 0);
							auto [key_part_exists, iter] = node->find(pos, key_part);

							Modification_t<depth - 1> child_update{};
							if constexpr (not (depth == 2 and tri::is_bool_valued and tri::is_lsb_unused)) {
								if (key_part_exists) {
									child_update.hashBefore() = iter->second;
									if (child_update.hashBefore().isCompressed())
										child_update.modOp() = ModificationOperations::INSERT_INTO_COMPRESSED_NODE;
									else
										child_update.modOp() = ModificationOperations::INSERT_INTO_UNCOMPRESSED_NODE;
								} else {
									if (child_inserted_entries.size() == 1)
										child_update.modOp() = ModificationOperations::NEW_COMPRESSED_NODE;
									else
										child_update.modOp() = ModificationOperations::NEW_UNCOMPRESSED_NODE;
								}
							} else {
								if (key_part_exists) {
									if (iter->second.isCompressed()) {
										child_inserted_entries.push_back({iter->second.getKeyPart()});
										child_update.modOp() = ModificationOperations::NEW_UNCOMPRESSED_NODE;
									} else {
										child_update.hashBefore() = iter->second.getTaggedNodeHash();
										child_update.modOp() = ModificationOperations::INSERT_INTO_UNCOMPRESSED_NODE;
									}
								} else {
									if (child_inserted_entries.size() == 1) {
										node->edges(pos)[key_part] = TaggedTensorHash<tri>{child_inserted_entries[0][0]};
										continue;
									} else {
										child_update.modOp() = ModificationOperations::NEW_UNCOMPRESSED_NODE;
									}
								}
							}


							child_update.entries() = std::move(child_inserted_entries);

							// execute changes
							if (key_part_exists)
								if constexpr (not (depth == 2 and tri::is_bool_valued and tri::is_lsb_unused))
									tri::template deref<key_part_type, TensorHash>(iter) = child_update.hashAfter();
								else
									tri::template deref<key_part_type, TaggedTensorHash<tri>>(iter) = child_update.hashAfter();
							else
								node->edges(pos)[key_part] = child_update.hashAfter();

							planUpdate(std::move(child_update), INC_COUNT_DIFF_AFTER);
						}
					}
				}
				if constexpr (depth == update_depth)
					this->nodec = {update.hashAfter(), node};

				return node_before_children_count_diff;

		}


	};
}// namespace hypertrie::internal

#endif//HYPERTRIE_REKNODEMODIFICATION_HPP
