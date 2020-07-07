#ifndef HYPERTRIE_NODESTORAGEUPDATE_HPP
#define HYPERTRIE_NODESTORAGEUPDATE_HPP

#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/NodeContainer.hpp"
#include "Dice/hypertrie/internal/node_based/NodeStorage.hpp"
#include "Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <Dice/hypertrie/internal/util/CountDownNTuple.hpp>

#include <tsl/hopscotch_set.h>

namespace hypertrie::internal::node_based {

	template<size_t node_storage_depth,
			 size_t update_depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			 typename = typename std::enable_if_t<(node_storage_depth >= 1)>>
	class NodeStorageUpdate {
	public:
		using tri = tri_t;
		/// public definitions
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		template<typename key, typename value>
		using map_type = typename tri::template map_type<key, value>;
		template<typename key>
		using set_type = typename tri::template set_type<key>;
		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

		template<size_t depth>
		using NodeStorage_t = NodeStorage<depth, tri>;

	private:
		template<size_t depth>
		struct AtomicUpdate;

		template<size_t depth>
		using LevelUpdates_t = std::unordered_set<AtomicUpdate<depth>, absl::Hash<AtomicUpdate<depth>>>;

		using PlannedUpdates = util::CountDownNTuple<LevelUpdates_t, update_depth>;

		using LevelRefChanges = std::unordered_map<TaggedNodeHash, long>;

		using RefChanges = std::array<LevelRefChanges, update_depth>;


		enum struct InsertOp : unsigned int {
			CHANGE_VALUE = 0,
			INSERT_TWO_KEY_UC_NODE,
			INSERT_C_NODE,
			EXPAND_UC_NODE,
			EXPAND_C_NODE,
			REMOVE_FROM_UC
		};

		template<size_t depth>
		struct AtomicUpdate {
			static_assert(depth >= 1);

			InsertOp insert_op{};
			TaggedNodeHash hash_before{};
			TaggedNodeHash hash_after{};
			RawKey<depth> key{};
			value_type value{};
			RawKey<depth> second_key{};
			value_type second_value{};

		public:
			void calcHashAfter(const value_type &old_value) {
				hash_after = hash_before;
				switch (insert_op) {
					case InsertOp::CHANGE_VALUE:
						assert(not hash_before.empty());
						hash_after.changeValue(key, old_value, value);
						break;
					case InsertOp::INSERT_TWO_KEY_UC_NODE:
						assert(hash_before.empty());
						hash_after = TaggedNodeHash::getTwoEntriesNodeHash(key, value, second_key, second_value);
						break;
					case InsertOp::INSERT_C_NODE:
						assert(hash_before.empty());
						hash_after = TaggedNodeHash::getCompressedNodeHash(key, value);
						break;
					case InsertOp::EXPAND_UC_NODE:
						assert(hash_before.isUncompressed());
						hash_after.addEntry(key, value);
						break;
					case InsertOp::EXPAND_C_NODE:
						assert(hash_before.isCompressed());
						hash_after.addEntry(key, value);
						assert(hash_after.isUncompressed());
						break;
				}
			}

			bool operator<(const AtomicUpdate<depth> &other) const {
				return std::make_tuple(this->insert_op, this->hash_before, this->key, this->value, this->second_key, this->second_value) <
					   std::make_tuple(other.insert_op, other.hash_before, other.key, other.value, other.second_key, other.second_value);
			};

			bool operator==(const AtomicUpdate<depth> &other) const {
				return std::make_tuple(this->insert_op, this->hash_before, this->key, this->value, this->second_key, this->second_value) ==
					   std::make_tuple(other.insert_op, other.hash_before, other.key, other.value, other.second_key, other.second_value);
			};

			template <typename H>
			friend H AbslHashValue(H h, const AtomicUpdate<depth>& other) {
				return H::combine(std::move(h), other.hash_before, other.hash_after);
			}
		};

	public:
		NodeStorage_t<node_storage_depth> &node_storage;

		UncompressedNodeContainer<update_depth, tri> &nodec;

		value_type old_value{};

		PlannedUpdates planned_updates{};

		RefChanges ref_changes{};

		template<size_t updates_depth>
		auto getRefChanges()
				-> LevelRefChanges & {
			return ref_changes[updates_depth -1];
		}

		// extract a change from count_changes
		template<size_t updates_depth>
		void planChangeCount(const TaggedNodeHash hash, const long count_change) {
			LevelRefChanges &count_changes = getRefChanges<updates_depth>();
			count_changes[hash] += count_change;
		};


