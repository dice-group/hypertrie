#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include <compare>

#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/NodeContainer.hpp"
#include "Dice/hypertrie/internal/node_based/NodeStorage.hpp"
#include "Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"

#include <Dice/hypertrie/internal/util/CountDownNTuple.hpp>
#include <itertools.hpp>

namespace hypertrie::internal::node_based {

	template<size_t max_depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			 typename = typename std::enable_if_t<(max_depth >= 1)>>
	class NodeContext {
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
		util::CountDownNTuple<NodeStorage_t, max_depth> node_storages_{};
		std::list<TaggedNodeHash> primary_nodes_{};

		template<size_t depth>
		NodeStorage_t<depth> &getNodeStorage() {
			return std::get<depth - 1>(node_storages_);
		}

		template<size_t depth, NodeCompression compressed>
		auto getNodeStorage() -> std::conditional_t<bool(compressed),
													typename NodeStorage_t<depth>::CompressedNodeMap, typename NodeStorage_t<depth>::UncompressedNodeMap> & {
			if constexpr (compressed == NodeCompression::compressed)
				return std::get<depth - 1>(node_storages_).compressedNodes();
			else
				return std::get<depth - 1>(node_storages_).uncompressedNodes();
		}

		template<size_t depth>
		CompressedNodeContainer<depth, tri> getCompressedNode(const TaggedNodeHash &node_hash) {
			return getNode<depth, NodeCompression::compressed>(node_hash);
		}

		template<size_t depth>
		UncompressedNodeContainer<depth, tri> getUncompressedNode(const TaggedNodeHash &node_hash) {
			return getNode<depth, NodeCompression::uncompressed>(node_hash);
		}

		template<size_t depth>
		void deleteCompressedNode(const TaggedNodeHash &node_hash) {
			assert(node_hash.isCompressed());
			auto &compressed_nodes = getNodeStorage<depth, NodeCompression::compressed>();
			compressed_nodes.erase(node_hash);
		}

		template<size_t depth>
		void deleteCompressedNode(const NodeContainer<depth, tri> &node_container) {
			deleteCompressedNode<depth>(node_container.thash_);
		}

		template<size_t depth>
		void deleteUncompressedNode(const TaggedNodeHash &node_hash) {
			assert(node_hash.isUncompressed());
			auto &uncompressed_nodes = getNodeStorage<depth, NodeCompression ::uncompressed>();
			uncompressed_nodes.erase(node_hash);
		}

		template<size_t depth>
		void deleteUncompressedNode(const NodeContainer<depth, tri> &node_container) {
			deleteUncompressedNode<depth>(node_container.thash_);
		}

		template<size_t depth>
		void deleteNode(const TaggedNodeHash &node_hash) {
			if (node_hash.isCompressed()) {
				deleteCompressedNode<depth>(node_hash);
			} else {
				deleteUncompressedNode<depth>(node_hash);
			}
		}

		template<size_t depth>
		NodeContainer<depth, tri> newCompressedNode(RawKey<depth> key, value_type value, size_t ref_count, TaggedNodeHash hash) {
			auto &node_storage = getNodeStorage<depth, NodeCompression::compressed>();
			auto [it, success] = [&]() {
				if constexpr(tri::is_bool_valued) return node_storage.insert({hash, CompressedNode<depth, tri>{key, ref_count}});
				else return node_storage.insert({hash, CompressedNode<depth, tri>{key, value, ref_count}}); }();
			assert(success);
			return NodeContainer<depth, tri>{hash, &NodeStorage<depth, tri>::template deref<NodeCompression::compressed>(it)};
		}

		template<size_t depth>
		NodeContainer<depth, tri> newUncompressedNode(RawKey<depth> key, value_type value, RawKey<depth> second_key, value_type second_value, size_t ref_count, TaggedNodeHash hash) {
			auto [it, success] = getNodeStorage<depth, NodeCompression::uncompressed>().insert({hash, UncompressedNode<depth, tri>{key, value, second_key, second_value, ref_count}});
			assert(success);
			return NodeContainer<depth, tri>{hash, &NodeStorage<depth, tri>::template deref<NodeCompression::uncompressed>(it)};
		}

