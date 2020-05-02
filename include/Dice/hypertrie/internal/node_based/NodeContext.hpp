#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include <compare>

#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/NodeStorage.hpp"
#include "TaggedNodeHash.hpp"

#include <Dice/hypertrie/internal/util/CountDownNTuple.hpp>
#include <itertools.hpp>

namespace hypertrie::internal::node_based {

	template<pos_type depth,
			typename tri_t = Hypertrie_internal_t<>>
	struct NodeContainer {
		TaggedNodeHash thash_{};
	private:
		void *node_ = nullptr;
	public:
		NodeContainer(const TaggedNodeHash &thash, void *node) : thash_(thash), node_(node) {}

		Node<depth, true, tri_t> *compressed_node() {
			return static_cast<Node<depth, true, tri_t> *>(node_);
		}

		Node<depth, false, tri_t> *uncompressed_node() {
			return static_cast<Node<depth, false, tri_t> *>(node_);
		}

		[[nodiscard]] bool empty() const { return thash_ == TaggedNodeHash{}; }
	};

	template<pos_type max_depth,
			typename tri_t = Hypertrie_internal_t<>,
			typename  = typename std::enable_if_t<(max_depth >= 1)>>
	class NodeContext {
	public:
		using tri = tri_t;
		/// public definitions
		using key_part_type = typename tri::key_part_type_t;
		using value_type = typename tri::key_part_type_t;
		template<typename key, typename value>
		using map_type = typename tri::template map_type_t<key, value>;
		template<typename key>
		using set_type = typename tri::template set_type_t<key>;
		template<pos_type depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<pos_type depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

		template<pos_type depth>
		using NodeStorage_t = NodeStorage<depth, tri>;
	private:
		util::CountDownNTuple<NodeStorage_t, max_depth> node_storages_{};
		std::list<TaggedNodeHash> primary_nodes_{};

		template<pos_type depth>
		NodeStorage_t<depth> &getNodeStorage() {
			return std::get<depth - 1>(node_storages_);
		}

		template<pos_type depth>
		NodeContainer<depth, tri> &getCompressedNode(const TaggedNodeHash &node_hash) {
			assert(node_hash.isCompressed());
			auto compressed_nodes = getNodeStorage<depth>().compressed_nodes_;
			auto found = compressed_nodes.find(node_hash);
			if (found != compressed_nodes.end())
				return {node_hash, &found.value()};
			else
				return {};
		}

		template<pos_type depth>
		NodeContainer<depth, tri> &getUncompressedNode(const TaggedNodeHash &node_hash) {
			assert(node_hash.isUncompressed());
			auto uncompressed_nodes = getNodeStorage<depth>().uncompressed_nodes_;
			auto found = uncompressed_nodes.find(node_hash);
			if (found != uncompressed_nodes.end())
				return {node_hash, &found.value()};
			else
				return {};
		}

	public:

		template<pos_type depth>
		NodeContainer<depth, tri> newPrimaryNode() {
			TaggedNodeHash base_hash{depth};
			primary_nodes_.push_front(base_hash);
			auto node_storage = getNodeStorage<depth>();
			auto found = node_storage.uncompressed_nodes_.find(base_hash);
			if (found) {
				auto &node = found.value();
				++node.ref_count_;
				return NodeContainer{base_hash, &node};
			} else {
				auto nodec_inserted = node_storage.uncompressed_nodes_.insert(base_hash, Node<depth, false, tri>{});
				auto &node = nodec_inserted.first.value();
				++node.ref_count_;
				return NodeContainer{base_hash, &nodec_inserted.first.value()};
			}
		}

		template<pos_type depth>
		void removeNode(NodeContainer<depth, tri> &nodec) {
			if (nodec.compressed_node) {
				auto *node = nodec.compressed_node();
				if (--node->ref_count_ == 0)
					getNodeStorage<depth>().compressed_nodes_.erase(nodec.thash_);
			} else {
				// TODO: first implement set
				// TODO: decrement counter
				// TODO: remove from NodeStorage if counter is 0
				// TODO: remove recursively if this counter AND their counter is 0
			}


		}

		template<pos_type depth>
		bool deletePrimaryNode(TaggedNodeHash thash) {
			{ // remove the hash from primary nodes list
				auto found = std::find(primary_nodes_.begin(), primary_nodes_.end(), thash);
				if (found == primary_nodes_.end())
					return false;
				primary_nodes_.erase(found);
			}
			removeNode<depth>(thash);

			return true;
		}