		template<size_t updates_depth>
		auto getPlannedUpdates()
				-> LevelUpdates_t<updates_depth> & {
			return std::get<updates_depth - 1>(planned_updates);
		}

		template<size_t updates_depth>
		void planUpdate(AtomicUpdate<updates_depth> planned_update, const long count_diff) {
			auto &planned_updates = getPlannedUpdates<updates_depth>();
			planned_update.calcHashAfter(this->old_value);
			if (not planned_update.hash_before.empty())
				planChangeCount<updates_depth>(planned_update.hash_before, -1*count_diff);
			planChangeCount<updates_depth>(planned_update.hash_after, count_diff);
			planned_updates.insert(planned_update);
		}


		NodeStorageUpdate(NodeStorage_t<node_storage_depth> &nodeStorage, UncompressedNodeContainer<update_depth, tri> &nodec)
			: node_storage(nodeStorage), nodec{nodec} {}

		void apply_update(const RawKey<update_depth> &key, const value_type value, const value_type old_value) {
			if (value == old_value)
				return;

			this->old_value = old_value;

			bool value_deleted = value == value_type{};

			bool value_changes = old_value != value_type{};

			AtomicUpdate<update_depth> update{};
			update.hash_before = nodec;
			update.key = key;
			update.value = value;
			if (value_deleted) {
				update.insert_op = InsertOp::REMOVE_FROM_UC;
				throw std::logic_error{"deleting values from hypertrie is not yet implemented. "};
			} else if (value_changes) {
				update.insert_op = InsertOp::CHANGE_VALUE;
			} else {// new entry
				update.insert_op = InsertOp::EXPAND_UC_NODE;
			}
			planUpdate(std::move(update), 1);
			apply_update_rek<update_depth>();
		}