		/**
		 * creates a new node with the value changed. The old node is NOT deleted if keep_old is true and must eventually be deleted afterwards.
		 */
		template<size_t depth, NodeCompression compressed, bool keep_old = true>
		NodeContainer<depth, tri> changeNodeValue(NodeContainer<depth, tri> nc, value_type value, long count_diff, TaggedNodeHash new_hash) {
			auto nc_ = nc.template specific<compressed>();
			if constexpr (compressed == NodeCompression::compressed and not tri::is_bool_valued) nc_.node()->value() = value;
			auto &nodes = getNodeStorage<depth, compressed>();

			auto [it, success] = [&]() {
				if constexpr (keep_old) return nodes.insert({new_hash, *nc_.node()});
				else
					return nodes.insert({new_hash, std::move(*nc_.node())});// if the old is not kept it is moved
			}();
			assert(success);
			if constexpr (not keep_old) {
				const auto removed = nodes.erase(nc_.thash_);
				assert(removed);
			}
			auto &node = NodeStorage<depth, tri>::template deref<compressed>(it);
			node.ref_count() += count_diff;
			return NodeContainer<depth, tri>{new_hash, &node};
		}

		template<size_t depth, bool keep_old = true>
		NodeContainer<depth, tri> insertEntryIntoUncompressedNode(NodeContainer<depth, tri> nc, RawKey<depth> key, value_type value, long count_diff, TaggedNodeHash new_hash) {
			auto nc_ = nc.uncompressed();
			auto &nodes = getNodeStorage<depth, NodeCompression::uncompressed>();
			auto [it, success] = [&]() {
				if constexpr (keep_old) return nodes.insert({new_hash, *nc_.node()});
				else
					return nodes.insert({new_hash, std::move(*nc_.node())});// if the old is not kept it is moved
			}();
			assert(success);
			assert(nc_.thash_ != new_hash);
			if constexpr (not keep_old) {
				const auto removed = nodes.erase(nc_.thash_);
				assert(removed);
				it = nodes.find(new_hash); // iterator was invalidates by modifying nodes. get a new one
			}
			auto &node = NodeStorage<depth, tri>::template deref<NodeCompression::uncompressed>(it);
			node.insertEntry(key, value);
			node.ref_count() += count_diff;
			return NodeContainer<depth, tri>{new_hash, &node};
		}

		template<size_t depth>
		NodeContainer<depth, tri> &getNode(const TaggedNodeHash &node_hash) {
			if (node_hash.isCompressed()) {
				getCompressedNode<depth>(node_hash);
			} else {
				getUncompressedNode<depth>(node_hash);
			}
		}

		template<size_t depth, NodeCompression compressed>
		SpecificNodeContainer<depth, compressed, tri> getNode(const TaggedNodeHash &node_hash) {
			assert(node_hash.isCompressed() == bool(compressed) or node_hash); // TODO: check if that really makes sense here
			auto &nodes = getNodeStorage<depth, compressed>();
			auto found = nodes.find(node_hash);
			if (found != nodes.end())
				return {node_hash, &NodeStorage<depth, tri>::template deref<compressed>(found)};
			else
				return {};
		}

	public:
		template<size_t depth>
		NodeContainer<depth, tri> newPrimaryNode() {
			TaggedNodeHash base_hash = TaggedNodeHash::getUncompressedEmptyNodeHash<depth>();
			primary_nodes_.push_front(base_hash);
			auto &node_storage = getNodeStorage<depth, NodeCompression::uncompressed>();
			auto found = node_storage.find(base_hash);
			if (found != node_storage.end()) {
				auto &node = NodeStorage<depth, tri>::template deref<NodeCompression::uncompressed>(found);
				++node.ref_count();
				return {base_hash, &node};
			} else {
				auto nodec_inserted = node_storage.insert({base_hash, UncompressedNode<depth, tri>{}});
				auto &node = NodeStorage<depth, tri>::template deref<NodeCompression::uncompressed>(nodec_inserted.first);
				++node.ref_count();
				return {base_hash, &node};
			}
		}

		template<size_t depth, NodeCompression compression>
		void decrementNodeCount(SpecificNodeContainer<depth, compression, tri> &nodec) {
			if (--node->ref_count() == 0){
				if constexpr( compression == NodeCompression::uncompressed) {
					for (size_t pos : iter::range(depth)){
						auto &edges = nodec.node()->edges(pos);
						// TODO: it makes sense to collect the changes
					}
						
				}
				getNodeStorage<depth>().compressed_nodes_.erase(nodec.thash_);
			} else {
				// TODO: first implement set
				// TODO: decrement counter
				// TODO: remove from NodeStorage if counter is 0
				// TODO: remove recursively if this counter AND their counter is 0
			}
		}

