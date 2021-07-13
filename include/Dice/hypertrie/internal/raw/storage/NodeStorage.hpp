#ifndef HYPERTRIE_LEVELNODESTORAGE_HPP
#define HYPERTRIE_LEVELNODESTORAGE_HPP

#include "Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp"
#include "Dice/hypertrie/internal/raw/node/Node.hpp"
#include "Dice/hypertrie/internal/raw/node/TensorHash.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp"

#include <memory>

namespace hypertrie::internal::raw {

	template<size_t depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			 typename Allocator = std::allocator<size_t>,
			 typename = void>
	struct LevelNodeStorage {
		using tri = tri_t;
		using allocator_type = Allocator;
		template<typename mapped_type>
		using map_alloc_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<mapped_type>;

		template<typename mapped_type>
		using mapped_type_ptr = typename std::allocator_traits<map_alloc_type<mapped_type>>::pointer;
		template<typename K, typename V>
		using map_type = typename tri::template map_type<K, mapped_type_ptr<V>, map_alloc_type<std::pair<K, mapped_type_ptr<V>>>>;
		using CompressedNode_type = CompressedNode<depth, tri, allocator_type>;
		using UncompressedNodeMap_type = UncompressedNode<depth, tri, allocator_type>;
		using map_key_type = TensorHash;
		using CompressedNodeMap = map_type<map_key_type, CompressedNode_type>;
		using UncompressedNodeMap = map_type<map_key_type, UncompressedNodeMap_type>;
		// TODO: add "revision" for compressed_nodes_ and uncompressed_nodes_
		// TODO: A node container must be updated if the revision does not fit the associated node storage
	protected:
		map_alloc_type<CompressedNode_type> compressed_nodes_alloc_;
		map_alloc_type<UncompressedNodeMap_type> uncompressed_nodes_alloc_;
		CompressedNodeMap compressed_nodes_;
		UncompressedNodeMap uncompressed_nodes_;

	public:

		LevelNodeStorage(const Allocator& alloc // = Allocator()
						 ) : compressed_nodes_alloc_(alloc),
							 uncompressed_nodes_alloc_(alloc),
							 compressed_nodes_(alloc),
							 uncompressed_nodes_(alloc) {}

		~LevelNodeStorage() {
			using c_alloc_trait = std::allocator_traits<map_alloc_type<CompressedNode_type>>;
			for (auto &[hash, node] : this->compressed_nodes_) {
				c_alloc_trait::destroy(compressed_nodes_alloc_, std::to_address(node));
				c_alloc_trait::deallocate(compressed_nodes_alloc_, node, 1);
			}
			using uc_alloc_trait = std::allocator_traits<map_alloc_type<UncompressedNodeMap_type>>;
			for (auto &[hash, node] : this->uncompressed_nodes_) {
				uc_alloc_trait::destroy(uncompressed_nodes_alloc_, std::to_address(node));
				uc_alloc_trait::deallocate(uncompressed_nodes_alloc_, node, 1);
			}
		}

		const CompressedNodeMap &compressedNodes() const { return this->compressed_nodes_; }
		CompressedNodeMap &compressedNodes() { return this->compressed_nodes_; }

		const UncompressedNodeMap &uncompressedNodes() const { return this->uncompressed_nodes_; }
		UncompressedNodeMap &uncompressedNodes() { return this->uncompressed_nodes_; }

		template<NodeCompression compression>
		static Node<depth, compression, tri> &deref(typename map_type<map_key_type, Node<depth, compression, tri>>::iterator &map_it) {
			return *tri::template deref<map_key_type, Node<depth, compression, tri> *>(map_it);
		}

		explicit operator std::string() const {
			return fmt::format("{{ depth = {}\n"
							   " uncompressed: {}\n"
							   "  compressed: {} }}",
							   depth, this->uncompressed_nodes_, this->compressed_nodes_);
		}
	};

	template<HypertrieInternalTrait tri_t, typename Allocator>
	struct LevelNodeStorage<1, tri_t, Allocator, std::enable_if_t<(tri_t::is_lsb_unused and tri_t::is_bool_valued)>> {
		using tri = tri_t;
		using allocator_type = Allocator;
		template<typename mapped_type>
		using map_alloc_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<mapped_type>;

