#ifndef HYPERTRIE_LEVELNODESTORAGE_HPP
#define HYPERTRIE_LEVELNODESTORAGE_HPP

#include "Dice/hypertrie/internal/node_based/Hypertrie_internal_traits.hpp"
#include "Dice/hypertrie/internal/node_based/Node.hpp"
#include "Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/util/CountDownNTuple.hpp"

namespace hypertrie::internal::node_based {

	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>>
	struct LevelNodeStorage {
		using tri = tri_t;
		template<typename K, typename V>
		using map_type = typename tri::template map_type<K, V>;
		using CompressedNodeMap = map_type<TaggedNodeHash, CompressedNode<depth, tri>>;
		using UncompressedNodeMap = map_type<TaggedNodeHash, UncompressedNode<depth, tri>>;
		// TODO: add "revision" for compressed_nodes_ and uncompressed_nodes_
		// TODO: A node container must be updated if the revision does not fit the associated node storage
	protected:
		CompressedNodeMap compressed_nodes_;
		UncompressedNodeMap uncompressed_nodes_;

	public:
		const CompressedNodeMap &compressedNodes() const { return this->compressed_nodes_; }
		CompressedNodeMap &compressedNodes() { return this->compressed_nodes_; }

		const UncompressedNodeMap &uncompressedNodes() const { return this->uncompressed_nodes_; }
		UncompressedNodeMap &uncompressedNodes() { return this->uncompressed_nodes_; }

		template<NodeCompression compressed>
		static Node<depth, compressed, tri> &deref(typename map_type<TaggedNodeHash, Node<depth, compressed, tri>>::iterator &map_it) {
			return tri::template deref<TaggedNodeHash, Node<depth, compressed, tri>>(map_it);
		}

		explicit operator std::string() const {
			return fmt::format("{{ depth = {}\n"
							   " uncompressed: {}\n"
							   "  compressed: {} }}",
							   depth, this->uncompressed_nodes_, this->compressed_nodes_);
		}
	};

	template<size_t max_depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			 typename = typename std::enable_if_t<(max_depth >= 1)>>
	class NodeStorage {
	public:
		using tri = tri_t;

		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

		template<size_t depth>
		using NodeStorage_t = LevelNodeStorage<depth, tri>;

		template<size_t depth>
		using CompressedNodeMap = typename NodeStorage_t<depth>::CompressedNodeMap;

		template<size_t depth>
		using UncompressedNodeMap = typename NodeStorage_t<depth>::UncompressedNodeMap;

	private:
		using storage_t = util::CountDownNTuple<NodeStorage_t, max_depth>;

		storage_t storage_;


		// TODO: remove
		template<size_t depth>
		NodeStorage_t<depth> &getStorage() {
			return std::get<depth - 1>(this->storage_);
		}

		template<size_t depth>
		const NodeStorage_t<depth> &getStorage() const {
			return std::get<depth - 1>(this->storage_);
		}

	public:
		// TODO: private?
		template<size_t depth, NodeCompression compression>
		auto getNodeStorage() -> std::conditional_t<bool(compression),
													CompressedNodeMap<depth>, UncompressedNodeMap<depth>> & {
			if constexpr (compression == NodeCompression::compressed)
				return getStorage<depth>().compressedNodes();
			else
				return getStorage<depth>().uncompressedNodes();
		}