		template<size_t depth>
		bool deletePrimaryNode(TaggedNodeHash thash) {
			{// remove the hash from primary nodes list
				auto found = std::find(primary_nodes_.begin(), primary_nodes_.end(), thash);
				if (found == primary_nodes_.end())
					return false;
				primary_nodes_.erase(found);
			}
			decrementNodeCount<depth, NodeCompression::uncompressed>(thash);

			return true;
		}


		template<size_t depth>
		bool change(NodeContainer<depth, tri> &nodec, RawKey<depth>) {
			// TODO: implement or remove?
			return false;
		}

		template<size_t depth>
		inline auto getChildHashOrValue(NodeContainer<depth, tri> &nodec, size_t pos, key_part_type key_part)
				-> std::optional<std::conditional_t<(depth > 1), TaggedNodeHash, value_type>> {
			assert(nodec.thash_.isUncompressed());
			assert(pos < depth);
			auto [was_found, iter] = nodec.uncompressed_node()->find(pos, key_part);
			if (was_found) {
				if constexpr (depth == 1 and tri::is_bool_valued)
					return true;
				else
					return iter->second;
			} else {
				return std::nullopt;// false, 0, 0.0
			}
		}

		/**
		 * Resolves a keypart by a given position
		 * @tparam depth the depth of the node container
		 * @param nodec the node container, must be a uncompressed node
		 * @param pos the position at which the key_part should be resolved
		 * @param key_part the key part
		 * @return an NodeContainer of depth - 1. It might be empty if there is no child for the given pos and key part. <br/>
		 * If depth is 1, a value_type is return
		 */
		template<size_t depth>
		inline auto getChild(NodeContainer<depth, tri> &nodec, size_t pos, key_part_type key_part)
				-> std::conditional_t<(depth > 1), NodeContainer<depth - 1, tri>, value_type> {
			// TODO: use getChildHashOrValue
			assert(nodec.thash_.isUncompressed());
			assert(pos < depth);
			if constexpr (depth > 1) {
				auto child_hash_opt = getChildHashOrValue<depth>(nodec, pos, key_part);
				if (child_hash_opt.has_value()) {
					TaggedNodeHash child_hash = child_hash_opt.value();
					if (child_hash.isCompressed()) {
						return NodeContainer<depth - 1, tri>{getCompressedNode<depth - 1>(child_hash)};
					} else {
						return NodeContainer<depth - 1, tri>{getUncompressedNode<depth - 1>(child_hash)};
					}
				} else {
					return {};
				}
			} else {// depth == 1
				auto value_opt = getChildHashOrValue<depth>(nodec, pos, key_part);
				if (value_opt.has_value()) {
					return value_opt.value();
				} else
					return {};// false, 0, 0.0
			}
		}

		template<size_t depth>
		inline void deleteChild(NodeContainer<depth, tri> &nodec, size_t pos, key_part_type key_part) {
			assert(nodec.thash_.isUncompressed());
			assert(pos < depth);
			auto &edges = nodec.compressed_node->edges_[pos];
			edges.erase(key_part);
		}

		template<class K, class V>
		using Map = container::boost_flat_map<K, V>;
		template<class K>
		using Set = container::boost_flat_set<K>;

		/**
		 *
		 * @tparam depth
		 * @param nodec
		 * @param key
		 * @return old value
		 */
		template<size_t depth>
		auto set(NodeContainer<depth, tri> &nodec, const RawKey<depth> &key, value_type value) -> value_type {
			value_type old_value = {};
			if (value != value_type{}) {
				bool only_value_changes = false;
				bool nothing_to_change = set_rek<depth, depth>({{nodec, key, value}}, {}, only_value_changes, old_value, value);
				if (not nothing_to_change) {// if something changed, update the primary node and reflect the changes to nodec
					if (only_value_changes) {
						auto new_hash = TaggedNodeHash{nodec.thash_}.changeValue(key, old_value, value);
						nodec = changeNodeValue<depth, NodeCompression::uncompressed, false>(nodec, value, 0, new_hash);
					} else {
						auto new_hash = TaggedNodeHash{nodec.thash_}.addEntry(key, value);
						nodec = insertEntryIntoUncompressedNode<depth, false>(nodec, key, value, 0, new_hash);
					}
				}
			} else {
				throw std::logic_error{"Delete is not yet implemented."};
			}
			return old_value;
		}