		template<size_t depth>
		void apply_update_rek() {

			LevelUpdates_t<depth> &updates = getPlannedUpdates<depth>();

			// all nodes used as nodes before.
			std::unordered_set<TaggedNodeHash> nodes_before{};

			std::unordered_set<TaggedNodeHash> nodes_after{};

			// populate count_changes and nodes_before
			for (const AtomicUpdate<depth> &update : updates) {
				if (not update.hash_before.empty()) {
					nodes_before.insert(update.hash_before);// NodeContainer is not yet materialized
				}
				if (not update.hash_after.empty()) {
					nodes_after.insert(update.hash_after);
				}
			}

			// nodes_before that are have ref_count 0 afterwards -> those could be reused/moved for nodes after
			std::unordered_set<TaggedNodeHash> unreferenced_nodes_before{};

			LevelRefChanges new_after_nodes{};

			// extract a change from count_changes
			auto pop_after_count_change = [&](TaggedNodeHash hash) -> std::tuple<bool, long> {
			  if (auto changed = new_after_nodes.find(hash); changed != new_after_nodes.end()) {
				  auto diff = changed->second;
				  new_after_nodes.erase(changed);
				  return {diff != 0, diff};
			  } else
				  return {false, 0};
			};


			// check if a hash is in count_changes
			auto peak_after_count_change = [&](TaggedNodeHash hash) -> bool {
			  if (auto changed = new_after_nodes.find(hash); changed != new_after_nodes.end()) {
				  auto diff = changed->second;
				  return diff != 0;
			  } else
				  return false;
			};

			// process reference changes to nodes_before
			for (const auto &[hash, count_diff] : getRefChanges<depth>()) {
				assert(not hash.empty());
				if (count_diff == 0)
					continue;
				auto nodec = node_storage.template getNode<depth>(hash);
				if (not nodec.null()){
					assert(not nodec.null());
					size_t *ref_count = [&]() {
						if (nodec.isCompressed())
							return &nodec.compressed_node()->ref_count();
						else
							return &nodec.uncompressed_node()->ref_count();
					}();
					*ref_count += count_diff;

					if (*ref_count == 0) {
						unreferenced_nodes_before.insert(hash);
					}
				} else {
					assert(nodes_after.count(hash));

					new_after_nodes.insert({hash, count_diff});
				}
			}

			static std::vector<std::pair<AtomicUpdate<depth>, long>> moveable_updates{};
			moveable_updates.clear();
			moveable_updates.reserve(updates.size());
			// extract movables
			for (const AtomicUpdate<depth> &update : updates) {
				// skip if it cannot be a movable
				if (update.insert_op == InsertOp::EXPAND_C_NODE or update.insert_op == InsertOp::INSERT_TWO_KEY_UC_NODE)
					continue;
				// check if it is a moveable. then save it and skip to the next iteration
				if (peak_after_count_change(update.hash_after)) {
					auto unref_before = unreferenced_nodes_before.find(update.hash_before);
					if (unref_before != unreferenced_nodes_before.end()) {
						unreferenced_nodes_before.erase(unref_before);
						auto [changes, after_count_change] = pop_after_count_change(update.hash_after);
						moveable_updates.emplace_back(update, after_count_change);
					}
				}
			}

			static std::vector<std::pair<AtomicUpdate<depth>, long>> unmoveable_updates{};
			unmoveable_updates.clear();
			unmoveable_updates.reserve(updates.size());
			// extract unmovables
			for (const AtomicUpdate<depth> &update : updates) {
				// check if it is a moveable. then save it and skip to the next iteration
				auto [changes, after_count_change] = pop_after_count_change(update.hash_after);
				if (changes) {
					unmoveable_updates.emplace_back(update, after_count_change);
				}
			}

			assert(new_after_nodes.empty());
			
			std::unordered_map<TaggedNodeHash, long> node_before_children_count_diffs{};

			// do unmoveables
			for (const auto &[update, count_change] : unmoveable_updates) {
				long node_before_children_count_diff;
				if (update.hash_before.isCompressed())
					node_before_children_count_diff = processUpdate<depth, NodeCompression::compressed, false>(update, count_change);
				else
					node_before_children_count_diff = processUpdate<depth, NodeCompression::uncompressed, false>(update, count_change);
				
				if (node_before_children_count_diff != 0)
					node_before_children_count_diffs[update.hash_before] += node_before_children_count_diff;
			}

			// remove remaining unreferenced_nodes_before and update the references of their children
			for (const auto &hash_before : unreferenced_nodes_before) {
				if (hash_before.isCompressed()){
					removeUnreferencedNode<depth, NodeCompression::compressed>(hash_before);
				} else {
					auto handle = node_before_children_count_diffs.extract(hash_before);
					long children_count_diff = 1;
					if (not handle.empty())
						children_count_diff += handle.mapped();

					removeUnreferencedNode<depth, NodeCompression::uncompressed>(hash_before, children_count_diff);
				}
			}

			// update the references of children not covered above (children of nodes that were not removed)
			for (const auto &[hash_before, children_count_diff] : node_before_children_count_diffs) {
				updateChildrenCountDiff<depth>(hash_before, children_count_diff);
			}

			// do moveables
			for (const auto &[update, count_change] : moveable_updates) {
				if (update.hash_before.isCompressed())
					processUpdate<depth, NodeCompression::compressed, true>(update, count_change);
				else
					processUpdate<depth, NodeCompression::uncompressed, true>(update, count_change);
			}

			if constexpr (depth > 1)
				apply_update_rek<depth - 1>();
		}

		template<size_t depth>
		void updateChildrenCountDiff(const TaggedNodeHash hash,const long children_count_diff){
			assert(hash.isUncompressed());
			if constexpr (depth > 1){
				if (children_count_diff != 0) {
					auto node = node_storage.template getUncompressedNode<depth>(hash).node();
					for (size_t pos : iter::range(depth)) {
						for (const auto &[_, child_hash] : node->edges(pos)) {
							planChangeCount<depth -1 >(child_hash, children_count_diff);
						}
					}
				}
			}
		}

		template<size_t depth, NodeCompression compression>
		void removeUnreferencedNode(const TaggedNodeHash hash, long children_count_diff = 0){
			if constexpr(compression == NodeCompression::uncompressed)
				this->template updateChildrenCountDiff<depth>(hash, children_count_diff);

			node_storage.template deleteNode<depth>(hash);
		}

