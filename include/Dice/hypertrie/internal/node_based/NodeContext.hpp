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
		TaggedNodeHash thash_;
	private:
		void *node_;
	public:
		NodeContainer(const TaggedNodeHash &thash, void *node) : thash_(thash), node_(node) {}

		Node<depth, true, tri_t> *compressed_node() {
			return static_cast<Node<depth, true, tri_t> *>(node_);
		}

		Node<depth, false, tri_t> *uncompressed_node() {
			return static_cast<Node<depth, false, tri_t> *>(node_);
		}
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
			// TODO: increment counter
			primary_nodes_.push_front(base_hash);
			auto node_storage = getNodeStorage<depth>();
			auto found = node_storage.uncompressed_nodes_.find(base_hash);
			if (found) {
				return NodeContainer{base_hash, &found.value()};
			} else {
				auto nodec_inserted = node_storage.uncompressed_nodes_.insert(base_hash, Node<depth, false, tri>{});
				return NodeContainer{base_hash, &nodec_inserted.first.value()};
			}
		}

		template<pos_type depth>
		bool deletePrimaryNode(TaggedNodeHash thash) {
			TaggedNodeHash base_hash{depth};
			{ // remove from primary nodes list
				auto found = std::find(primary_nodes_.begin(), primary_nodes_.end(), thash);
				if (found == primary_nodes_.end())
					return false;
				primary_nodes_.erase(found);
			}
			// TODO: increment counter
			// TODO: remove from NodeStorage if counter is 0
			// TODO: remove recursively if this counter AND their counter is 0
			return true;
		}


		template<pos_type depth>
		bool change(NodeContainer<depth, tri> &nodec, RawKey<depth>) {
			return false;
		}

		template<pos_type depth>
		value_type get(NodeContainer<depth, tri> &nodec, RawKey<depth>) {
			return false;
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