		struct set_rek_result {
			value_type old_value_;
			bool done_ = false;
		};


		enum struct InsertOp : unsigned int {
			CHANGE_VALUE = 0,
			INSERT_TWO_KEY_UC_NODE,
			INSERT_C_NODE,
			EXPAND_UC_NODE,
			EXPAND_C_NODE
		};

		template<size_t depth>
		struct PlannedUpdate {

			InsertOp insert_op{};
			TaggedNodeHash hash_before{};
			RawKey<depth> sub_key{};
			value_type value{};
			RawKey<depth> second_sub_key{};
			value_type second_value{};

		public:
			auto hashAfter(const value_type &old_value) const -> TaggedNodeHash {
				TaggedNodeHash next_hash = hash_before;
				switch (insert_op) {
					case InsertOp::CHANGE_VALUE:
						next_hash.changeValue(sub_key, old_value, value);
						break;
					case InsertOp::INSERT_TWO_KEY_UC_NODE:
						next_hash = TaggedNodeHash::getTwoEntriesNodeHash(sub_key, value, second_sub_key, second_value);
						break;
					case InsertOp::INSERT_C_NODE:
						next_hash = TaggedNodeHash::getCompressedNodeHash(sub_key, value);
						break;
					case InsertOp::EXPAND_UC_NODE:
						assert(hash_before.isUncompressed());
						next_hash.addEntry(sub_key, value);
						break;
					case InsertOp::EXPAND_C_NODE:
						assert(hash_before.isCompressed());
						next_hash.addEntry(sub_key, value);
						break;
				}
				return next_hash;
			}

			auto operator<=>(const PlannedUpdate<depth> &other) const = default;
		};