		template<typename mapped_type>
		using mapped_type_ptr = typename std::allocator_traits<map_alloc_type<mapped_type>>::pointer;
		template<typename K, typename V>
		using map_type = typename tri::template map_type<K, mapped_type_ptr<V>, map_alloc_type<std::pair<K, mapped_type_ptr<V>>>>;
		using UncompressedNodeMap_type = UncompressedNode<1, tri, allocator_type>;
		using map_key_type = TensorHash;
		using UncompressedNodeMap = map_type<map_key_type, UncompressedNodeMap_type>;

	protected:
		map_alloc_type<UncompressedNodeMap_type> uncompressed_nodes_alloc_;
		UncompressedNodeMap uncompressed_nodes_;

	public:
		LevelNodeStorage(const Allocator &alloc /* = Allocator()*/)
			: uncompressed_nodes_alloc_(alloc),
			  uncompressed_nodes_(alloc) {}

		~LevelNodeStorage() {
			using uc_alloc_trait = std::allocator_traits<map_alloc_type<UncompressedNodeMap_type>>;
			for (auto &[hash, node] : this->uncompressed_nodes_) {
				uc_alloc_trait::destroy(uncompressed_nodes_alloc_, std::to_address(node));
				uc_alloc_trait::deallocate(uncompressed_nodes_alloc_, node, 1);
			}
		}

		const UncompressedNodeMap &uncompressedNodes() const { return this->uncompressed_nodes_; }
		UncompressedNodeMap &uncompressedNodes() { return this->uncompressed_nodes_; }

		template<NodeCompression compression = NodeCompression::uncompressed>
		static UncompressedNode<1, tri> &deref(typename map_type<TensorHash, UncompressedNode<1, tri>>::iterator &map_it) {
			static_assert(compression == NodeCompression::uncompressed, "LevelNodeStorage of depth 1 with is_lsb_unused and is_bool_valued doesn't manage compressed nodes.");
			return *tri::template deref<map_key_type, UncompressedNode<1, tri> *>(map_it);
		}

		explicit operator std::string() const {
			return fmt::format("{{ depth = 1\n"
							   " uncompressed: {} }}",
							   this->uncompressed_nodes_);
		}
	};

