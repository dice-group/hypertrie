#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include <compare>

#include "Dice/hypertrie/internal/node_based/interface/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/raw/node/NodeContainer.hpp"
#include "Dice/hypertrie/internal/node_based/raw/node/TensorHash.hpp"
#include "Dice/hypertrie/internal/node_based/raw/storage/NodeStorage.hpp"
#include "Dice/hypertrie/internal/node_based/raw/storage/RekNodeModification.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"

#include <Dice/hypertrie/internal/node_based/raw/iterator/IterationNodeContainer.hpp>
#include <itertools.hpp>

namespace hypertrie::internal::node_based::raw {

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
		using DiagonalPositions = typename tri::template DiagonalPositions<depth>;

		using NodeStorage_t = NodeStorage<max_depth, tri>;

	public:
		NodeStorage_t storage{};

		/**
		 *
		 * @tparam depth
		 * @param nodec
		 * @param key
		 * @return old value
		 */
		template<size_t depth>
		void incRefCount(NodeContainer<depth, tri> &nodec) {
			if constexpr (depth == 1 and tri::is_bool_valued and tri::is_lsb_unused)
				return;// there is no real node to be counted
			else {
				assert(nodec.ref_count() > 0);
				nodec.ref_count()++;
			}
		}
		template<size_t depth>
		void decrRefCount(NodeContainer<depth, tri> &nodec) {
			if constexpr (depth == 1 and tri::is_bool_valued and tri::is_lsb_unused)
				return;
			else {
				RekNodeModification<max_depth, depth, tri> update{this->storage, nodec};
				update.apply_decrement_ref_count();
			}
		}

		/**
		 *
		 * @tparam depth
		 * @param nodec
		 * @param key
		 * @return old value
		 */
		template<size_t depth>
		auto set(NodeContainer<depth, tri> &nodec, const RawKey<depth> &key, value_type value) -> value_type {
			const value_type old_value = get(nodec, key);
			if (value == old_value)
				return value;
			RekNodeModification<max_depth, depth, tri> update{this->storage, nodec};
			update.apply_update(key, value, old_value);
			return old_value;
		}

