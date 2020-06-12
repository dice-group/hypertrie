#ifndef HYPERTRIE_NODESTORAGEUPDATE_HPP
#define HYPERTRIE_NODESTORAGEUPDATE_HPP

#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/NodeContainer.hpp"
#include "Dice/hypertrie/internal/node_based/NodeStorage.hpp"
#include "Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <Dice/hypertrie/internal/util/CountDownNTuple.hpp>

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
		using LevelUpdates_t = std::vector<AtomicUpdate<depth>>;

		using PlannedUpdates = util::CountDownNTuple<LevelUpdates_t, update_depth>;


		enum struct InsertOp : unsigned int {
			CHANGE_VALUE = 0,
			INSERT_TWO_KEY_UC_NODE,
			INSERT_C_NODE,
			EXPAND_UC_NODE,
			EXPAND_C_NODE
		};

		template<size_t depth>
		struct AtomicUpdate {
			static_assert(depth >= 1);

			InsertOp insert_op{};
			TaggedNodeHash hash_before{};
			RawKey<depth> key{};
			value_type value{};
			RawKey<depth> second_key{};
			value_type second_value{};
			TaggedNodeHash hash_after{};

		public:
			void calcHashAfter(const value_type &old_value) {
				hash_after = hash_before;
				switch (insert_op) {
					case InsertOp::CHANGE_VALUE:
						hash_after.changeValue(key, old_value, value);
						break;
					case InsertOp::INSERT_TWO_KEY_UC_NODE:
						hash_after = TaggedNodeHash::getTwoEntriesNodeHash(key, value, second_key, second_value);
						break;
					case InsertOp::INSERT_C_NODE:
						hash_after = TaggedNodeHash::getCompressedNodeHash(key, value);
						break;
					case InsertOp::EXPAND_UC_NODE:
						assert(hash_before.isUncompressed());
						hash_after.addEntry(key, value);
						break;
					case InsertOp::EXPAND_C_NODE:
						assert(hash_before.isCompressed());
						hash_after.addEntry(key, value);
						break;
				}
			}

			auto operator<=>(const AtomicUpdate<depth> &other) const = default;
		};


		template<size_t depth>
		struct SingleEntryChange {
			UncompressedNodeContainer<depth, tri> nodec;
			RawKey<depth> key;
			value_type value;
			bool primary_change;
		};

		template<size_t depth>
		struct NewDoubleEntryNode {
			RawKey<depth> key;
			value_type value;
			RawKey<depth> second_key;
			value_type second_value;
		};


	public:
		NodeStorage_t<node_storage_depth> &node_storage;

		UncompressedNodeContainer<update_depth, tri> &nodec;

		value_type new_value;

		value_type old_value{};

		bool only_value_changes = false;

		bool nothing_changes = false;

		PlannedUpdates planned_updates{};

		template<size_t depth>
		auto getPlannedUpdates()
				-> std::vector<AtomicUpdate<depth>> & {
			return std::get<depth - 1>(planned_updates);
		}

		bool
		hash_changes() {
			return not this->nothing_changes;
		}


		NodeStorageUpdate(NodeStorage_t<node_storage_depth> &nodeStorage, UncompressedNodeContainer<update_depth, tri> &nodec)
			: node_storage(nodeStorage), nodec{nodec} {}


		void plan(const RawKey<update_depth> &key, value_type value) {
			new_value = value;
			if (new_value != value_type{}) {
				SingleEntryChange<update_depth> change{nodec, key, value, true};
				plan_rek<update_depth>({change}, {});
				if (hash_changes()) {
					AtomicUpdate<update_depth> update{};
					update.hash_before = nodec;
					update.key = key;
					update.value = value;
					if (only_value_changes)
						update.insert_op = InsertOp ::CHANGE_VALUE;
					else// new entry (key + value)
						update.insert_op = InsertOp ::EXPAND_UC_NODE;
					getPlannedUpdates<update_depth>().push_back(std::move(update));
				}
			} else {
				throw std::logic_error{"not yet implemented!"};
			}
		}

		/**
		 *
		 * @tparam depth depth of the nodes currently processed
		 * @param node_cs vector of (NodeContainer, RawKey<depth>, value_type) to be inserted.
		 * @param expand_uc vector of two entries (RawKey<depth>, value_type, RawKey<depth> (2), value_type (2)) that should form a node.
		 * @param only_value_changes this is set to true if the key already exists with another value
		 * @param old_value the old value
		 * @param new_value the new value or 0, 0.0, false if it was not yet found or wasn't set before
		 * @return
		 */
		template<size_t depth>
		auto plan_rek(
				std::vector<SingleEntryChange<depth>> node_cs,
				std::vector<NewDoubleEntryNode<depth>> expand_uc) {
			static_assert(depth >= 1);
			if constexpr (depth == 1) {

				for (const SingleEntryChange<depth> &change : node_cs) {
					if (change.primary_change) {// it is not just an expanded node
						this->old_value = change.nodec.node()->child(0, change.key[0]);
						if (this->old_value == change.value) {
							this->nothing_changes = true;
						} else if (this->old_value != value_type{}) {
							this->only_value_changes = true;
						}
						return;
					}
				}
				this->nothing_changes = false;
				return;

			} else {
				// we only need to look at what to do with subtries for a depth > 1.
				static constexpr const auto subkey = &tri::template subkey<depth>;


				std::vector<AtomicUpdate<depth - 1>> &planned_updates = getPlannedUpdates<depth - 1>();

				std::vector<SingleEntryChange<depth - 1>> next_node_cs{};

				std::vector<NewDoubleEntryNode<depth - 1>> next_expand_uc{};

				for (SingleEntryChange<depth> &change : node_cs) {
					for (size_t pos : iter::range(depth)) {
						RawKey<depth - 1> key = subkey(change.key, pos);
						AtomicUpdate<depth - 1> planned_update{};
						planned_update.key = key;
						planned_update.value = change.value;
						TaggedNodeHash child_hash = change.nodec.getChildHashOrValue(pos, change.key[pos]);
						if (not child_hash.empty()) {
							planned_update.hash_before = child_hash;
							if (child_hash.isCompressed()) {
								CompressedNodeContainer<depth - 1, tri> childc = node_storage.template getCompressedNode<depth - 1>(child_hash);
								auto *child = childc.node();
								if (child->key() == key) {
									if constexpr (not tri::is_bool_valued) {
										if (child->value() != change.value) {// child->value() != value
											if (change.primary_change) {
												this->old_value = change.value;
												this->only_value_changes = true;
											}
											planned_update.insert_op = InsertOp::CHANGE_VALUE;
										} else {
											if (change.primary_change) {
												this->old_value = change.value;
												this->nothing_changes = true;
												return;// nothing left to be done. The value is already set.
											}
										}
									} else {// tri::is_bool_valued
										if (change.primary_change) {
											this->old_value = true;
											this->nothing_changes = true;
											return;// nothing left to be done. The value is already set.
										}
									}
								} else {// child->key() != key
									planned_update.second_key = child->key();
									if constexpr (not tri::is_bool_valued)
										planned_update.second_value = child->value();
									planned_update.insert_op = InsertOp::EXPAND_C_NODE;

									next_expand_uc.push_back(
											{key, change.value,
											 planned_update.second_key, planned_update.second_value});
								}
							} else {// child_hash.isUncompressed()
								auto child_nc = node_storage.template getUncompressedNode<depth - 1>(child_hash);
								assert(not child_nc.empty() and not child_nc.null());// TODO: that fails sometimes.
								next_node_cs.push_back(
										{child_nc,
										 key, change.value, change.primary_change});
								planned_update.insert_op = InsertOp::EXPAND_UC_NODE;
							}
						} else {// child_hash.empty()
							// no child exists for that key_part at that position
							planned_update.insert_op = InsertOp::INSERT_C_NODE;
						}
						planned_updates.push_back(std::move(planned_update));
					}
				}
				// creating uncompressed nodes with two keys (expanded compressed nodes)
				for (const NewDoubleEntryNode<depth> change : expand_uc) {
					for (size_t pos : iter::range(depth)) {
						const RawKey<depth - 1> key = subkey(change.key, pos);
						const RawKey<depth - 1> second_key = subkey(change.second_key, pos);
						if (change.key[pos] == change.second_key[pos]) {
							AtomicUpdate<depth - 1> planned_update{};
							planned_update.key = key;
							planned_update.value = change.value;
							planned_update.second_key = second_key;
							planned_update.second_value = change.second_value;
							planned_update.insert_op = InsertOp::INSERT_TWO_KEY_UC_NODE;
							planned_updates.push_back(std::move(planned_update));
							next_expand_uc.push_back({key, change.value,
													  second_key, change.second_value});
						} else {
							AtomicUpdate<depth - 1> planned_update{};
							planned_update.key = key;
							planned_update.value = change.value;
							planned_update.insert_op = InsertOp::INSERT_C_NODE;
							planned_updates.push_back(std::move(planned_update));

							AtomicUpdate<depth - 1> second_planned_update{};
							second_planned_update.key = second_key;
							second_planned_update.value = change.second_value;
							second_planned_update.insert_op = InsertOp::INSERT_C_NODE;
							planned_updates.push_back(std::move(second_planned_update));
						}
					}
				}
				plan_rek<depth - 1>(next_node_cs, next_expand_uc);
			}
		}

		void apply() {
			if (this->hash_changes())
				apply_rek<update_depth>();
			//			auto &updates = getPlannedUpdates<update_depth>();
			//			TaggedNodeHash new_hash = updates[0].hash_after;
			// TODO: update primary nodes list.
			//			return node_storage.template getUncompressedNode<update_depth>(new_hash);
		};

		template<size_t depth>
		void apply_rek() {

			std::vector<AtomicUpdate<depth>> &updates = getPlannedUpdates<depth>();
			// ascending
			std::set<TaggedNodeHash> hashes_before{};
			boost::container::flat_map<TaggedNodeHash, NodeContainer<depth, tri>> nodes_before{};
			boost::container::flat_map<TaggedNodeHash, NodeContainer<depth, tri>> nodes_after{};
			std::map<TaggedNodeHash, long> count_changes{};
			// node before is empty
			std::vector<AtomicUpdate<depth>> trivial_updates{};
			// node before is compressed
			std::vector<AtomicUpdate<depth>> simple_updates{};
			// node before is uncompressed
			std::vector<AtomicUpdate<depth>> complex_updates{};

			for (AtomicUpdate<depth> &update : updates)
				update.calcHashAfter(this->old_value);

			for (const AtomicUpdate<depth> &update : updates) {
				if (not update.hash_before.empty()) {
					count_changes[update.hash_before]--;
					nodes_before[update.hash_before];
				}
				assert(not update.hash_after.empty());
				count_changes[update.hash_after]++;

				hashes_before.insert(update.hash_before);


				if (update.hash_before.empty())
					trivial_updates.push_back(update);
				else if (update.hash_before.isCompressed()) {
					nodes_after[update.hash_after];
					simple_updates.push_back(update);
				} else {
					nodes_after[update.hash_after];
					complex_updates.push_back(update);
				}
			}

			auto pop_count_change = [&](TaggedNodeHash hash) -> std::tuple<bool, long> {
				if (auto changed = count_changes.find(hash); changed != count_changes.end()) {
					auto diff = changed->second;
					count_changes.erase(changed);
					return {true, diff};
				} else
					return {false, 0};
			};

			std::set<TaggedNodeHash> finally_unreferenced_hashes_before{};

			for (auto &entry : nodes_before) {
				TaggedNodeHash &hash_before = entry.first;
				auto &nodec_before = entry.second;
				if (nodec_before.null())
					nodec_before = node_storage.template getNode<depth>(hash_before);
				auto [success, count_change] = pop_count_change(hash_before);
				assert(success);
				size_t *ref_count = [&]() {
					if (nodec_before.isCompressed())
						return &nodec_before.compressed_node()->ref_count();
					else
						return &nodec_before.uncompressed_node()->ref_count();
				}();
				(*ref_count) += count_change;
				if ((*ref_count) == 0)
					finally_unreferenced_hashes_before.insert(hash_before);
			}

			std::set<TaggedNodeHash> nonexisting_hashes_after{};

			for (auto &[hash, nodec_after] : nodes_after) {
				nodec_after = node_storage.template getNode<depth>(hash);
				if (nodec_after.empty())
					nonexisting_hashes_after.insert(hash);
			}

			std::vector<AtomicUpdate<depth>> movable_updates{};

			std::set<TaggedNodeHash> movable_hashes_after{};

			// sort out updates that shall be moved
			for (auto it = complex_updates.begin(); it != complex_updates.end();) {
				if (nonexisting_hashes_after.count(it->hash_after)) {
					if (finally_unreferenced_hashes_before.count(it->hash_before)) {
						nonexisting_hashes_after.erase(it->hash_after);
						movable_updates.push_back(*it);
						finally_unreferenced_hashes_before.erase(it->hash_before);
						movable_hashes_after.insert(it->hash_after);
						it = complex_updates.erase(it);
						continue;
					}
				}
				it++;
			}

			// TODO extract a function for this and the one above
			for (auto it = simple_updates.begin(); it != simple_updates.end();) {
				if (it->insert_op == InsertOp::CHANGE_VALUE and
					nonexisting_hashes_after.count(it->hash_after)) {
					if (finally_unreferenced_hashes_before.count(it->hash_before)) {
						nonexisting_hashes_after.erase(it->hash_after);
						movable_updates.push_back(*it);
						finally_unreferenced_hashes_before.erase(it->hash_before);
						movable_hashes_after.insert(it->hash_after);
						it = simple_updates.erase(it);
						continue;
					}
				}
				it++;
			}

			for (AtomicUpdate<depth> &update : trivial_updates) {
				if (not movable_hashes_after.count(update.hash_after)) {
					const auto [update_node_after, node_after_count_diff] = pop_count_change(update.hash_after);
					if (update_node_after) {
						switch (update.insert_op) {
							case InsertOp::INSERT_C_NODE:
								insertCompressedNode<depth>(update, node_after_count_diff);
								break;
							case InsertOp::INSERT_TWO_KEY_UC_NODE:
								insertTwoValueUncompressed<depth>(update, node_after_count_diff);
								break;
							default:
								assert(false);
								break;
						}
					}
				}
			}


			for (AtomicUpdate<depth> &update : simple_updates) {
				if (not movable_hashes_after.count(update.hash_after)) {
					const auto [update_node_after, node_after_count_diff] = pop_count_change(update.hash_after);
					if (update_node_after) {
						switch (update.insert_op) {
							case InsertOp::CHANGE_VALUE:
								changeValue<depth, NodeCompression ::compressed>(update, node_after_count_diff);
								break;
							case InsertOp::EXPAND_C_NODE:
								insertTwoValueUncompressed<depth>(update, node_after_count_diff);
								break;
							default:
								assert(false);
								break;
						}
					}
				}
			}

			for (AtomicUpdate<depth> &update : complex_updates) {
				if (not movable_hashes_after.count(update.hash_after)) {
					const auto [update_node_after, node_after_count_diff] = pop_count_change(update.hash_after);
					if (update_node_after) {
						switch (update.insert_op) {
							case InsertOp::CHANGE_VALUE:
								changeValue<depth, NodeCompression ::compressed>(update, node_after_count_diff);
								break;
							case InsertOp::EXPAND_UC_NODE:
								insertIntoUncompressed<depth>(update, node_after_count_diff);
								break;
							default:
								assert(false);
						}
					}
				}
			}

			for (AtomicUpdate<depth> &update : movable_updates) {
				const auto [update_node_after, node_after_count_diff] = pop_count_change(update.hash_after);
				assert(update_node_after);
				switch (update.insert_op) {
					case InsertOp::CHANGE_VALUE:
						changeValue<depth, NodeCompression ::compressed, true>(update, node_after_count_diff);
						break;
					case InsertOp::EXPAND_UC_NODE:
						insertIntoUncompressed<depth, true>(update, node_after_count_diff);
						break;
					default:
						assert(false);
				}
			}


			for (auto &node_hash : finally_unreferenced_hashes_before)
				node_storage.template deleteNode<depth>(node_hash);

			if constexpr (depth > 1) apply_rek<depth - 1>();
		}

		template<size_t depth>
		void insertCompressedNode(const AtomicUpdate<depth> &update, const long after_count_diff) {
			auto nc_after = node_storage.template getCompressedNode<depth>(update.hash_after);
			if (nc_after.empty()) {// node_after doesn't exit already
				node_storage.template newCompressedNode<depth>(
						update.key, update.value, after_count_diff, update.hash_after);
			} else {
				nc_after.node()->ref_count() += after_count_diff;
			}
		}

		template<size_t depth, NodeCompression compression, bool reuse_node_before = false>
		void changeValue(const AtomicUpdate<depth> &update, const long after_count_diff) {


			SpecificNodeContainer<depth, compression, tri> nc_after;

			if constexpr (reuse_node_before) {// node before ref_count is zero -> maybe reused
				// update the node_before with the after_count and value
				auto nc_before = node_storage.template getNode<depth, compression>(update.hash_before);
				nc_after = node_storage.template changeNodeValue<depth, compression, false>(
						nc_before, update.value, after_count_diff, update.hash_after);
			} else {
				nc_after = node_storage.template getNode<depth, compression>(update.hash_before);
				if (nc_after.empty()) {
					auto nc_before = node_storage.template getNode<depth, compression>(update.hash_before);
					if constexpr (compression == NodeCompression::compressed)
						node_storage.template newCompressedNode<depth>(
								nc_before.node()->key(), update.value, after_count_diff, update.hash_after);
					else
						// keeps the node_before
						nc_after = node_storage.template changeNodeValue<depth, compression, true>(
								nc_before, update.value, after_count_diff, update.hash_after);
				} else {
					assert(not reuse_node_before);
					nc_after.node()->ref_count() += after_count_diff;
				}
			}

			if constexpr (depth == update_depth and compression == NodeCompression::uncompressed)
				this->nodec = nc_after;
		}

		template<size_t depth>
		void insertTwoValueUncompressed(const AtomicUpdate<depth> &update, const long after_count_diff) {
			auto nc_after = node_storage.template getUncompressedNode<depth>(update.hash_after);
			if (nc_after.empty()) {// node_after doesn't exit already
				node_storage.template newUncompressedNode<depth>(
						update.key, update.value,
						update.second_key, update.second_value, after_count_diff, update.hash_after);
			} else {
				nc_after.node()->ref_count() += after_count_diff;
			}
		}

		template<size_t depth, bool reuse_node_before = false>
		void insertIntoUncompressed(const AtomicUpdate<depth> &update, const long after_count_diff) {

			UncompressedNodeContainer<depth, tri> nc_after;

			if constexpr (reuse_node_before) {// node before ref_count is zero -> maybe reused
											  // update the node_before with the after_count and value
				auto nc_before = node_storage.template getUncompressedNode<depth>(update.hash_before);
				assert(not nc_before.null());
				nc_after = node_storage.template insertEntryIntoUncompressedNode<depth, false>(
						nc_before, update.key, update.value, after_count_diff, update.hash_after);
			} else {
				nc_after = node_storage.template getUncompressedNode<depth>(update.hash_after);
				if (nc_after.empty()) {// node_after doesn't exit already
					auto nc_before = node_storage.template getUncompressedNode<depth>(update.hash_before);
					nc_after = node_storage.template insertEntryIntoUncompressedNode<depth, true>(
							nc_before, update.key, update.value, after_count_diff, update.hash_after);
				} else {
					nc_after.node()->ref_count() += after_count_diff;
				}
			}

			if constexpr (depth == update_depth)
				this->nodec = nc_after;
		}
	};
}// namespace hypertrie::internal::node_based

#endif//HYPERTRIE_NODESTORAGEUPDATE_HPP
