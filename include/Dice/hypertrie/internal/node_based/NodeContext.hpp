#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/NodeStorage.hpp"
#include "TaggedNodeHash.hpp"

#include <Dice/hypertrie/internal/util/CountDownNTuple.hpp>

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

		[[nodiscard]] bool empty() const { return node_ == nullptr; }
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
		void removeNode(TaggedNodeHash hash) {

			// TODO: decrement counter
			// TODO: remove from NodeStorage if counter is 0
			// TODO: remove recursively if this counter AND their counter is 0
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
			return false;
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
		inline auto get(NodeContainer<depth, tri> &nodec, pos_type pos, key_part_type key_part)
		-> std::conditional<(depth > 1), NodeContainer<depth - 1, tri>, value_type> {
			assert (nodec.thash_.isUncompressed());
			assert(pos < depth);
			if constexpr(depth > 1) {
				auto found = nodec.compressed_node->edges_[pos].find(key_part);
				if (found) {
					TaggedNodeHash child_hash = found.second;
					auto &child_node_storage = getNodeStorage<depth - 1>();
					if (child_hash.isCompressed()) {
						auto &child_node = child_node_storage.compressed_nodes_[child_hash];
						return NodeContainer<depth - 1, tri>{child_hash, &child_node};
					}
				} else
					return NodeContainer<depth - 1, tri>{};
			} else { // depth == 1
				auto found = nodec->compressed_node->edges_.find(key_part);
				if (found) {
					if constexpr(tri::is_bool)
						return true;
					else
						return found.second;
				} else
					return {}; // false, 0, 0.0
			}
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
					RawKey<depth - 1> next_key;
					for (auto i = 0, j = 0; i < depth; ++i)
						if (i != pos) next_key[j++] = key[i];
					return get<depth - 1>(child, next_key);
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