		/**
		 * the keys are required to not be in the hypertrie yet!
		 * @tparam depth
		 * @param keys
		 * @return
		 */
		template<size_t depth>
		void bulk_insert(NodeContainer<depth, tri> &nodec, std::vector<RawKey<depth>> keys) {
			RekNodeModification<max_depth, depth, tri> update{this->storage, nodec};
			update.apply_update(std::move(keys));
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
		inline auto getChild(UncompressedNodeContainer<depth, tri> &nodec, size_t pos, key_part_type key_part)
		-> std::conditional_t<(depth > 1), NodeContainer<depth - 1, tri>, value_type> {
			assert(pos < depth);
			if (nodec.empty())
				return {};
			else {
				auto child = nodec.getChildHashOrValue(pos, key_part);
				if constexpr (depth == 2 and tri::is_lsb_unused and tri::is_bool_valued) {
					if (child.isCompressed()) {
						return CompressedNodeContainer<depth - 1, tri>{child, {}};
					} else {
						return storage.template getUncompressedNode<depth - 1>(child.getTaggedNodeHash());
					}
				} else if constexpr (depth > 1) {
					if (not child.empty())
						return storage.template getNode<depth - 1>(child);
					else
						return {};
				} else {
					return child;
				}
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
		auto get(const NodeContainer<depth, tri> &nodec, RawKey<depth> key) -> value_type {
			static constexpr const auto subkey = &tri::template subkey<depth>;
			if (nodec.empty())
				return {};
			else if (nodec.isCompressed()) {
				auto *node = nodec.compressed_node();
				if (node->key() == key) {
					if constexpr (tri::is_bool_valued) return true;
					else
						return node->value();
				} else
					return {};
			} else {
				UncompressedNodeContainer<depth, tri> nc = nodec.uncompressed();
				if constexpr (depth > 1) {
					// TODO: implement minCardPos();
					auto pos = 0;//minCardPos();
					NodeContainer<depth - 1, tri> child = this->template getChild<depth>(nc, pos, key[pos]);
					if (not child.empty()) {
						if constexpr (depth == 2 and tri::is_lsb_unused and tri::is_bool_valued) {
							if (child.isCompressed()) // here, we have an KeyPart stored instead of a hash
								return key[1] == child.hash().getKeyPart();
						}
						return get<depth - 1>(child, subkey(key, pos));
					} else {
						return {};// false, 0, 0.0
					}
				} else {

					return this->template getChild<1>(nc, 0UL, key[0]);
				}
			}
		}


		/**
		 * Returns a pair of a node container and a boolean which states if the pointed node is managed (true) or if it is unmanaged (false) and MUST be deleted by the user manually.
		 * Only compressed nodes can be managed.
		 * if fixed_depth == depth, just a scalar is returned
		 * @tparam depth depth of the node container
		 * @tparam fixed_keyparts number of fixed key_parts in the slice key
		 * @param nodec a container with a node.
		 * @param raw_slice_key the slice key
		 * @return see above
		 */
		template<size_t depth, size_t fixed_keyparts>
		auto slice(const NodeContainer<depth, tri> &nodec, RawSliceKey<fixed_keyparts> raw_slice_key)
		-> std::conditional_t<(depth > fixed_keyparts), std::pair<NodeContainer<depth - fixed_keyparts, tri>,bool>, value_type> {
			return slice_rek(nodec, raw_slice_key);
		}

	private:
		template<size_t current_depth, size_t fixed_keyparts, size_t slice_offset = 0>
		auto slice_rek(const NodeContainer<current_depth, tri> &nodec, const RawSliceKey<fixed_keyparts> &raw_slice_key)
				-> std::conditional_t<(current_depth - fixed_keyparts + slice_offset > 0),
				        std::pair<NodeContainer<current_depth - fixed_keyparts + slice_offset, tri>, bool>,
				                value_type> {

			constexpr static const size_t result_depth = current_depth - fixed_keyparts + slice_offset;
			if constexpr (slice_offset >= fixed_keyparts) // break condition
				return {nodec, true};
			else { // recursion
				if (nodec.isUncompressed()){
					auto uncompressed_nodec = nodec.uncompressed();
					auto child = this->template getChild(uncompressed_nodec, raw_slice_key[slice_offset].pos - slice_offset, raw_slice_key[slice_offset].key_part);
					if (child.empty())
						return {};
					else
						return slice_rek<current_depth - 1, fixed_keyparts, slice_offset +1> (child, raw_slice_key);

				} else { // nodec.isCompressed()
					// check if key-parts match the slice key
					for (auto slice_key_i : iter::range(slice_offset, fixed_keyparts))
						if (nodec.compressed_node()->key()[raw_slice_key[slice_key_i].pos - slice_offset] != raw_slice_key[slice_key_i].key_part)
							return {};
					// when this point is reached, the compressed node is a match
					if constexpr (result_depth > 0){ // return key/value
						CompressedNodeContainer<result_depth, tri> nc;
						if constexpr (tri::is_bool_valued and tri::is_lsb_unused and (result_depth == 1)){
							size_t slice_pos = 0;
							for (auto nodec_pos : iter::range(current_depth)){
								if (slice_pos + slice_offset < fixed_keyparts and nodec_pos == raw_slice_key[slice_pos + slice_offset].pos - slice_offset){
									++slice_pos;
									continue;
								} else {
									nc.hash() = TaggedTensorHash(nodec.compressed_node()->key()[nodec_pos]);
									return {nc,false};
								}
							}
							assert(false);
							return {};

						} else {
							nc.node() = new CompressedNode<result_depth, tri>();
							size_t slice_pos = 0;
							size_t result_pos = 0;
							for (auto nodec_pos : iter::range(current_depth)){
								if (slice_pos + slice_offset < fixed_keyparts and nodec_pos == raw_slice_key[slice_pos + slice_offset].pos - slice_offset){
									++slice_pos;
									continue;
								} else {
									nc.compressed_node()->key()[result_pos++] = nodec.compressed_node()->key()[nodec_pos];
								}
							}
							if constexpr (not tri::is_bool_valued)
								nc.compressed_node()->value() = nodec.compressed_node()->value();
							nc.hash() = TensorHash().addFirstEntry(nc.compressed_node()->key(), nc.compressed_node()->value());
							return {nc,false};
						}
					} else { // return just the mapped value
						if constexpr (tri::is_bool_valued)
							return true;
						else
							return nodec.compressed_node()->value();
					}
				}
			}
		}

	public:
		template<size_t depth, size_t fixed_keyparts, size_t result_depth = depth - fixed_keyparts, typename ccn = std::nullptr_t >
		auto diagonal_slice(const NodeContainer<depth, tri> &nodec,
							const DiagonalPositions<depth> &diagonal_positions, const key_part_type &key_part,
							ccn contextless_compressed_result = nullptr)
		-> std::conditional_t<(result_depth > 0), IterationNodeContainer<result_depth, tri>, value_type> {
			return diagonal_slice_rek<depth, fixed_keyparts>(nodec, diagonal_positions, key_part, contextless_compressed_result);
		}


	private:
		template<size_t current_depth, size_t fixed_keyparts, size_t offset = 0, size_t result_depth = current_depth - fixed_keyparts + offset, typename ccn = std::nullptr_t >
		auto diagonal_slice_rek(const NodeContainer<current_depth, tri> &nodec, const DiagonalPositions<current_depth + offset> &diagonal_positions, const key_part_type &key_part, ccn contextless_compressed_result = nullptr,
								size_t key_pos = 0)
		-> std::conditional_t<(result_depth > 0), IterationNodeContainer<result_depth, tri>, value_type>  {

			if constexpr (offset >= fixed_keyparts) // break condition
				return {nodec, true};
			else { // recursion
				if (nodec.isUncompressed()){
					auto uncompressed_nodec = nodec.uncompressed();
					while(not diagonal_positions[key_pos])
						++key_pos;
					auto child = this->template getChild(uncompressed_nodec, key_pos - offset, key_part);
					if constexpr (current_depth == 1)
						return child;
					else {
					if (child.empty())
						return {};
					else
						return diagonal_slice_rek<current_depth - 1, fixed_keyparts, offset +1> (child, diagonal_positions, key_part, contextless_compressed_result, key_pos);
					}

				} else { // nodec.isCompressed()
					// check if key-parts match the slice key
					auto key_pos_copy = key_pos;
					while (key_pos_copy < fixed_keyparts) {
						if (diagonal_positions[key_pos_copy])
							if (nodec.compressed_node()->key()[key_pos_copy - offset] != key_part)
								return {};

						++key_pos_copy;
					}
					// when this point is reached, the compressed node is a match
					if constexpr (result_depth > 0){ // return key/value
						CompressedNodeContainer<result_depth, tri> nc;
						if constexpr (tri::is_bool_valued and tri::is_lsb_unused and (result_depth == 1)){
							while (key_pos < fixed_keyparts) {
								if (not diagonal_positions[key_pos]){
									CompressedNodeContainer<result_depth, tri> nc;
									nc.hash() = TaggedTensorHash(nodec.compressed_node()->key()[key_pos - offset]);
									return {nc,false};
								}
								++key_pos;
							}
							assert(false);
							return {};

						} else {
							if (contextless_compressed_result == nullptr)
								nc.node() = new CompressedNode<result_depth, tri>();
							else
								nc.node() = contextless_compressed_result;
							size_t result_pos = 0;
							while (key_pos < fixed_keyparts) {
								if (not diagonal_positions[key_pos])
									nc.compressed_node()->key()[result_pos++] = nodec.compressed_node()->key()[key_pos - offset];
								++key_pos;
							}
							if constexpr (not tri::is_bool_valued)
								nc.compressed_node()->value() = nodec.compressed_node()->value();
							nc.hash() = TensorHash().addFirstEntry(nc.compressed_node()->key(), nc.compressed_node()->value());
							return {nc,false};
						}
					} else { // return just the mapped value
						if constexpr (tri::is_bool_valued)
							return true;
						else
							return nodec.compressed_node()->value();
					}
				}
			}
		}

	public:


		template<size_t depth>
		std::size_t size(NodeContainer<depth, tri> &nodec) {
			if (nodec.empty())
				return 0;
			else if (nodec.isCompressed())
				return 1;
			else
				nodec.uncompressed_node()->size();
		}
	};
}// namespace hypertrie::internal::node_based

#endif//HYPERTRIE_NODECONTEXT_HPP