		template<pos_type depth>
		bool change(NodeContainer<depth, tri> &nodec, RawKey<depth>) {
			// TODO: implement or remove?
			return false;
		}

		template<pos_type depth>
		inline auto getChildHash(NodeContainer<depth, tri> &nodec, pos_type pos, key_part_type key_part)
		-> std::optional<std::conditional<(depth > 1), TaggedNodeHash, value_type>> {
			assert (nodec.thash_.isUncompressed());
			assert(pos < depth);
			auto &edges = nodec.compressed_node->edges_[pos];
			if constexpr(depth > 1) {
				auto found = edges.find(key_part);
				if (found != edges.end()) {
					return found.second;
				} else
					return std::nullopt;
			} else { // depth == 1
				auto found = edges.find(key_part);
				if (found != edges.end()) {
					if constexpr(tri::is_bool)
						return true;
					else
						return found.second;
				} else
					return {}; // false, 0, 0.0
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
		template<pos_type depth>
		inline auto getChild(NodeContainer<depth, tri> &nodec, pos_type pos, key_part_type key_part)
		-> std::conditional<(depth > 1), NodeContainer<depth - 1, tri>, value_type> {
			// TODO: use getChildHash
			assert (nodec.thash_.isUncompressed());
			assert(pos < depth);
			auto &edges = nodec.compressed_node->edges_[pos];
			if constexpr(depth > 1) {
				auto child_hash_opt = getChildHash<depth>(nodec, pos, key_part);
				if(child_hash_opt.has_value()){
					TaggedNodeHash child_hash = child_hash_opt.value();
					if (child_hash.isCompressed()){
						return getCompressedNode<depth>(child_hash);
					} else {
						return getUncompressedNode<depth>(child_hash);
					}
				} else {
					return {};
				}
			} else { // depth == 1
				auto value_opt = getChildHash<depth>(nodec, pos, key_part);
				if (value_opt.has_value()) {
					return value_opt.value();
				} else
					return {}; // false, 0, 0.0
			}
		}

		template<pos_type depth>
		inline void deleteChild(NodeContainer<depth, tri> &nodec, pos_type pos, key_part_type key_part) {
			assert (nodec.thash_.isUncompressed());
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
		 */template<pos_type depth>
		auto set(NodeContainer<depth, tri> &nodec, RawKey<depth> key, value_type value) -> value_type {
			auto[old_value, _] = set_rek<depth, depth>({nodec}, {{nodec, 1}}, key, value);
			return old_value;
		}

		struct set_rek_result {
			value_type old_value_;
			bool done_ = false;
		};


		enum struct InsertOp : unsigned int {
			CHANGE_VALUE = 0,
			INSERT_C_NODE,
			INSERT_TWO_KEY_UC_NODE,
			EXPAND_UC_NODE,
			EXPAND_C_NODE
		};

		template<pos_type depth>
		struct PlannedUpdate {

			InsertOp insert_op{};
			TaggedNodeHash hash_before{};
			RawKey<depth> sub_key{};
			value_type value{};
			RawKey<depth> second_sub_key{};
			value_type second_value{};
		public:
			auto operator<=>(const PlannedUpdate<depth> &other) const = default;

		};


		template<pos_type depth, pos_type total_depth>
		auto set_rek(
				std::vector<std::tuple<NodeContainer<depth, tri>, RawKey<depth>, value_type>> node_cs,
				std::vector<std::tuple<RawKey<depth>, value_type, RawKey<depth>, value_type>> expand_uc,
				bool &only_value_changes, value_type new_value
		) -> set_rek_result {
			bool change_only_the_value = false;


			Set<PlannedUpdate<depth - 1>> planned_updates{};

			std::vector<std::tuple<NodeContainer<depth - 1, tri>, RawKey<depth - 1>, value_type>> next_node_cs{};

			std::vector<std::tuple<RawKey<depth - 1>, value_type, RawKey<depth - 1>, value_type>> next_expand_uc{};

			for (auto &[nodec, key, value] : node_cs) {
				for (pos_type pos: iter::range(depth)) {
					RawKey<depth - 1> sub_key = subkey<depth>(key, pos);
					PlannedUpdate<depth - 1> planned_update{};
					planned_update.sub_key = sub_key;
					planned_update.value = value;
					auto child_hash_opt = getChildHash(nodec, pos, key[pos]);
					if (child_hash_opt.has_value()) {
						TaggedNodeHash child_hash = child_hash_opt.value();
						planned_update.hash_before = child_hash;
						if (child_hash.isCompressed()) {
							NodeContainer<depth - 1, tri> childc = getCompressedNode<depth - 1>(child_hash);
							auto *child = childc.compressed_node();
							if (child->key_ == sub_key) {
								if (child->value_ == value and value == new_value) {
									return {value, true}; // nothing left to be done. The value is already set.
								} else { // child->value_ != value
									if (value == new_value)
										only_value_changes = true;
									planned_update.insert_op = InsertOp::CHANGE_VALUE;
								}
							} else { // child->key_ != sub_key
								planned_update.second_sub_key = child->key_;
								planned_update.second_value = child->value_;
								next_expand_uc.push_back(
										{{{child->key_, child->value_}, {sub_key, value}}});
								planned_update.insert_op = InsertOp::EXPAND_C_NODE;
							}
						} else { // child_hash.isUncompressed()
							next_node_cs.push_back({getUncompressedNode<depth - 1>(child_hash), sub_key, value});
							planned_update.insert_op = InsertOp::EXPAND_UC_NODE;
						}
					} else { // not child_hash_opt.has_value()
						// no child exists for that key_part at that position
						planned_update.insert_op = InsertOp::INSERT_C_NODE;
					}
					planned_updates.insert(std::move(planned_update));
				}

				// creating uncompressed nodes with two keys (expanded compressed nodes)
				for (auto &[key, value, second_key, second_value] : expand_uc) {
					for (pos_type pos: iter::range(depth)) {
						RawKey<depth - 1> sub_key = subkey<depth>(key, pos);
						RawKey<depth - 1> second_sub_key = subkey<depth>(second_key, pos);
						if (key[pos] == second_key[pos]) {
							PlannedUpdate<depth - 1> planned_update{};
							planned_update.sub_key = sub_key;
							planned_update.value = value;
							planned_update.second_sub_key = second_sub_key;
							planned_update.second_value = second_value;
							planned_update.insert_op = InsertOp::INSERT_TWO_KEY_UC_NODE;
							planned_updates.insert(std::move(planned_update));
							// todo: add to next_exp_node_cs
						} else {
							PlannedUpdate<depth - 1> planned_update{};
							planned_update.sub_key = sub_key;
							planned_update.value = value;
							planned_update.insert_op = InsertOp::INSERT_C_NODE;
							planned_updates.insert(std::move(planned_update));

							PlannedUpdate<depth - 1> second_planned_update{};
							second_planned_update.sub_key = second_sub_key;
							second_planned_update.value = second_value;
							second_planned_update.insert_op = InsertOp::INSERT_C_NODE;
							planned_updates.insert(std::move(second_planned_update));
						}
					}
				}

				Map<TaggedNodeHash, PlannedUpdate<depth - 1>> required_updates;
				for (auto &planned_update : planned_updates) {

				}
			}


		}

		template<pos_type depth>
		auto subkey(const RawKey<depth> &key, pos_type remove_pos) -> RawKey<depth - 1> {
			RawKey<depth - 1> sub_key;
			for (auto i = 0, j = 0; i < depth; ++i)
				if (i != remove_pos) sub_key[j++] = key[i];
			return sub_key;
		}

		/**
		 * Retrieves the value for a key.
		 * @tparam depth the depth of the node container
		 * @param nodec the node container
		 * @param key the key
		 * @return the value associated to the key.
		 */
		template<pos_type depth>
		auto get(NodeContainer<depth, tri> &nodec, RawKey<depth> key) -> value_type {
			if (nodec.thash_.isCompressed()) {
				auto *node = nodec.compressed_node();
				if (node->key_ == key) {
					if constexpr(tri::is_bool) return true;
					else return node->value_;
				} else
					return {};
			} else if constexpr(depth > 1) {
				// TODO: implement minCardPos();
				auto pos = 0;//minCardPos();
				NodeContainer<depth - 1, tri> child = get<depth>(nodec, pos, key[pos]);
				if (not child.empty()) {
					return get<depth - 1>(child, subkey(key, pos));
				} else {
					return {}; // false, 0, 0.0
				}
			} else {
				return get<1>(nodec, 0, key[0]);
			}
		}

		template<pos_type depth>
		bool slice(NodeContainer<depth, tri> &nodec, RawSliceKey<depth>) {
			return false;
		}

		template<pos_type depth>
		bool size(NodeContainer<depth, tri> &nodec) {

		}


	};
}

#endif //HYPERTRIE_NODECONTEXT_HPP