		/**
		 * 
		 * @tparam depth depth of the nodes currently processed
		 * @tparam total_depth starting depth
		 * @param node_cs vector of (NodeContainer, RawKey<depth>, value_type) to be inserted.  
		 * @param expand_uc vector of two entries (RawKey<depth>, value_type, RawKey<depth> (2), value_type (2)) that should form a node.
		 * @param only_value_changes this is set to true if the key already exists with another value
		 * @param old_value the old value
		 * @param new_value the new laue or 0, 0.0, false if it was not yet found or wasn't set before
		 * @return 
		 */
		template<size_t depth, size_t total_depth>
		auto set_rek(
				std::vector<std::tuple<NodeContainer<depth, tri>, RawKey<depth>, value_type>> node_cs,
				std::vector<std::tuple<RawKey<depth>, value_type, RawKey<depth>, value_type>> expand_uc,
				bool &only_value_changes,
				const value_type &old_value,
				const value_type new_value) -> bool {
			static_assert(depth >= 1);
			if constexpr (depth == 1) {
				if (new_value == value_type{})
					for (auto &[nodec, key, value] : node_cs) {
						if (value == new_value) {// it is not just an expanded node
							auto old_value_opt = getChildHashOrValue(nodec, 0, key[0]);
							if (old_value_opt.has_value()) {
								if (old_value_opt.value() == new_value)
									return true;
								else
									only_value_changes = true;
							}
							break;
						}
					}
				return false;
			} else {
				static constexpr const auto subkey = &tri::template subkey<depth>;
				bool change_only_the_value = false;
				// TODO: think about how to handle primary nodes
				//  -> we don't need to handle them extra as they are always ref_count >= 1

				std::vector<PlannedUpdate<depth - 1>> planned_updates{};

				std::vector<std::tuple<NodeContainer<depth - 1, tri>, RawKey<depth - 1>, value_type>> next_node_cs{};

				std::vector<std::tuple<RawKey<depth - 1>, value_type, RawKey<depth - 1>, value_type>> next_expand_uc{};
				// descending
				if constexpr (depth > 1) {// we only need to look at what to do with subtries for a depth > 1.
					for (auto &[nodec, key, value] : node_cs) {
						for (size_t pos : iter::range(depth)) {
							RawKey<depth - 1> sub_key = subkey(key, pos);
							PlannedUpdate<depth - 1> planned_update{};
							planned_update.sub_key = sub_key;
							planned_update.value = value;
							auto child_hash_opt = getChildHashOrValue(nodec, pos, key[pos]);
							if (child_hash_opt.has_value()) {
								TaggedNodeHash child_hash = child_hash_opt.value();
								planned_update.hash_before = child_hash;
								if (child_hash.isCompressed()) {
									NodeContainer<depth - 1, tri> childc = getCompressedNode<depth - 1>(child_hash);
									auto *child = childc.compressed_node();
									if (child->key() == sub_key) {
										if constexpr (tri::is_bool_valued)
											return true;// nothing left to be done. The value is already set.
										else {
											if (child->value() == value) {
												if (value == new_value) {
													return true;// nothing left to be done. The value is already set.
												}
											} else {// child->value() != value
												if (value == new_value) {
													only_value_changes = true;
												}
												planned_update.insert_op = InsertOp::CHANGE_VALUE;
											}
										}
									} else {// child->key_ != sub_key
										planned_update.second_sub_key = child->key();
										if constexpr (not tri::is_bool_valued)
											planned_update.second_value = child->value();
										next_expand_uc.push_back(
												{sub_key, value, planned_update.second_sub_key, planned_update.second_value});
										planned_update.insert_op = InsertOp::EXPAND_C_NODE;
									}
								} else {// child_hash.isUncompressed()
									next_node_cs.push_back(
											{getUncompressedNode<depth - 1>(child_hash), sub_key, value});
									planned_update.insert_op = InsertOp::EXPAND_UC_NODE;
								}
							} else {// not child_hash_opt.has_value()
								// no child exists for that key_part at that position
								planned_update.insert_op = InsertOp::INSERT_C_NODE;
							}
							planned_updates.push_back(std::move(planned_update));
						}
					}
					// creating uncompressed nodes with two keys (expanded compressed nodes)
					for (auto &[key, value, second_key, second_value] : expand_uc) {
						for (size_t pos : iter::range(depth)) {
							RawKey<depth - 1> sub_key = subkey(key, pos);
							RawKey<depth - 1> second_sub_key = subkey(second_key, pos);
							if (key[pos] == second_key[pos]) {
								PlannedUpdate<depth - 1> planned_update{};
								planned_update.sub_key = sub_key;
								planned_update.value = value;
								planned_update.second_sub_key = second_sub_key;
								planned_update.second_value = second_value;
								planned_update.insert_op = InsertOp::INSERT_TWO_KEY_UC_NODE;
								planned_updates.push_back(std::move(planned_update));
								next_expand_uc.push_back({sub_key, value, second_sub_key, second_value});
							} else {
								PlannedUpdate<depth - 1> planned_update{};
								planned_update.sub_key = sub_key;
								planned_update.value = value;
								planned_update.insert_op = InsertOp::INSERT_C_NODE;
								planned_updates.push_back(std::move(planned_update));

								PlannedUpdate<depth - 1> second_planned_update{};
								second_planned_update.sub_key = second_sub_key;
								second_planned_update.value = second_value;
								second_planned_update.insert_op = InsertOp::INSERT_C_NODE;
								planned_updates.push_back(std::move(second_planned_update));
							}
						}
					}
					bool done = set_rek<depth - 1, total_depth>(next_node_cs, next_expand_uc, only_value_changes, old_value, new_value);
					if (done)
						return true;
				}

				// ascending
				std::map<TaggedNodeHash, long> count_changes{};
				for (PlannedUpdate<depth - 1> &planned_update : planned_updates) {
					if (planned_update.hash_before)
						count_changes[planned_update.hash_before]--;
					if (auto hash_after = planned_update.hashAfter(old_value); hash_after)
						count_changes[hash_after]++;
				}

				auto pop_count_change = [&](TaggedNodeHash hash) -> std::tuple<bool, long> {
					if (auto changed = count_changes.find(hash); changed != count_changes.end()) {
						auto diff = changed->second;
						count_changes.erase(changed);
						return {true, diff};
					} else
						return {false, 0};
				};

				Set<TaggedNodeHash> nodes_to_remove{};

				std::vector<std::vector<PlannedUpdate<depth - 1>>> grouped_updates = [&]() {// group by hash before
					std::sort(planned_updates.begin(), planned_updates.end(),
							  [](auto a, auto b) { return a.hash_before < b.hash_before; });
					std::vector<std::vector<PlannedUpdate<depth - 1>>> grouped_updates;
					auto first = 0;
					const auto last = std::size(planned_updates) - 1;
					for (auto current : iter::range(std::size(planned_updates))) {
						if (planned_updates[first].hash_before != planned_updates[current].hash_before or current >= last) {
							std::vector<PlannedUpdate<depth - 1>> next_group{begin(planned_updates) + first, begin(planned_updates) + current + 1};
							grouped_updates.push_back(next_group);
							first = current;
						}
					}
					return grouped_updates;
				}();

				for (std::vector<PlannedUpdate<depth - 1>> &updated_group : grouped_updates) {
					const TaggedNodeHash hash_before = updated_group[0].hash_before;
					const auto [update_node_before, node_before_count_diff] = pop_count_change(hash_before);

					for (long i : iter::range(updated_group.size() - 1)) {
						auto &planned_update = updated_group[i];
						const TaggedNodeHash hash_after = planned_update.hashAfter(old_value);
						const auto [update_node_after, node_after_count_diff] = pop_count_change(hash_after);
						if (hash_before.isCompressed())
							updateNode<depth - 1, NodeCompression::compressed, true>(
									planned_update, hash_before, update_node_before, node_before_count_diff, hash_after,
									update_node_after, node_after_count_diff, nodes_to_remove);
						else
							updateNode<depth - 1, NodeCompression::uncompressed, true>(
									planned_update, hash_before, update_node_before, node_before_count_diff, hash_after,
									update_node_after, node_after_count_diff, nodes_to_remove);
					}
					auto &planned_update = *updated_group.rbegin();
					const TaggedNodeHash hash_after = planned_update.hashAfter(old_value);
					const auto [update_node_after, node_after_count_diff] = pop_count_change(hash_after);
					bool reused_node_before = false;
					if (hash_before.isCompressed())
						reused_node_before = updateNode<depth - 1, NodeCompression::compressed, false>(
								planned_update, hash_before, update_node_before, node_before_count_diff, hash_after,
								update_node_after, node_after_count_diff, nodes_to_remove);
					else
						reused_node_before = updateNode<depth - 1, NodeCompression::uncompressed, false>(
								planned_update, hash_before, update_node_before, node_before_count_diff, hash_after,
								update_node_after, node_after_count_diff, nodes_to_remove);

					// TODO: that is not yet optimal as we cannot be sure that a change will be the last which actually
					//  reuses the node_before
					if (not reused_node_before)
						count_changes[hash_before] = 0;
				}

				for (auto &node_hash : nodes_to_remove)
					deleteNode<depth - 1>(node_hash);

				return false;
			}
		}