	/*template<size_t max_depth,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			 typename Allocator = std::allocator<size_t>,
			 typename = typename std::enable_if_t<(max_depth >= 1)>>
	class NodeStorage {
	public:
		using tri = tri_t;
		using allocator_type = Allocator;

		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

		template<size_t depth>
		using NodeStorage_t = LevelNodeStorage<depth, tri, allocator_type>;

		template<size_t depth>
		using CompressedNodeMap = typename NodeStorage_t<depth>::CompressedNodeMap;

		template<size_t depth>
		using UncompressedNodeMap = typename NodeStorage_t<depth>::UncompressedNodeMap;

	private:
		using storage_t = util::IntegralTemplatedTuple<NodeStorage_t, 1, max_depth>;

		storage_t storage_;


		// TODO: remove
		template<size_t depth>
		NodeStorage_t<depth> &getStorage() {
			return this->storage_.template get<depth>();
		}

		template<size_t depth>
		const NodeStorage_t<depth> &getStorage() const {
			return this->storage_.template get<depth>();
		}

	public:
		NodeStorage(const Allocator& alloc // = Allocator()
					) : storage_(alloc){}

		// TODO: private?
		template<size_t depth, NodeCompression compression, typename = std::enable_if_t<(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued and compression == NodeCompression::compressed))>>
		auto &getNodeStorage() {
			if constexpr (compression == NodeCompression::compressed)
				return getStorage<depth>().compressedNodes();
			else
				return getStorage<depth>().uncompressedNodes();
		}

		template<size_t depth, NodeCompression compression, typename = std::enable_if_t<(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued and compression == NodeCompression::compressed))>>
		SpecificNodeContainer<depth, compression, tri> getNode(const TensorHash &node_hash) {
			auto &nodes = getNodeStorage<depth, compression>();
			auto found = nodes.find(node_hash);
			if (found != nodes.end())
				if constexpr((depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued))
					return {node_hash, &LevelNodeStorage<depth, tri>::deref(found)};
				else
					return {node_hash, &LevelNodeStorage<depth, tri>::template deref<compression>(found)};
			else
				return {};
		}

		template<size_t depth, typename = std::enable_if_t<(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued))>>
		CompressedNodeContainer<depth, tri> getCompressedNode(const TensorHash &node_hash) {
			return getNode<depth, NodeCompression::compressed>(node_hash);
		}

		template<size_t depth>
		UncompressedNodeContainer<depth, tri> getUncompressedNode(const TensorHash &node_hash) {
			return getNode<depth, NodeCompression::uncompressed>(node_hash);
		}

		template<size_t depth>
		NodeContainer<depth, tri> getNode(const TensorHash &node_hash) {
			if (node_hash.isCompressed()) {
				assert(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued));
				if constexpr(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued))
					return getCompressedNode<depth>(node_hash);
			} else {
				return getUncompressedNode<depth>(node_hash);
			}
		}

	public:
		template<size_t depth, typename = std::enable_if_t<(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued))>>
		CompressedNodeContainer<depth, tri> newCompressedNode(const RawKey<depth> &key, value_type value, size_t ref_count, TensorHash hash) {
			auto &node_storage = getNodeStorage<depth, NodeCompression::compressed>();
			auto [it, success] = [&]() {
			  if constexpr(tri::is_bool_valued) return node_storage.insert({hash, new CompressedNode<depth, tri>{key, ref_count}});
			  else return node_storage.insert({hash, new CompressedNode<depth, tri>{key, value, ref_count}});
			}();
			assert(success);
			return CompressedNodeContainer<depth, tri>{hash, &LevelNodeStorage<depth, tri>::template deref<NodeCompression::compressed>(it)};
		}


		*//**
		 * creates a new node with the value changed. The old node is NOT deleted if keep_old is true and must eventually be deleted afterwards.
		 *//*
		template<size_t depth, NodeCompression compression, bool keep_old = true, typename = std::enable_if_t<(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued and compression == NodeCompression::compressed))>>
		auto changeNodeValue(SpecificNodeContainer<depth, compression, tri> nc, RawKey<depth> key, value_type old_value, value_type new_value, long count_diff, TensorHash new_hash)
				-> SpecificNodeContainer<depth, compression, tri> {
			auto &nodes = getNodeStorage<depth, compression>();
			assert(nc.hash() != new_hash);

			auto [it, success] = [&]() {
				if constexpr (keep_old) return nodes.insert({new_hash, new Node<depth, compression, tri>{*nc.template specific_node<compression>()}});
				else
					return nodes.insert({new_hash, nc.template specific_node<compression>()});// if the old is not kept it is moved
			}();
			assert(success);
			if constexpr (not keep_old) {
				const auto removed = nodes.erase(nc.hash());
				assert(removed);
				it = nodes.find(new_hash);// iterator was invalidates by modifying nodes. get a new one
			}
			auto &node = LevelNodeStorage<depth, tri>::template deref<compression>(it);
			if constexpr (not tri::is_bool_valued) {
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

		template<size_t depth, NodeCompression compression, typename = std::enable_if_t<(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued and compression == NodeCompression::compressed))>>
		void deleteNode(const TensorHash &node_hash) {
			auto &nodes = getNodeStorage<depth, compression>();
			auto it = nodes.find(node_hash);
			assert(it != nodes.end());
			auto *node = &LevelNodeStorage<depth, tri>::template deref<compression>(it);
			delete node;
			nodes.erase(it);
		}

		void setLSBCompressedLeaf(NodeContainer<1, tri> &nodec, const key_part_type &key_part, const bool &value) {
			if (value)
				nodec.hash() = TaggedTensorHash<tri>{key_part};
		}

		template<size_t depth>
		void deleteNode(const TensorHash &node_hash) {
			if (node_hash.isCompressed()) {
				assert(not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued));
				if constexpr (not (depth == 1 and tri_t::is_lsb_unused and tri_t::is_bool_valued))
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
				return ((std::string) this->getStorage<str_depth>()) + "\n" + this->to_string_rek<str_depth - 1>();
		}

	public:
		friend std::ostream &operator<<(std::ostream &os, NodeStorage &storage) {
			os << (std::string) storage;
			return os;
		}
	};*/


}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_LEVELNODESTORAGE_HPP
