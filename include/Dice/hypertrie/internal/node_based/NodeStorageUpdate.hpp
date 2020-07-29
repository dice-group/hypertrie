#ifndef HYPERTRIE_NODESTORAGEUPDATE_HPP
#define HYPERTRIE_NODESTORAGEUPDATE_HPP

#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/NodeContainer.hpp"
#include "Dice/hypertrie/internal/node_based/NodeStorage.hpp"
#include "Dice/hypertrie/internal/node_based/TensorHash.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <Dice/hypertrie/internal/util/CountDownNTuple.hpp>

#include <robin_hood.h>
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
		class MultiUpdate;

		template<size_t depth>
		using LevelMultiUpdates_t = std::unordered_set<MultiUpdate<depth>, absl::Hash<MultiUpdate<depth>>>;

		using PlannedMultiUpdates = util::CountDownNTuple<LevelMultiUpdates_t, update_depth>;


		template <size_t depth>
		struct CountDiffAndNodePtr{ long count_diff; NodePtr<depth, tri> node_ptr; };

		template <size_t depth>
		using LevelRefChanges = tsl::sparse_map<TensorHash, CountDiffAndNodePtr<depth>>;

		using RefChanges = util::CountDownNTuple<LevelRefChanges, update_depth>;



		template<size_t depth>
		class BoolEntry {

			RawKey<depth> key_;

		public:
			BoolEntry() : key_{}{}
			BoolEntry(RawKey<depth> key, [[maybe_unused]] value_type value = value_type(1)) : key_(key) {}


			constexpr value_type value() const noexcept { return value_type(1); };
		};

		template<size_t depth>
		class NonBoolEntry : public BoolEntry<depth> {
			RawKey<depth> key_;

			value_type value_;

		public:
			NonBoolEntry() : key_{},  value_{}{}
			NonBoolEntry(const RawKey<depth> &key, [[maybe_unused]] value_type value = value_type(1)) : key_{key}, value_(value) {}

			key_part_type & operator[](size_t pos) {
				return key_[pos];
			}

			const key_part_type & operator[](size_t pos) const {
				return key_[pos];
			}

			friend class NodeStorageUpdate;
		};

		template<size_t depth>
		using Entry = std::conditional_t <(tri::is_bool_valued), RawKey<depth>, NonBoolEntry<depth>>;

		template<size_t depth>
		static auto make_Entry(RawKey<depth> key, const value_type value = value_type(1)) -> Entry<depth> {
			if constexpr (tri::is_bool_valued)
				return {key};
			else
				return {key, value};
		}

		template<size_t depth>
		static RawKey<depth> &key(NonBoolEntry<depth> &entry){
			return entry.key_;
		}

		template<size_t depth>
		static const RawKey<depth> &key(const NonBoolEntry<depth> &entry){
			return entry.key_;
		}

		template<size_t depth>
		static value_type &value(NonBoolEntry<depth> &entry){
			return entry.value_;
		}

		template<size_t depth>
		static const value_type &value(const NonBoolEntry<depth> &entry){
			return entry.value_;
		}

		template<size_t depth>
		static value_type value([[maybe_unused]] const RawKey<depth> &entry){
			return true;
		}

		template<size_t depth>
		static RawKey<depth> &key([[maybe_unused]] RawKey<depth> &entry){
			return entry;
		}

		template<size_t depth>
		static const RawKey<depth> &key([[maybe_unused]] const RawKey<depth> &entry){
			return entry;
		}



		enum struct InsertOp : unsigned int {
			NONE = 0,
			CHANGE_VALUE,
			INSERT_C_NODE,
			REMOVE_FROM_UC,
			INSERT_MULT_INTO_C,
			INSERT_MULT_INTO_UC,
			NEW_MULT_UC
		};

		template<size_t depth>
		class MultiUpdate {
			static_assert(depth >= 1);


			InsertOp insert_op_{};
			TensorHash hash_before_{};
			mutable TensorHash hash_after_{};
			mutable std::vector<Entry<depth>> entries_{};

		public:

			InsertOp &insertOp()  noexcept { return this->insert_op_;}

			const InsertOp &insertOp() const noexcept { return this->insert_op_;}

			TensorHash &hashBefore()  noexcept { return this->hash_before_;}

			const TensorHash &hashBefore() const noexcept {
				if (hash_after_.empty()) calcHashAfter();
				return this->hash_before_;
			}

			TensorHash &hashAfter() noexcept {
				if (hash_after_.empty()) calcHashAfter();
				return this->hash_after_;
			}

			const TensorHash &hashAfter() const noexcept { return this->hash_after_;}

			std::vector<Entry<depth>> &entries() noexcept { return this->entries_;}

			const std::vector<Entry<depth>> &entries() const noexcept { return this->entries_;}



			void addEntry(Entry<depth> entry) noexcept {
				entries_.push_back(entry);
			}

			void addEntry(RawKey<depth> key, const value_type  value) noexcept {
				entries_.push_back(make_Entry(key, value));
			}

			void addKey(RawKey<depth> key) noexcept {
				entries_.push_back({key});
			}

			value_type &oldValue() noexcept {
				if constexpr (not tri::is_bool_valued){
					assert(insert_op_ == InsertOp::CHANGE_VALUE);
					entries_.resize(2);
					return value(entries_[1]);
				} else
					assert(false);
			}

			const value_type &oldValue() const noexcept {
				if constexpr (not tri::is_bool_valued){
					assert(insert_op_ == InsertOp::CHANGE_VALUE);
					entries_.resize(2);
					return value(entries_[1]);
				} else
					assert(false);
			}

			RawKey<depth> &firstKey() noexcept { return key(entries_[0]); }

			const RawKey<depth> &firstKey() const noexcept { return key(entries_[0]); }

			value_type &firstValue() noexcept  { return value(entries_[0]); }

			value_type firstValue() const noexcept  { return value(entries_[0]); }

		private:
			void calcHashAfter() const noexcept {
				hash_after_ = hash_before_;
				switch (insert_op_) {
					case InsertOp::CHANGE_VALUE:
						assert(not hash_before_.empty());
						assert(entries_.size() == 2);
						this->hash_after_.changeValue(firstKey(), oldValue(), firstValue());
						break;
					case InsertOp::INSERT_C_NODE:{
						assert(hash_before_.empty());
						hash_after_ = TensorHash::getCompressedNodeHash(firstKey(), firstValue());
						break;
					}
					case InsertOp::INSERT_MULT_INTO_C:
						[[fallthrough]];
					case InsertOp::INSERT_MULT_INTO_UC:
						assert(not hash_before_.empty());
						for (const auto &entry : entries_)
							hash_after_.addEntry(key(entry), value(entry));
						break;
					case InsertOp::NEW_MULT_UC:
						assert(hash_before_.empty());
						assert(entries_.size() > 1);

						hash_after_ = TensorHash::getCompressedNodeHash(firstKey(), firstValue());
						for (auto entry_it = std::next(entries_.begin()); entry_it != entries_.end(); ++entry_it)
							hash_after_.addEntry(key(*entry_it), value(*entry_it));
						break;
					default:
						assert(false);
				}
			}

		public:

			bool operator<(const MultiUpdate<depth> &other) const noexcept {
				return std::make_tuple(this->insert_op_, this->hash_before_, this->hashAfter()) <
					   std::make_tuple(other.insert_op_, other.hash_before_, other.hashAfter());
			};

			bool operator==(const MultiUpdate<depth> &other) const noexcept {
				return std::make_tuple(this->insert_op_, this->hash_before_, this->hashAfter()) ==
					   std::make_tuple(other.insert_op_, other.hash_before_, other.hashAfter());
			};

			template<typename H>
			friend H AbslHashValue(H h, const MultiUpdate<depth> &update) {
				return H::combine(std::move(h), update.hash_before_, update.hashAfter());
			}
		};

	public:
		NodeStorage_t<node_storage_depth> &node_storage;

		UncompressedNodeContainer<update_depth, tri> &nodec;

		PlannedMultiUpdates planned_multi_updates{};

		RefChanges ref_changes{};

		template<size_t updates_depth>
		auto getRefChanges()
				-> LevelRefChanges<updates_depth> & {
			return std::get<updates_depth - 1>(ref_changes);
		}

		// extract a change from count_changes
		template<size_t updates_depth>
		void planChangeCount(const TensorHash hash, const long count_change) {
			LevelRefChanges<updates_depth> &count_changes = getRefChanges<updates_depth>();
			count_changes[hash].count_diff += count_change;
		};


		template<size_t updates_depth>
		auto getPlannedMultiUpdates()
				-> LevelMultiUpdates_t<updates_depth> & {
			return std::get<updates_depth - 1>(planned_multi_updates);
		}

		template<size_t updates_depth>
		void planUpdate(MultiUpdate<updates_depth> planned_update, const long count_diff) {
			auto &planned_updates = getPlannedMultiUpdates<updates_depth>();
			if (not planned_update.hashBefore().empty())
				planChangeCount<updates_depth>(planned_update.hashBefore(), -1 * count_diff);
			planChangeCount<updates_depth>(planned_update.hashAfter(), count_diff);
			planned_updates.insert(std::move(planned_update));
		}


		NodeStorageUpdate(NodeStorage_t<node_storage_depth> &nodeStorage, UncompressedNodeContainer<update_depth, tri> &nodec)
			: node_storage(nodeStorage), nodec{nodec} {}

		auto apply_update(std::vector<RawKey<update_depth>> keys) {
			assert(keys.size() > 2);
			MultiUpdate<update_depth> update{};
			update.insertOp() = InsertOp::INSERT_MULT_INTO_UC;
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

			MultiUpdate<update_depth> update{};
			update.hashBefore() = nodec;
			update.addEntry(key, value);
			if (value_deleted) {
				update.insertOp() = InsertOp::REMOVE_FROM_UC;
				throw std::logic_error{"deleting values from hypertrie is not yet implemented. "};
			} else if (value_changes) {
				update.insertOp() = InsertOp::CHANGE_VALUE;
				update.oldValue() = old_value;
			} else {// new entry
				update.insertOp() = InsertOp::INSERT_MULT_INTO_UC;
			}
			planUpdate(std::move(update), INC_COUNT_DIFF_AFTER);
			apply_update_rek<update_depth>();
		}

		template<size_t depth>
		void apply_update_rek() {

			LevelMultiUpdates_t<depth> &multi_updates = getPlannedMultiUpdates<depth>();

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

			static std::vector<std::pair<MultiUpdate<depth>, size_t>> moveable_multi_updates{};
			moveable_multi_updates.clear();
			moveable_multi_updates.reserve(multi_updates.size());
			// extract movables
			for (const MultiUpdate<depth> &update : multi_updates) {
				// skip if it cannot be a movable
				if (update.insertOp() == InsertOp::NEW_MULT_UC
					or update.insertOp() == InsertOp::INSERT_C_NODE
					or update.insertOp() ==  InsertOp::INSERT_MULT_INTO_C)
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

			static std::vector<std::pair<MultiUpdate<depth>, size_t>> unmoveable_multi_updates{};
			unmoveable_multi_updates.clear();
			unmoveable_multi_updates.reserve(multi_updates.size());
			// extract unmovables
			for (const MultiUpdate<depth> &update : multi_updates) {
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
				auto node = node_storage.template getUncompressedNode<depth>(hash).node();
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
		}

		template<size_t depth, bool reuse_node_before = false>
		long processUpdate(MultiUpdate<depth> &update, const size_t after_count_diff) {
			long node_before_children_count_diff = 0;

			switch (update.insertOp()) {
				case InsertOp::CHANGE_VALUE:
					if constexpr (not tri::is_bool_valued) {
						if (update.hashBefore().isCompressed())
							changeValue<depth, NodeCompression::compressed, reuse_node_before>(update, after_count_diff);
						else
							node_before_children_count_diff = changeValue<depth, NodeCompression::uncompressed, reuse_node_before>(update, after_count_diff);
					}
					break;
				case InsertOp::INSERT_C_NODE:
					if constexpr(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued))
						insertCompressedNode<depth>(update, after_count_diff);
					break;
				case InsertOp::INSERT_MULT_INTO_UC:
					node_before_children_count_diff = insertBulkIntoUC<depth, reuse_node_before>(update, after_count_diff);
					break;
				case InsertOp::INSERT_MULT_INTO_C:
					if constexpr (not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued))
						insertBulkIntoC<depth>(update, after_count_diff);
					break;
				case InsertOp::NEW_MULT_UC:
					newUncompressedBulk<depth>(update, after_count_diff);
					break;
				case InsertOp::REMOVE_FROM_UC:
					assert(false);// not yet implemented
					break;
				default:
					assert(false);
			}
			return node_before_children_count_diff;
		}

		template<size_t depth>
		void insertCompressedNode(const MultiUpdate<depth> &update, const size_t after_count_diff) {

			node_storage.template newCompressedNode<depth>(
					update.firstKey(), update.firstValue(), after_count_diff, update.hashAfter());
		}

		template<size_t depth, NodeCompression compression, bool reuse_node_before = false>
		long changeValue(const MultiUpdate<depth> &update, const size_t after_count_diff) {
			long node_before_children_count_diff = 0;
			SpecificNodeContainer<depth, compression, tri> nc_before = node_storage.template getNode<depth, compression>(update.hashBefore());
			assert(not nc_before.null());
			Node<depth, compression, tri> *node_before = nc_before.node();

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

							MultiUpdate<depth - 1> child_update{};
							child_update.insertOp() = InsertOp::CHANGE_VALUE;
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

							MultiUpdate<depth - 1> child_update{};
							child_update.insertOp() = InsertOp::CHANGE_VALUE;
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

			if constexpr (depth == update_depth and compression == NodeCompression::uncompressed)
				this->nodec = nc_after;

			return node_before_children_count_diff;
		}

		template<size_t depth>
		void newUncompressedBulk(const MultiUpdate<depth> &update, const size_t after_count_diff) {
			// TODO: implement for valued
			static constexpr const auto subkey = &tri::template subkey<depth>;

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
							node->edges().insert(key(entry)[0]);
						else
							node->edges().emplace(key(entry)[0], value(entry));
					}
				} else {
					// # group the subkeys by the key part at pos

					// maps key parts to the keys to be inserted for that child
					robin_hood::unordered_map<key_part_type, std::vector<Entry<depth - 1>>> children_inserted_keys{};

					// populate children_inserted_keys
					for (const Entry<depth> &entry : update.entries())
						children_inserted_keys[key(entry)[pos]]
								.push_back(make_Entry(subkey(key(entry), pos), value(entry)));

					// process the changes to the node at pos and plan the updates to the sub nodes
					for (auto &[key_part, child_inserted_entries] : children_inserted_keys) {
						assert(child_inserted_entries.size() > 0);

						// plan the new subnodes and insert references
						MultiUpdate<depth - 1> child_update{};
						if (child_inserted_entries.size() == 1){
							if constexpr (not (depth == 2 and tri::is_bool_valued and tri::is_lsb_unused)) {
								child_update.insertOp() = InsertOp::INSERT_C_NODE;
							} else {
								node->edges(pos)[key_part] = KeyPartUCNodeHashVariant<tri>{child_inserted_entries[0][0]};
								continue;
							}
						} else
							child_update.insertOp() = InsertOp::NEW_MULT_UC;
						child_update.entries() = std::move(child_inserted_entries);

						// insert reference to subnode
						node->edges(pos)[key_part] = child_update.hashAfter();
						// submit subnode plan
						planUpdate(std::move(child_update), INC_COUNT_DIFF_AFTER);
					}
				}
			}
		}

		template<size_t depth>
		void insertBulkIntoC(MultiUpdate<depth> &update, const long after_count_diff) {
			auto &storage = node_storage.template getNodeStorage<depth, NodeCompression::compressed>();
			assert(storage.find(update.hashBefore()) != storage.end());
			assert(storage.find(update.hashAfter()) == storage.end());

			CompressedNode<depth, tri> const *const node_before = storage[update.hashBefore()];

			update.insertOp() = InsertOp::NEW_MULT_UC;
			update.addEntry(node_before->key(), node_before->value());
			update.hashBefore() = {};
			newUncompressedBulk<depth>(update, after_count_diff);
		}

		template<size_t depth, bool reuse_node_before = false>
		long insertBulkIntoUC(const MultiUpdate<depth> &update, const long after_count_diff) {
				static constexpr const auto subkey = &tri::template subkey<depth>;
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
								node->edges().insert(key(entry)[0]);
							else
								node->edges().emplace(key(entry)[0], value(entry));
						}
					} else {
						// # group the subkeys by the key part at pos

						// maps key parts to the keys to be inserted for that child
						std::unordered_map<key_part_type, std::vector<Entry<depth - 1>>> children_inserted_keys{};

						// populate children_inserted_keys
						for (const Entry<depth> &entry : update.entries())
							children_inserted_keys[key(entry)[pos]]
									.push_back(make_Entry(subkey(key(entry), pos), value(entry)));

						// process the changes to the node at pos and plan the updates to the sub nodes
						for (auto &[key_part, child_inserted_entries] : children_inserted_keys) {
							assert(child_inserted_entries.size() > 0);
							auto [key_part_exists, iter] = node->find(pos, key_part);

							MultiUpdate<depth - 1> child_update{};
							if constexpr (not (depth == 2 and tri::is_bool_valued and tri::is_lsb_unused)) {
								if (key_part_exists) {
									child_update.hashBefore() = iter->second;
									if (child_update.hashBefore().isCompressed())
										child_update.insertOp() = InsertOp::INSERT_MULT_INTO_C;
									else
										child_update.insertOp() = InsertOp::INSERT_MULT_INTO_UC;
								} else {
									if (child_inserted_entries.size() == 1)
										child_update.insertOp() = InsertOp::INSERT_C_NODE;
									else
										child_update.insertOp() = InsertOp::NEW_MULT_UC;
								}
							} else {
								if (key_part_exists) {
									if (iter->second.isCompressed()) {
										child_inserted_entries.push_back({iter->second.getKeyPart()});
										child_update.insertOp() = InsertOp::NEW_MULT_UC;
									} else {
										child_update.hashBefore() = iter->second.getTaggedNodeHash();
										child_update.insertOp() = InsertOp::INSERT_MULT_INTO_UC;
									}
								} else {
									if (child_inserted_entries.size() == 1) {
										node->edges(pos)[key_part] = KeyPartUCNodeHashVariant<tri>{child_inserted_entries[0][0]};
										continue;
									} else {
										child_update.insertOp() = InsertOp::NEW_MULT_UC;
									}
								}
							}


							child_update.entries() = std::move(child_inserted_entries);

							// execute changes
							if (key_part_exists)
								if constexpr (not (depth == 2 and tri::is_bool_valued and tri::is_lsb_unused))
									tri::template deref<key_part_type, TensorHash>(iter) = child_update.hashAfter();
								else
									tri::template deref<key_part_type, KeyPartUCNodeHashVariant<tri>>(iter) = child_update.hashAfter();
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
}// namespace hypertrie::internal::node_based

#endif//HYPERTRIE_NODESTORAGEUPDATE_HPP