		template<size_t depth, NodeCompression compression>
		SpecificNodeContainer<depth, compression, tri> getNode(const TaggedNodeHash &node_hash) {
			auto &nodes = getNodeStorage<depth, compression>();
			auto found = nodes.find(node_hash);
			if (found != nodes.end())
				return {node_hash, &LevelNodeStorage<depth, tri>::template deref<compression>(found)};
			else
				return {};
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
		NodeContainer<depth, tri> getNode(const TaggedNodeHash &node_hash) {
			if (node_hash.isCompressed()) {
				return getCompressedNode<depth>(node_hash);
			} else {
				return getUncompressedNode<depth>(node_hash);
			}
		}

	public:
		template<size_t depth>
		CompressedNodeContainer<depth, tri> newCompressedNode(const RawKey<depth> &key, value_type value, size_t ref_count, TaggedNodeHash hash) {
			auto &node_storage = getNodeStorage<depth, NodeCompression::compressed>();
			auto [it, success] = [&]() {
			  if constexpr(tri::is_bool_valued) return node_storage.insert({hash, CompressedNode<depth, tri>{key, ref_count}});
			  else return node_storage.insert({hash, CompressedNode<depth, tri>{key, value, ref_count}}); }();
			assert(success);
			return CompressedNodeContainer<depth, tri>{hash, &LevelNodeStorage<depth, tri>::template deref<NodeCompression::compressed>(it)};
		}

		template<size_t depth>
		UncompressedNodeContainer<depth, tri> newUncompressedNode(RawKey<depth> key, value_type value, RawKey<depth> second_key, value_type second_value, size_t ref_count, TaggedNodeHash hash) {
			auto [it, success] = getNodeStorage<depth, NodeCompression::uncompressed>().insert({hash, UncompressedNode<depth, tri>{key, value, second_key, second_value, ref_count}});
			assert(success);
			return UncompressedNodeContainer<depth, tri>{hash, &LevelNodeStorage<depth, tri>::template deref<NodeCompression::uncompressed>(it)};
		}


		template<size_t depth>
		UncompressedNodeContainer<depth, tri> newUncompressedNode(size_t ref_count) {
			static const TaggedNodeHash hash = TaggedNodeHash::getUncompressedEmptyNodeHash<depth>();
			auto [it, success] = getNodeStorage<depth, NodeCompression::uncompressed>().insert({hash, UncompressedNode<depth, tri>{ref_count}});
			assert(success);
			return UncompressedNodeContainer<depth, tri>{hash, &LevelNodeStorage<depth, tri>::template deref<NodeCompression::uncompressed>(it)};
		}

		/**
		 * creates a new node with the value changed. The old node is NOT deleted if keep_old is true and must eventually be deleted afterwards.
		 */
		template<size_t depth, NodeCompression compression, bool keep_old = true>
		auto changeNodeValue(SpecificNodeContainer<depth, compression, tri> nc, RawKey<depth> key, value_type old_value, value_type new_value, long count_diff, TaggedNodeHash new_hash)
				-> SpecificNodeContainer<depth, compression, tri> {
			auto &nodes = getNodeStorage<depth, compression>();
			assert(nc.thash_ != new_hash);

			auto [it, success] = [&]() {
				if constexpr (keep_old) return nodes.insert({new_hash, *nc.node()});
				else
					return nodes.insert({new_hash, std::move(*nc.node())});// if the old is not kept it is moved
			}();
			assert(success);
			if constexpr (not keep_old) {
				const auto removed = nodes.erase(nc.thash_);
				assert(removed);
				it = nodes.find(new_hash);// iterator was invalidates by modifying nodes. get a new one
			}
			auto &node = LevelNodeStorage<depth, tri>::template deref<compression>(it);
			if constexpr (not tri::is_bool_valued){
				if constexpr (compression == NodeCompression::compressed) node.value() = new_value;
				else
					node.change_value(key, old_value, new_value);
			}
			if constexpr (keep_old)
				node.ref_count() = 0;
			assert(node.ref_count() == 0);
			node.ref_count() += count_diff;
			return {new_hash, &node};
		}

		template<size_t depth, bool keep_old = true>
		UncompressedNodeContainer<depth, tri> insertEntryIntoUncompressedNode(UncompressedNodeContainer<depth, tri> nc, RawKey<depth> key, value_type value, long count_diff, TaggedNodeHash new_hash) {
			auto &nodes = getNodeStorage<depth, NodeCompression::uncompressed>();
			auto [it, success] = [&]() {
				if constexpr (keep_old) return nodes.insert({new_hash, *nc.node()});
				else
					return nodes.insert({new_hash, std::move(*nc.node())});// if the old is not kept it is moved
			}();
			assert(success);
			assert(nc.thash_ != new_hash);
			if constexpr (not keep_old) {
				const auto removed = nodes.erase(nc.thash_);
				assert(removed);
				it = nodes.find(new_hash);// iterator was invalidates by modifying nodes. get a new one
			}
			auto &node = LevelNodeStorage<depth, tri>::template deref<NodeCompression::uncompressed>(it);
			node.insertEntry(key, value);
			if constexpr (keep_old)
				node.ref_count() = 0;
			node.ref_count() += count_diff;
			return {new_hash, &node};
		}

		template<size_t depth, NodeCompression compression>
		void deleteNode(const TaggedNodeHash &node_hash) {
			auto &nodes = getNodeStorage<depth, compression>();
			assert(nodes.count(node_hash));
			nodes.erase(node_hash);
		}

		template<size_t depth>
		void deleteNode(const TaggedNodeHash &node_hash) {
			if (node_hash.isCompressed()) {
				deleteNode<depth, NodeCompression::compressed>(node_hash);
			} else {
				deleteNode<depth, NodeCompression::uncompressed>(node_hash);
			}
		}

		explicit operator std::string() const {
			return std::string(
					fmt::format("[ NodeStorage \n"
								"{}"
								"]",
								this->to_string_rek<max_depth>()));
		}

	private:
		template<size_t str_depth>
		std::string to_string_rek() const {
			if constexpr (str_depth < 1)
				return {};
			else
				return ((std::string) this->getStorage<str_depth>()) + "\n" + this->to_string_rek<str_depth -1>();
		}

	public:
		friend std::ostream &operator<<(std::ostream &os, NodeStorage &storage) {
			os << (std::string) storage;
			return os;
		}
	};


}// namespace hypertrie::internal::node_based

#endif//HYPERTRIE_LEVELNODESTORAGE_HPP
