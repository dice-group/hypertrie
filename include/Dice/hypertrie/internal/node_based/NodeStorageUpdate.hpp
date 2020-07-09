#ifndef HYPERTRIE_NODESTORAGEUPDATE_HPP
#define HYPERTRIE_NODESTORAGEUPDATE_HPP

#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/NodeContainer.hpp"
#include "Dice/hypertrie/internal/node_based/NodeStorage.hpp"
#include "Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <Dice/hypertrie/internal/util/CountDownNTuple.hpp>

#include <tsl/hopscotch_set.h>
#include <tsl/hopscotch_map.h>

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
		static const constexpr long INC_COUNT_DIFF_AFTER = 1;
		static const constexpr long DEC_COUNT_DIFF_AFTER = -1;
		static const constexpr long INC_COUNT_DIFF_BEFORE = -1;
		static const constexpr long DEC_COUNT_DIFF_BEFORE = 1;

		template<size_t depth>
		struct AtomicUpdate;

		template<size_t depth>
		struct MultiUpdate;

		template<size_t depth>
		using LevelAtomicUpdates_t = std::unordered_set<AtomicUpdate<depth>, absl::Hash<AtomicUpdate<depth>>>;

		using PlannedAtomicUpdates = util::CountDownNTuple<LevelAtomicUpdates_t, update_depth>;

		template<size_t depth>
		using LevelMultiUpdates_t = std::unordered_set<MultiUpdate<depth>, absl::Hash<MultiUpdate<depth>>>;

		using PlannedMultiUpdates = util::CountDownNTuple<LevelMultiUpdates_t, update_depth>;

		using LevelRefChanges = tsl::hopscotch_map<TaggedNodeHash, long>;

		using RefChanges = std::array<LevelRefChanges, update_depth>;


		enum struct InsertOp : unsigned int {
			CHANGE_VALUE = 0,
			INSERT_TWO_KEY_UC_NODE,
			INSERT_C_NODE,
			EXPAND_UC_NODE,
			EXPAND_C_NODE,
			REMOVE_FROM_UC,
			INSERT_MULT_INTO_C,
			INSERT_MULT_INTO_UC,
			NEW_MULT_UC
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
			value_type old_value{};

		public:
			void calcHashAfter() {
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
					default:
						assert(false);
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

			template<typename H>
			friend H AbslHashValue(H h, const AtomicUpdate<depth> &update) {
				return H::combine(std::move(h), update.hash_before, update.hash_after);
			}
		};

		template<size_t depth>
		struct MultiUpdate {

			MultiUpdate(InsertOp insert_op, TaggedNodeHash hash_before = {}) : insert_op(insert_op), hash_before(hash_before) {
				if (insert_op == InsertOp::INSERT_MULT_INTO_UC or insert_op == InsertOp::INSERT_MULT_INTO_C)
					assert(hash_before != TaggedNodeHash{});
			}
			static_assert(depth >= 1);

			InsertOp insert_op{};
			TaggedNodeHash hash_before{};
			TaggedNodeHash hash_after{};
			std::vector<RawKey<depth>> keys{};

			void addKey(const RawKey<depth> key) {
				keys.push_back(key);
			}

		public:
			void calcHashAfter() {
				hash_after = hash_before;
				switch (insert_op) {
					case InsertOp::INSERT_MULT_INTO_C:
						[[fall_through]];
					case InsertOp::INSERT_MULT_INTO_UC:
						for (const auto &key : keys)
							hash_after.addEntry(key, true);
						break;
					case InsertOp::NEW_MULT_UC:
						assert(hash_before.empty());
						assert(keys.size() > 0);

						hash_after = TaggedNodeHash::getCompressedNodeHash(keys[0], true);
						if (keys.size() > 1)
							for (auto key_it = keys.begin() + 1; key_it != keys.end(); ++key_it)
								hash_after.addEntry(*key_it, true);
						break;
					default:
						assert(false);
				}
			}

			bool operator<(const MultiUpdate<depth> &other) const {
				return std::make_tuple(this->insert_op, this->hash_before, this->hash_after) <
					   std::make_tuple(other.insert_op, other.hash_before, other.hash_after);
			};

			bool operator==(const MultiUpdate<depth> &other) const {
				return std::make_tuple(this->insert_op, this->hash_before, this->hash_after) ==
					   std::make_tuple(other.insert_op, other.hash_before, other.hash_after);
			};

			template<typename H>
			friend H AbslHashValue(H h, const MultiUpdate<depth> &update) {
				return H::combine(std::move(h), update.hash_before, update.hash_after);
			}
		};

	public:
		NodeStorage_t<node_storage_depth> &node_storage;

		UncompressedNodeContainer<update_depth, tri> &nodec;

		value_type old_value{};

		PlannedAtomicUpdates planned_atomic_updates{};

		PlannedMultiUpdates planned_multi_updates{};

		RefChanges ref_changes{};

		template<size_t updates_depth>
		auto getRefChanges()
				-> LevelRefChanges & {
			return ref_changes[updates_depth - 1];
		}

		// extract a change from count_changes
		template<size_t updates_depth>
		void planChangeCount(const TaggedNodeHash hash, const long count_change) {
			LevelRefChanges &count_changes = getRefChanges<updates_depth>();
			count_changes[hash] += count_change;
		};


		template<size_t updates_depth>
		auto getPlannedAtomicUpdates()
				-> LevelAtomicUpdates_t<updates_depth> & {
			return std::get<updates_depth - 1>(planned_atomic_updates);
		}

		template<size_t updates_depth>
		auto getPlannedMultiUpdates()
				-> LevelMultiUpdates_t<updates_depth> & {
			return std::get<updates_depth - 1>(planned_multi_updates);
		}

		template<size_t updates_depth>
		void planUpdate(AtomicUpdate<updates_depth> planned_update, const long count_diff) {
			auto &planned_updates = getPlannedAtomicUpdates<updates_depth>();
			if (planned_update.hash_after == TaggedNodeHash{})
				planned_update.calcHashAfter();
			if (not planned_update.hash_before.empty())
				planChangeCount<updates_depth>(planned_update.hash_before, -1 * count_diff);
			planChangeCount<updates_depth>(planned_update.hash_after, count_diff);
			planned_updates.insert(std::move(planned_update));
		}

		template<size_t updates_depth>
		void planUpdate(MultiUpdate<updates_depth> planned_update, const long count_diff) {
			auto &planned_updates = getPlannedMultiUpdates<updates_depth>();
			if (planned_update.hash_after == TaggedNodeHash{})
				planned_update.calcHashAfter();
			if (not planned_update.hash_before.empty())
				planChangeCount<updates_depth>(planned_update.hash_before, -1 * count_diff);
			planChangeCount<updates_depth>(planned_update.hash_after, count_diff);
			planned_updates.insert(std::move(planned_update));
		}


		NodeStorageUpdate(NodeStorage_t<node_storage_depth> &nodeStorage, UncompressedNodeContainer<update_depth, tri> &nodec)
			: node_storage(nodeStorage), nodec{nodec} {}

		void apply_update(std::vector<RawKey<update_depth>> keys) {
			assert(keys.size() > 2);
			MultiUpdate<update_depth> update(InsertOp::INSERT_MULT_INTO_UC, nodec.thash_);
			update.keys = std::move(keys);

			planUpdate(std::move(update), 1);

			apply_update_rek<update_depth>();
		}

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
				update.old_value = this->old_value;
				update.insert_op = InsertOp::CHANGE_VALUE;
			} else {// new entry
				update.insert_op = InsertOp::EXPAND_UC_NODE;
			}
			planUpdate(std::move(update), 1);
			apply_update_rek<update_depth>();
		}

		template<size_t depth>
		void apply_update_rek() {

			LevelAtomicUpdates_t<depth> &atomic_updates = getPlannedAtomicUpdates<depth>();

			LevelMultiUpdates_t<depth> &multi_updates = getPlannedMultiUpdates<depth>();

			// nodes_before that are have ref_count 0 afterwards -> those could be reused/moved for nodes after
			tsl::hopscotch_set<TaggedNodeHash> unreferenced_nodes_before{};

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
					new_after_nodes.insert({hash, count_diff});
				}
			}

			static std::vector<std::pair<MultiUpdate<depth>, long>> moveable_multi_updates{};
			moveable_multi_updates.clear();
			moveable_multi_updates.reserve(multi_updates.size());
			// extract movables
			for (const MultiUpdate<depth> &update : multi_updates) {
				// skip if it cannot be a movable
				if (update.insert_op != InsertOp::INSERT_MULT_INTO_UC)
					continue;
				// check if it is a moveable. then save it and skip to the next iteration
				auto unref_before = unreferenced_nodes_before.find(update.hash_before);
				if (unref_before != unreferenced_nodes_before.end()) {
					if (peak_after_count_change(update.hash_after)) {
						unreferenced_nodes_before.erase(unref_before);
						auto [changes, after_count_change] = pop_after_count_change(update.hash_after);
						moveable_multi_updates.emplace_back(update, after_count_change);
					}
				}
			}

			static std::vector<std::pair<AtomicUpdate<depth>, long>> moveable_atomic_updates{};
			moveable_atomic_updates.clear();
			moveable_atomic_updates.reserve(atomic_updates.size());
			// extract movables
			for (const AtomicUpdate<depth> &update : atomic_updates) {
				// skip if it cannot be a movable
				if (update.insert_op == InsertOp::EXPAND_C_NODE or update.insert_op == InsertOp::INSERT_TWO_KEY_UC_NODE)
					continue;
				// check if it is a moveable. then save it and skip to the next iteration
				if (peak_after_count_change(update.hash_after)) {
					auto unref_before = unreferenced_nodes_before.find(update.hash_before);
					if (unref_before != unreferenced_nodes_before.end()) {
						unreferenced_nodes_before.erase(unref_before);
						auto [changes, after_count_change] = pop_after_count_change(update.hash_after);
						moveable_atomic_updates.emplace_back(update, after_count_change);
					}
				}
			}

			static std::vector<std::pair<MultiUpdate<depth>, long>> unmoveable_multi_updates{};
			unmoveable_multi_updates.clear();
			unmoveable_multi_updates.reserve(atomic_updates.size());
			// extract unmovables
			for (const MultiUpdate<depth> &update : multi_updates) {
				// check if it is a moveable. then save it and skip to the next iteration
				auto [changes, after_count_change] = pop_after_count_change(update.hash_after);
				if (changes) {
					unmoveable_multi_updates.emplace_back(update, after_count_change);
				}
			}

			static std::vector<std::pair<AtomicUpdate<depth>, long>> unmoveable_atomic_updates{};
			unmoveable_atomic_updates.clear();
			unmoveable_atomic_updates.reserve(atomic_updates.size());
			// extract unmovables
			for (const AtomicUpdate<depth> &update : atomic_updates) {
				// check if it is a moveable. then save it and skip to the next iteration
				auto [changes, after_count_change] = pop_after_count_change(update.hash_after);
				if (changes) {
					unmoveable_atomic_updates.emplace_back(update, after_count_change);
				}
			}

			assert(new_after_nodes.empty());

			std::unordered_map<TaggedNodeHash, long> node_before_children_count_diffs{};

			// do unmoveables
			for (auto &[update, count_change] : unmoveable_multi_updates) {
				long node_before_children_count_diff;
				if (update.hash_before.isCompressed())
					node_before_children_count_diff = processUpdate<depth, NodeCompression::compressed, false>(update, count_change);
				else
					node_before_children_count_diff = processUpdate<depth, NodeCompression::uncompressed, false>(update, count_change);

				if (node_before_children_count_diff != 0)
					node_before_children_count_diffs[update.hash_before] += node_before_children_count_diff;
			}

			// do unmoveables
			for (const auto &[update, count_change] : unmoveable_atomic_updates) {
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
			for (auto &[update, count_change] : moveable_multi_updates) {
				if (update.hash_before.isCompressed())
					processUpdate<depth, NodeCompression::compressed, true>(update, count_change);
				else
					processUpdate<depth, NodeCompression::uncompressed, true>(update, count_change);
			}

			// do moveables
			for (const auto &[update, count_change] : moveable_atomic_updates) {
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
							planChangeCount<depth -1 >(child_hash, -1*children_count_diff);
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
					if constexpr (compression == NodeCompression::uncompressed)
						node_before_children_count_diff = insertIntoUncompressed<depth, reuse_node_before>(update, after_count_diff);
					break;
				case InsertOp::EXPAND_C_NODE:
					if constexpr (compression == NodeCompression::compressed)
						insertTwoValueUncompressed<depth>(update, after_count_diff);
					break;
				case InsertOp::REMOVE_FROM_UC:
					assert(false);// not yet implemented
					break;
				default:
					assert(false);
			}
			return node_before_children_count_diff;
		}

		template<size_t depth, NodeCompression compression, bool reuse_node_before = false>
		long processUpdate(MultiUpdate<depth> &update, const long after_count_diff) {
			auto nc_after = node_storage.template getNode<depth>(update.hash_after);
			assert(nc_after.null());

			long node_before_children_count_diff = 0;
			switch (update.insert_op) {
				case InsertOp::INSERT_MULT_INTO_UC:
					if constexpr (compression == NodeCompression::uncompressed)
						node_before_children_count_diff = insertBulkIntoUC<depth, reuse_node_before>(update, after_count_diff);
					break;
				case InsertOp::INSERT_MULT_INTO_C:
					assert(compression == NodeCompression::uncompressed);
					if constexpr (compression == NodeCompression::compressed)
						insertBulkIntoC<depth>(update, after_count_diff);
					break;
				case InsertOp::NEW_MULT_UC:
					assert(compression == NodeCompression::uncompressed);
					if constexpr (compression == NodeCompression::uncompressed)
						newUncompressedBulk<depth>(update, after_count_diff);
					break;
				default:
					assert(false);
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
						child_update.old_value = old_value;
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
							child_update.old_value = old_value;
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

		template<size_t depth>
		void newUncompressedBulk(const MultiUpdate<depth> &update, const long after_count_diff) {
			// TODO: implement for valued
			if constexpr (not tri::is_bool_valued)
				return;
			else {
				static constexpr const auto subkey = &tri::template subkey<depth>;

				// move or copy the node from old_hash to new_hash
				auto &storage = node_storage.template getNodeStorage<depth, NodeCompression::uncompressed>();
				// make sure everything is set correctly
				assert(update.hash_before.empty());
				assert(storage.find(update.hash_after) == storage.end());

				// create node and insert it into the storage
				UncompressedNode<depth, tri> *const node = new UncompressedNode<depth, tri>{after_count_diff};
				storage.insert({update.hash_after, node});

				if constexpr (depth > 1)
					node->size_ = update.keys.size();

				// populate the new node
				for (const size_t pos : iter::range(depth)) {
					if constexpr (depth == 1) {
						for (const RawKey<depth> &key : update.keys)
							node->edges().insert(key[0]);
					} else {
						// # group the subkeys by the key part at pos

						// maps key parts to the keys to be inserted for that child
						std::unordered_map<key_part_type, std::vector<RawKey<depth - 1>>> children_inserted_keys{};

						// populate children_inserted_keys
						for (const RawKey<depth> &key : update.keys)
							children_inserted_keys[key[pos]].push_back(subkey(key, pos));

						// process the changes to the node at pos and plan the updates to the sub nodes
						for (auto &[key_part, child_inserted_keys] : children_inserted_keys) {
							assert(child_inserted_keys.size() > 0);
							auto [key_part_exists, iter] = node->find(pos, key_part);

							TaggedNodeHash hash_after;
							// plan the changes and calculate hash-after
							const TaggedNodeHash child_hash = (key_part_exists) ? iter->second : TaggedNodeHash{};
							// EXPAND_C_NODE (insert only one key)
							if (child_inserted_keys.size() == 1) {
								// plan next update
								AtomicUpdate<depth - 1> child_update{};
								child_update.key = child_inserted_keys[0];
								child_update.value = true;
								child_update.insert_op = InsertOp::INSERT_C_NODE;
								child_update.calcHashAfter();

								// safe hash_after for executing the change
								hash_after = child_update.hash_after;
								// submit the planned update
								planUpdate(std::move(child_update), 1);
							} else if (child_inserted_keys.size() == 2) {
								AtomicUpdate<depth - 1> child_update{};
								child_update.key = child_inserted_keys[0];
								child_update.value = true;
								child_update.second_key = child_inserted_keys[1];
								child_update.second_value = true;
								child_update.insert_op = InsertOp::INSERT_TWO_KEY_UC_NODE;
								child_update.calcHashAfter();

								// safe hash_after for executing the change
								hash_after = child_update.hash_after;

								planUpdate(std::move(child_update), 1);
							} else {

								MultiUpdate<depth - 1> child_update(InsertOp::NEW_MULT_UC, child_hash);
								child_update.keys = std::move(child_inserted_keys);

								child_update.calcHashAfter();

								// safe hash_after for executing the change
								hash_after = child_update.hash_after;
								planUpdate(std::move(child_update), 1);
							}
							// execute changes
							node->edges(pos)[key_part] = hash_after;
						}
					}
				}
			}
		}

		template<size_t depth>
		void insertBulkIntoC(MultiUpdate<depth> &update, const long after_count_diff) {
			auto &storage = node_storage.template getNodeStorage<depth, NodeCompression::compressed>();
			assert(storage.find(update.hash_before) == storage.end());

			CompressedNode<depth, tri> const *const node_before = storage[update.hash_before];

			update.addKey(node_before->key());
			update.insert_op = InsertOp::NEW_MULT_UC;
			update.hash_before = {};
			newUncompressedBulk<depth>(update, after_count_diff);
		}

		template<size_t depth, bool reuse_node_before = false>
		long insertBulkIntoUC(const MultiUpdate<depth> &update, const long after_count_diff) {
			// TODO: implement for valued
			if constexpr (not tri::is_bool_valued)
				return 0;
			else {
				static constexpr const auto subkey = &tri::template subkey<depth>;
				const long node_before_children_count_diff =
						(not reuse_node_before and depth > 1) ? INC_COUNT_DIFF_BEFORE : 0;

				// move or copy the node from old_hash to new_hash
				auto &storage = node_storage.template getNodeStorage<depth, NodeCompression::uncompressed>();
				auto node_it = storage.find(update.hash_before);
				assert(node_it != storage.end());
				UncompressedNode<depth, tri> *node = node_it->second;
				if constexpr (reuse_node_before) {// node before ref_count is zero -> maybe reused
					storage.erase(node_it);
				} else {
					node = new UncompressedNode<depth, tri>{*node};
				}
				assert(storage.find(update.hash_after) == storage.end());
				storage[update.hash_after] = node;

				// update the node count
				node->ref_count() += after_count_diff;
				if constexpr (depth > 1)
					node->size_ += update.keys.size();

				// update the node (new_hash)
				for (const size_t pos : iter::range(depth)) {
					// # group the subkeys by the key part at pos
					if constexpr (depth == 1) {
						for (const RawKey<depth> &key : update.keys)
							node->edges().insert(key[0]);
					} else {
						// maps key parts to the keys to be inserted for that child
						std::unordered_map<key_part_type, std::vector<RawKey<depth - 1>>> children_inserted_keys{};

						// populate children_inserted_keys
						for (const RawKey<depth> &key : update.keys)
							children_inserted_keys[key[pos]].push_back(subkey(key, pos));

						// process the changes to the node at pos and plan the updates to the sub nodes
						for (auto &[key_part, child_inserted_keys] : children_inserted_keys) {
							assert(child_inserted_keys.size() > 0);
							// TODO: handle what happens when we hit level 1
							auto [key_part_exists, iter] = node->find(pos, key_part);

							TaggedNodeHash child_hash_after;
							// plan the changes and calculate hash-after
							const TaggedNodeHash child_hash_before = (key_part_exists) ? iter->second : TaggedNodeHash{};
							// EXPAND_C_NODE (insert only one key)
							if (child_inserted_keys.size() == 1) {
								// plan next update
								AtomicUpdate<depth - 1> child_update{};
								child_update.hash_before = child_hash_before;
								child_update.key = child_inserted_keys[0];
							child_update.value = true;
							if (key_part_exists)
								child_update.insert_op = InsertOp::EXPAND_C_NODE;
							else
								child_update.insert_op = InsertOp::INSERT_C_NODE;
							child_update.calcHashAfter();

							// safe child_hash_after for executing the change
							child_hash_after = child_update.hash_after;
							// submit the planned update
							planUpdate(std::move(child_update), 1);
						} else {// INSERT_MULT_INTO_UC (insert multiple keys)
							if (not key_part_exists and child_inserted_keys.size() == 2) {
								AtomicUpdate<depth - 1> child_update{};
								child_update.key = child_inserted_keys[0];
								child_update.value = true;
								child_update.second_key = child_inserted_keys[1];
								child_update.second_value = true;
								child_update.insert_op = InsertOp::INSERT_TWO_KEY_UC_NODE;
								child_update.calcHashAfter();

								// safe child_hash_after for executing the change
								child_hash_after = child_update.hash_after;

								planUpdate(std::move(child_update), 1);
							} else {

								MultiUpdate<depth - 1> child_update(
										[&]() -> InsertOp {
											if (key_part_exists)
												if (child_hash_before.isCompressed())
													return InsertOp::INSERT_MULT_INTO_C;
												else
													return InsertOp::INSERT_MULT_INTO_UC;
											else
												return InsertOp::NEW_MULT_UC;
										}(),
										child_hash_before);
								if (key_part_exists)
									child_update.hash_before = child_hash_before;
								child_update.keys = std::move(child_inserted_keys);

								child_update.calcHashAfter();

								// safe child_hash_after for executing the change
								child_hash_after = child_update.hash_after;
								planUpdate(std::move(child_update), 1);
							}
						}
						// execute changes
						if (key_part_exists)
							tri::template deref<key_part_type, TaggedNodeHash>(iter) = child_hash_after;
						else
							node->edges(pos)[key_part] = child_hash_after;
						}
					}
				}
				return node_before_children_count_diff;
			}
		}


	};
}// namespace hypertrie::internal::node_based

#endif//HYPERTRIE_NODESTORAGEUPDATE_HPP