		template<size_t depth, NodeCompression compression, bool reuse_node_before = false>
		long processUpdate(const AtomicUpdate<depth> &update, const long after_count_diff) {
			auto nc_after = node_storage.template getNode<depth>(update.hash_after);
			assert(nc_after.null());

			long node_before_children_count_diff = 0;
			switch (update.insert_op) {
				case InsertOp::CHANGE_VALUE:
					node_before_children_count_diff = changeValue<depth, compression, reuse_node_before>(update, after_count_diff);
					break;
				case InsertOp::INSERT_TWO_KEY_UC_NODE:
					assert (compression == NodeCompression::uncompressed);
					if constexpr(compression == NodeCompression::uncompressed)
					insertTwoValueUncompressed<depth>(update, after_count_diff);
					break;
				case InsertOp::INSERT_C_NODE:
					assert(compression == NodeCompression::uncompressed);
					if constexpr(compression == NodeCompression::uncompressed)
						insertCompressedNode<depth>(update, after_count_diff);
					break;
				case InsertOp::EXPAND_UC_NODE:
					if constexpr(compression == NodeCompression::uncompressed)
						node_before_children_count_diff = insertIntoUncompressed<depth, reuse_node_before>(update, after_count_diff);
					break;
				case InsertOp::EXPAND_C_NODE:
					if constexpr(compression == NodeCompression::compressed)
						insertTwoValueUncompressed<depth>(update, after_count_diff);
					break;
				case InsertOp::REMOVE_FROM_UC:
					assert(false); // not yet implemented
					break;
			}
			return node_before_children_count_diff;
		}

		template<size_t depth>
		void insertCompressedNode(const AtomicUpdate<depth> &update, const long after_count_diff) {

			node_storage.template newCompressedNode<depth>(
					update.key, update.value, after_count_diff, update.hash_after);
		}

		template<size_t depth, NodeCompression compression, bool reuse_node_before = false>
		long changeValue(const AtomicUpdate<depth> &update, const long after_count_diff) {
			long node_before_children_count_diff = 0;
			SpecificNodeContainer<depth, compression, tri> nc_before = node_storage.template getNode<depth, compression>(update.hash_before);
			assert(not nc_before.null());

			SpecificNodeContainer<depth, compression, tri> nc_after;

			// reusing the node_before
			if constexpr (reuse_node_before) {// node before ref_count is zero -> maybe reused

				// create updates for sub node
				if constexpr (compression == NodeCompression::uncompressed and depth > 1) {
					// change the values of children recursively
					auto *node = nc_before.node();

					static constexpr const auto subkey = &tri::template subkey<depth>;
					for (const size_t pos : iter::range(depth)) {
						auto sub_key = subkey(update.key, pos);
						auto key_part = update.key[pos];

						AtomicUpdate<depth - 1> child_update{};
						child_update.insert_op = InsertOp::CHANGE_VALUE;
						child_update.value = update.value;
						child_update.key = sub_key;

						child_update.hash_before = node->child(pos, key_part);
						assert(not child_update.hash_before.empty());

						planUpdate(std::move(child_update), 1);
					}
				}

				// update the node_before with the after_count and value
				nc_after = node_storage.template changeNodeValue<depth, compression, false>(
						nc_before, update.key, this->old_value, update.value, after_count_diff, update.hash_after);

			} else {// leaving the node_before untouched
				if constexpr (compression == NodeCompression::compressed)
					node_storage.template newCompressedNode<depth>(
							nc_before.node()->key(), update.value, after_count_diff, update.hash_after);
				else {
					if constexpr (depth > 1) {
						node_before_children_count_diff = -1;
					}

					if constexpr (compression == NodeCompression::uncompressed and depth > 1) {
						auto *node = nc_before.node();

						static constexpr const auto subkey = &tri::template subkey<depth>;
						for (const size_t pos : iter::range(depth)) {
							auto sub_key = subkey(update.key, pos);
							auto key_part = update.key[pos];

							AtomicUpdate<depth - 1> child_update{};
							child_update.insert_op = InsertOp::CHANGE_VALUE;
							child_update.value = update.value;
							child_update.key = sub_key;

							child_update.hash_before = node->child(pos, key_part);
							assert(not child_update.hash_before.empty());

							planUpdate(std::move(child_update), 1);
						}
					}

					nc_after = node_storage.template changeNodeValue<depth, compression, true>(
							nc_before, update.key, this->old_value, update.value, after_count_diff, update.hash_after);
				}
			}

			if constexpr (depth == update_depth and compression == NodeCompression::uncompressed)
				this->nodec = nc_after;

			return node_before_children_count_diff;
		}