		template<size_t depth, NodeCompression is_before_compressed, bool keep_before>
		auto updateNode(const PlannedUpdate<depth> &planned_update,
						const TaggedNodeHash hash_before, const bool update_before_node, const long before_count_diff,
						const TaggedNodeHash hash_after, const bool update_after_node, const long after_count_diff,
						Set<TaggedNodeHash> &nodes_to_remove)
				-> std::conditional_t<keep_before, void, bool> {

			bool reuse_node_before = false;
			bool reused_node_before = false;
			auto nc_before = (hash_before)
									 ? getNode<depth, is_before_compressed>(hash_before)
									 : SpecificNodeContainer<depth, is_before_compressed, tri>{};
			if (update_before_node) {
				auto *node_before = nc_before.node();
				node_before->ref_count() += before_count_diff;
				if (node_before->ref_count() == 0)
					reuse_node_before = true;
				nodes_to_remove.insert(hash_before);
			}

			switch (planned_update.insert_op) {
				case InsertOp::CHANGE_VALUE: {
					if (update_after_node) {
						auto nc_after = getNode<depth, is_before_compressed>(hash_after);
						if (reuse_node_before) {   // node before ref_count is zero -> maybe reused
							if (nc_after.empty()) {// node_after doesn't exit already
								// update the node_before with the after_count and value
								changeNodeValue<depth, is_before_compressed, keep_before>(
										nc_before, planned_update.value, after_count_diff, hash_after);
								if constexpr (not keep_before)
									reused_node_before = true;
							} else {
								nc_after.node()->ref_count() += after_count_diff;
							}
						} else {
							if (nc_after.empty()) {// node_after doesn't exit already
								if constexpr (is_before_compressed == NodeCompression::compressed)
									newCompressedNode<depth>(
											nc_before.node()->key(), planned_update.value, after_count_diff, hash_after);
								else
									// keeps the node_before
									changeNodeValue<depth, is_before_compressed, true>(
											nc_before, planned_update.value, after_count_diff, hash_after);
							} else {
								nc_after.node()->ref_count() += after_count_diff;
							}
						}
					}
				} break;
				case InsertOp::INSERT_TWO_KEY_UC_NODE: {
					if (update_after_node) {
						auto nc_after = getUncompressedNode<depth>(hash_after);
						if (nc_after.empty()) {// node_after doesn't exit already
							newUncompressedNode<depth>(
									planned_update.sub_key, planned_update.value, planned_update.second_sub_key,
									planned_update.second_value, after_count_diff, hash_after);
						} else {
							nc_after.node()->ref_count() += after_count_diff;
						}
					}
				} break;
				case InsertOp::INSERT_C_NODE: {
					if (update_after_node) {
						auto nc_after = getNode<depth, is_before_compressed>(hash_after);
						if (nc_after.empty()) {// node_after doesn't exit already
							newCompressedNode<depth>(
									planned_update.sub_key, planned_update.value, after_count_diff, hash_after);
						} else {
							nc_after.node()->ref_count() += after_count_diff;
						}
					}
				} break;
				case InsertOp::EXPAND_UC_NODE: {
					if (update_after_node) {
						auto nc_after = getNode<depth, is_before_compressed>(hash_after);
						if (reuse_node_before) {   // node before ref_count is zero -> maybe reused
							if (nc_after.empty()) {// node_after doesn't exit already
								// update the node_before with the after_count and value
								insertEntryIntoUncompressedNode<depth, keep_before>(
										nc_before, planned_update.sub_key, planned_update.value, after_count_diff, hash_after);
								if constexpr (not keep_before)
									reused_node_before = true;
							} else {
								nc_after.node()->ref_count() += after_count_diff;
							}
						} else {
							if (nc_after.empty()) {// node_after doesn't exit already
								insertEntryIntoUncompressedNode<depth, true>(
										nc_before, planned_update.sub_key, planned_update.value, after_count_diff, hash_after);
							} else {
								nc_after.node()->ref_count() += after_count_diff;
							}
						}
					}
				} break;
				case InsertOp::EXPAND_C_NODE: {
					if (update_after_node) {
						auto nc_after = getUncompressedNode<depth>(hash_after);
						if (nc_after.empty()) {// node_after doesn't exit already
							auto node_before = nc_before.compressed().node();
							value_type existing_value = [&]() { if constexpr (tri::is_bool_valued) return true; else return node_before->value(); }();
							newUncompressedNode<depth>(
									node_before->key(), existing_value, planned_update.second_sub_key,
									planned_update.second_value, after_count_diff, hash_after);
						} else {
							nc_after.node()->ref_count() += after_count_diff;
						}
					}
				} break;
			}

			if constexpr (not keep_before) {
				if (hash_before) {
					auto deleted = nodes_to_remove.erase(hash_before);
					assert(deleted);
				}
				return reused_node_before;
			}
		}

