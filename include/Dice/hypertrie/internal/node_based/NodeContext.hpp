#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include <compare>

#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/NodeContainer.hpp"
#include "Dice/hypertrie/internal/node_based/NodeStorage.hpp"
#include "Dice/hypertrie/internal/node_based/NodeStorageUpdate.hpp"
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

		using NodeStorage_t = LevelNodeStorage<max_depth, tri>;

	private:
		std::list<TaggedNodeHash> primary_nodes_{};
		NodeStorage_t node_storage_{};


	public:
		template<size_t depth>
		UncompressedNodeContainer<depth, tri> newPrimaryNode() {
			TaggedNodeHash base_hash = TaggedNodeHash::getUncompressedEmptyNodeHash<depth>();
			primary_nodes_.push_front(base_hash);
			auto &node_storage = getNodeStorage<depth, NodeCompression::uncompressed>();
			auto found = node_storage.find(base_hash);
			if (found != node_storage.end()) {
				auto &node = LevelNodeStorage<depth, tri>::template deref<NodeCompression::uncompressed>(found);
				++node.ref_count();
				return {base_hash, &node};
			} else {
				auto nodec_inserted = node_storage.insert({base_hash, UncompressedNode<depth, tri>{}});
				auto &node = LevelNodeStorage<depth, tri>::template deref<NodeCompression::uncompressed>(nodec_inserted.first);
				++node.ref_count();
				return {base_hash, &node};
			}
		}

		template<size_t depth, NodeCompression compression>
		void decrementNodeCount(SpecificNodeContainer<depth, compression, tri> &nodec) {
			//			if (--node->ref_count() == 0){
			//				if constexpr( compression == NodeCompression::uncompressed) {
			//					for (size_t pos : iter::range(depth)){
			//						auto &edges = nodec.node()->edges(pos);
			//						// TODO: it makes sense to collect the changes
			//					}
			//
			//				}
			//				getNodeStorage<depth>().compressed_nodes_.erase(nodec.thash_);
			//			} else {
			//				// TODO: first implement set
			//				// TODO: decrement counter
			//				// TODO: remove from NodeStorage if counter is 0
			//				// TODO: remove recursively if this counter AND their counter is 0
			//			}
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
			NodeStorageUpdate<max_depth, depth, tri> update{this->node_storage_};
			update.plan(nodec, key, value);
			update.apply();
			return update.old_value;
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