		template<size_t depth>
		void insertTwoValueUncompressed(AtomicUpdate<depth> update, const long after_count_diff) {
			if (update.insert_op == InsertOp::EXPAND_C_NODE){
				auto *node_before = node_storage.template getCompressedNode<depth>(update.hash_before).node();
				update.second_key = node_before->key();
				update.second_value = node_before->value();
			}
			node_storage.template newUncompressedNode<depth>(
					update.key, update.value,
					update.second_key, update.second_value, after_count_diff, update.hash_after);

			if constexpr (depth > 1) {
				static constexpr const auto subkey = &tri::template subkey<depth>;
				for (const size_t pos : iter::range(depth)) {
					auto sub_key = subkey(update.key, pos);
					auto second_sub_key = subkey(update.second_key, pos);
					auto key_part = update.key[pos];
					auto second_key_part = update.second_key[pos];


					if (key_part == second_key_part) {
						AtomicUpdate<depth - 1> child_update{};
						child_update.insert_op = InsertOp::INSERT_TWO_KEY_UC_NODE;
						child_update.key = sub_key;
						child_update.value = update.value;
						child_update.second_key = second_sub_key;
						child_update.second_value = update.second_value;
						planUpdate(std::move(child_update), 1);
					} else {
						AtomicUpdate<depth - 1> first_child_update{};
						first_child_update.insert_op = InsertOp::INSERT_C_NODE;
						first_child_update.key = sub_key;
						first_child_update.value = update.value;
						planUpdate(std::move(first_child_update), 1);

						AtomicUpdate<depth - 1> second_child_update{};
						second_child_update.insert_op = InsertOp::INSERT_C_NODE;
						second_child_update.key = second_sub_key;
						second_child_update.value = update.second_value;
						planUpdate(std::move(second_child_update), 1);
					}
				}
			}
		}

		template<size_t depth, bool reuse_node_before = false>
		long insertIntoUncompressed(const AtomicUpdate<depth> &update, const long after_count_diff) {
			long node_before_children_count_diff = 0;

			UncompressedNodeContainer<depth, tri> nc_before = node_storage.template getUncompressedNode<depth>(update.hash_before);

			UncompressedNodeContainer<depth, tri> nc_after;

			if constexpr (reuse_node_before) {// node before ref_count is zero -> maybe reused
											  // update the node_before with the after_count and value

				if constexpr (depth > 1) {
					static constexpr const auto subkey = &tri::template subkey<depth>;
					UncompressedNode<depth, tri> *node_before = nc_before.node();
					for (const size_t pos : iter::range(depth)) {
						auto sub_key = subkey(update.key, pos);
						auto key_part = update.key[pos];
						AtomicUpdate<depth - 1> child_update{};
						child_update.key = sub_key;
						child_update.value = update.value;
						TaggedNodeHash child_hash = node_before->child(pos, key_part);
						child_update.hash_before = child_hash;
						if (child_hash.empty()) {
							child_update.insert_op = InsertOp::INSERT_C_NODE;
						} else {
							if (child_hash.isCompressed())
								child_update.insert_op = InsertOp::EXPAND_C_NODE;
							else
								child_update.insert_op = InsertOp::EXPAND_UC_NODE;
						}
						planUpdate(std::move(child_update), 1);
					}
				}

				assert(not nc_before.null());
				nc_after = node_storage.template insertEntryIntoUncompressedNode<depth, false>(
						nc_before, update.key, update.value, after_count_diff, update.hash_after);
			} else {
				if constexpr (depth > 1) {
					static constexpr const auto subkey = &tri::template subkey<depth>;
					UncompressedNode<depth, tri> *node_before = nc_before.node();
					for (const size_t pos : iter::range(depth)) {
						auto sub_key = subkey(update.key, pos);
						auto key_part = update.key[pos];
						AtomicUpdate<depth - 1> child_update{};
						child_update.key = sub_key;
						child_update.value = update.value;
						TaggedNodeHash child_hash = node_before->child(pos, key_part);
						child_update.hash_before = child_hash;
						if (child_hash.empty()) {
							child_update.insert_op = InsertOp::INSERT_C_NODE;
						} else {
							if (child_hash.isCompressed())
								child_update.insert_op = InsertOp::EXPAND_C_NODE;
							else
								child_update.insert_op = InsertOp::EXPAND_UC_NODE;
						}
						planUpdate(std::move(child_update), 1);
					}
				}

				if constexpr (depth > 1) {
					node_before_children_count_diff = -1;
				}

				nc_after = node_storage.template insertEntryIntoUncompressedNode<depth, true>(
						nc_before, update.key, update.value, after_count_diff, update.hash_after);
			}

			if constexpr (depth == update_depth)
				this->nodec = nc_after;
			return node_before_children_count_diff;
		}
	};
}// namespace hypertrie::internal::node_based

#endif//HYPERTRIE_NODESTORAGEUPDATE_HPP
