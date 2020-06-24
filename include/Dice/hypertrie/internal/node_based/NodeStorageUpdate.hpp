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
		using LevelUpdates_t = std::set<AtomicUpdate<depth>>;

		using PlannedUpdates = util::CountDownNTuple<LevelUpdates_t, update_depth>;


		enum struct InsertOp : unsigned int {
			CHANGE_VALUE = 0,
			INSERT_TWO_KEY_UC_NODE,
			INSERT_C_NODE,
			EXPAND_UC_NODE,
			EXPAND_C_NODE,
			CHANGE_REF_COUNT
		};

		enum struct Modification : unsigned int {
			CHANGE = 0,
			INSERT,
			DELETE
		};

		template<size_t depth>
		struct AtomicUpdate {
			static_assert(depth >= 1);

			Modification mod{};
			InsertOp insert_op{};
			TaggedNodeHash hash_before{};
			RawKey<depth> key{};
			value_type value{};
			RawKey<depth> second_key{};
			value_type second_value{};
			mutable TaggedNodeHash hash_after{};
			mutable long ref_count = 1;

		public:
			void calcHashAfter(const value_type &old_value) const {
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
					case InsertOp::CHANGE_REF_COUNT:
						assert(not hash_before.empty());
						hash_after = {};
						break;
				}
			}

			bool operator<(const AtomicUpdate<depth> &other) const {
				return std::make_tuple(this->mod, this->insert_op, this->hash_before, this->key, this->value, this->second_key, this->second_value) <
					   std::make_tuple(other.mod, other.insert_op, other.hash_before, other.key, other.value, other.second_key, other.second_value);
			};

			bool operator==(const AtomicUpdate<depth> &other) const {
				return std::make_tuple(this->mod, this->insert_op, this->hash_before, this->key, this->value, this->second_key, this->second_value) <
					   std::make_tuple(other.mod, other.insert_op, other.hash_before, other.key, other.value, other.second_key, other.second_value);
			};
		};


		template<size_t depth>
		struct SingleEntryChange {
			UncompressedNodeContainer<depth, tri> nodec;
			RawKey<depth> key;
			value_type value;
			Modification mod{};
			bool operator<(const SingleEntryChange &other) const {
				return std::make_tuple(this->mod, this->nodec.thash_, this->key, this->value) <
					   std::make_tuple(other.mod, other.nodec.thash_, other.key, other.value);
			}

			bool operator==(const SingleEntryChange &other) const {
				return std::make_tuple(this->mod, this->nodec.thash_, this->key, this->value) ==
					   std::make_tuple(other.mod, other.nodec.thash_, other.key, other.value);
			}
		};

		template<size_t depth>
		struct NewDoubleEntryNode {
			RawKey<depth> key;
			value_type value;
			RawKey<depth> second_key;
			value_type second_value;
			bool operator<(const NewDoubleEntryNode<depth> &other) const {
				return std::make_tuple(this->key, this->value, this->second_key, this->second_value) <
					   std::make_tuple(other.key, other.value, other.second_key, other.second_value);
			}
			bool operator==(const NewDoubleEntryNode<depth> &other) const {
				return std::make_tuple(this->key, this->value, this->second_key, this->second_value) ==
					   std::make_tuple(other.key, other.value, other.second_key, other.second_value);
			}
		};


	public:
		NodeStorage_t<node_storage_depth> &node_storage;

		UncompressedNodeContainer<update_depth, tri> &nodec;

		value_type new_value;

		value_type old_value{};

		bool only_value_changes = false;

		bool nothing_changes = false;

		PlannedUpdates planned_updates{};

		template<size_t updates_depth>
		auto getPlannedUpdates()
				-> std::set<AtomicUpdate<updates_depth>> & {
			return std::get<updates_depth - 1>(planned_updates);
		}

		template<size_t updates_depth>
		void planUpdate(AtomicUpdate<updates_depth> planned_update, const long count_diff = 1) {
			auto &planned_updates = getPlannedUpdates<updates_depth>();
			auto it = planned_updates.find(planned_update);
			if (it == planned_updates.end())
				planned_updates.insert(it, std::move(planned_update));
			else {
				auto &x = *it;
				assert(planned_update.hash_before == x.hash_before);
				x.ref_count += count_diff;
			}
		}

		bool
		hash_changes() {
			return not this->nothing_changes;
		}


		NodeStorageUpdate(NodeStorage_t<node_storage_depth> &nodeStorage, UncompressedNodeContainer<update_depth, tri> &nodec)
			: node_storage(nodeStorage), nodec{nodec} {}


		void plan(const RawKey<update_depth> &key, value_type value, value_type old_value) {
			// TODO: handle insert and change value seperately.
			this->old_value = this->old_value;
			this->only_value_changes = this->old_value != value_type{};
			if (this->only_value_changes) {
				AtomicUpdate<update_depth> update{};
				update.hash_before = nodec;
				update.mod = Modification::CHANGE;
				update.key = key;
				update.value = value;
				planUpdate(std::move(update));

				plan_change_value_rek<update_depth>();
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
		auto plan_change_value_rek() {
			// we only need to look at what to do with subtries for a depth > 1.
			static constexpr const auto subkey = &tri::template subkey<depth>;

			apply_updates<depth>();

			if constexpr (depth > 1)
				plan_change_value_rek<depth - 1>();
		}

		template<size_t depth>
		void apply_updates() {

			std::set<AtomicUpdate<depth>> &updates = getPlannedUpdates<depth>();

			// reference changes to single nodes
			std::map<TaggedNodeHash, long> count_changes{};

			// extract a change from count_changes
			auto pop_count_change = [&](TaggedNodeHash hash) -> std::tuple<bool, long> {
				if (auto changed = count_changes.find(hash); changed != count_changes.end()) {
					auto diff = changed->second;
					count_changes.erase(changed);
					return {diff != 0, diff};
				} else
					return {false, 0};
			};

			// check if a hash is in count_changes
			auto peak_count_change = [&](TaggedNodeHash hash) -> bool {
				if (auto changed = count_changes.find(hash); changed != count_changes.end()) {
					auto diff = changed->second;
					return diff != 0;
				} else
					return false;
			};

			// all nodes used as nodes before.
			std::map<TaggedNodeHash, NodeContainer<depth, tri>> nodes_before{};

			// populate count_changes and nodes_before
			for (const AtomicUpdate<depth> &update : updates) {
				if (not update.hash_before.empty()) {
					count_changes[update.hash_before] -= update.ref_count;
					nodes_before[update.hash_before];// NodeContainer is not yet materialized
				}

				if (not update.hash_after.empty()) {
					count_changes[update.hash_after] += update.ref_count;
					// nodes_after[update.hash_after];
				}
			}

			// nodes_before that are have ref_count 0 afterwards -> those could be reused/moved for nodes after
			std::map<TaggedNodeHash, NodeContainer<depth, tri>> unreferenced_nodes_before{};

			// process reference changes to nodes_before
			for (auto &[hash_before, nodec_before] : nodes_before) {
				const auto &[before_changes, before_count_change] = pop_count_change(hash_before);
				if (before_changes) {
					nodec_before = node_storage.template getNode<depth>(hash_before);
					assert(not nodec_before.null());
					size_t *ref_count = [&]() {
						if (nodec_before.isCompressed())
							return &nodec_before.compressed_node()->ref_count();
						else
							return &nodec_before.uncompressed_node()->ref_count();
					}();
					*ref_count += before_count_change;

					if (*ref_count == 0) {
						unreferenced_nodes_before[hash_before] = nodec_before;
					}
				}
			}

			std::set<AtomicUpdate<depth>> moveable_updates{};
			// extract movables and do unmoveables
			for (const AtomicUpdate<depth> &update : updates) {
				// skip CHANGE_REF_COUNT as they were already applied in the nodes_before loop
				if (update.insert_op == InsertOp::CHANGE_REF_COUNT)
					continue;
				// check if it is a moveable. then save it and skip to the next iteration
				if (peak_count_change(update.hash_after)) {

					if (auto handle = unreferenced_nodes_before.extract(update.hash_before);
						// TODO: maybe also skip EXPAND_C_NODE ?
						not handle.empty()) {
						moveable_updates.insert(update);
					} else {
						auto [changes, after_count_change] = pop_count_change(update.hash_after);
						NodeContainer<depth, tri> node_before = nodes_before[update.hash_before];
						if (update.hash_before.isCompressed())
							processUpdate<depth, NodeCompression::compressed, false>(update, after_count_change, node_before.compressed());
						else
							processUpdate<depth, NodeCompression::uncompressed, false>(update, after_count_change, node_before.uncompressed());
					}
				}
			}

			// do moveables
			for (const AtomicUpdate<depth> &update : moveable_updates) {
				NodeContainer<depth, tri> node_before = nodes_before[update.hash_before];
				auto [changes, after_count_change] = pop_count_change(update.hash_after);

				assert(changes);
				if (update.hash_before.isCompressed())
					processUpdate<depth, NodeCompression::compressed, true>(update, after_count_change, node_before.compressed());
				else
					processUpdate<depth, NodeCompression::uncompressed, true>(update, after_count_change, node_before.uncompressed());
			}

			for (auto &[hash_before, _] : unreferenced_nodes_before) {
				node_storage.template deleteNode<depth>(hash_before);
			}
		}

		template<size_t depth, NodeCompression compression, bool reuse_node_before = false>
		void processUpdate(const AtomicUpdate<depth> &update, const long after_count_diff, SpecificNodeContainer<depth, compression, tri> nc_before) {
			// TODO: deactivate unreachable code with if constexpr()
			switch (update.insert_op) {
				case InsertOp::CHANGE_VALUE:
					changeValue<depth, compression, reuse_node_before>(update, after_count_diff, nc_before);
					break;
				case InsertOp::INSERT_TWO_KEY_UC_NODE:
					insertTwoValueUncompressed<depth>(update, after_count_diff);
					break;
				case InsertOp::INSERT_C_NODE:
					insertCompressedNode<depth>(update, after_count_diff);
					break;
				case InsertOp::EXPAND_UC_NODE:
					insertCompressedNode<depth>(update, after_count_diff);
					break;
				case InsertOp::EXPAND_C_NODE:
					// TODO: merge with insert two key uc?
					insertTwoValueUncompressed<depth>(update, after_count_diff);
					break;
				case InsertOp::CHANGE_REF_COUNT:
					break;
			}
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
		void changeValue(const AtomicUpdate<depth> &update, const long after_count_diff, SpecificNodeContainer<depth, compression, tri> nc_before) {
			assert(not nc_before.null());
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

					planUpdate(std::move(child_update));
				}
			}

			SpecificNodeContainer<depth, compression, tri> nc_after;

			// reusing the node_before
			if constexpr (reuse_node_before) {// node before ref_count is zero -> maybe reused
				// update the node_before with the after_count and value
				assert(not nc_before.null());
				// create updates for sub node

				nc_after = node_storage.template changeNodeValue<depth, compression, false>(
						nc_before, update.key, this->old_value, update.value, after_count_diff, update.hash_after);

			} else {// leaving the node_before untouched
				nc_after = node_storage.template getNode<depth, compression>(update.hash_after);
				if (nc_after.empty()) {
					if constexpr (compression == NodeCompression::compressed)
						node_storage.template newCompressedNode<depth>(
								nc_before.node()->key(), update.value, after_count_diff, update.hash_after);
					else {
						nc_after = node_storage.template changeNodeValue<depth, compression, true>(
								nc_before, update.key, this->old_value, update.value, after_count_diff, update.hash_after);

						if constexpr (depth > 1) {
							UncompressedNode<depth, tri> *node_before = nc_before.node();

							if (node_before->ref_count() == 0) {

								for (size_t pos : iter::range(depth)) {
									for (const auto &[_, hash] : node_before->edges(pos)) {
										AtomicUpdate<depth - 1> update_ref_count{};
										update_ref_count.hash_before = hash;
										update_ref_count.insert_op = InsertOp::CHANGE_REF_COUNT;
										update_ref_count.ref_count = -1L;

										planUpdate(std::move(update_ref_count), -1L);
									}
								}
							}
						}
					}
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

					if constexpr (depth > 1) {
						UncompressedNode<depth, tri> *node_before = nc_before.node();
						for (size_t pos : iter::range(depth)) {
							for (const auto &[_, hash] : node_before->edges(pos)) {
								AtomicUpdate<depth - 1> update_ref_count{};
								update_ref_count.hash_before = hash;
								update_ref_count.insert_op = InsertOp::CHANGE_REF_COUNT;
								update_ref_count.ref_count = -1L;

								planUpdate(std::move(update_ref_count), -1L);
							}
						}
					}

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