		/**
		 * Retrieves the value for a key.
		 * @tparam depth the depth of the node container
		 * @param nodec the node container
		 * @param key the key
		 * @return the value associated to the key.
		 */
		template<size_t depth>
		auto get(NodeContainer<depth, tri> &nodec, RawKey<depth> key) -> value_type {
			static constexpr const auto subkey = &tri::template subkey<depth>;
			if (nodec.thash_.isCompressed()) {
				auto *node = nodec.compressed_node();
				if (node->key() == key) {
					if constexpr (tri::is_bool_valued) return true;
					else
						return node->value();
				} else
					return {};
			} else if constexpr (depth > 1) {
				// TODO: implement minCardPos();
				auto pos = 0;//minCardPos();
				NodeContainer<depth - 1, tri> child = getChild<depth>(nodec, pos, key[pos]);
				if (not child.empty()) {
					return get<depth - 1>(child, subkey(key, pos));
				} else {
					return {};// false, 0, 0.0
				}
			} else {
				return getChild<1>(nodec, 0, key[0]);
			}
		}

		template<size_t depth>
		bool slice(NodeContainer<depth, tri> &nodec, RawSliceKey<depth>) {
			return false;
		}

		template<size_t depth>
		bool size(NodeContainer<depth, tri> &nodec) {
		}
	};
}// namespace hypertrie::internal::node_based

#endif//HYPERTRIE_NODECONTEXT_HPP
