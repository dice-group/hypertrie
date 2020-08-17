#ifndef HYPERTRIE_ITERATOR_HPP
#define HYPERTRIE_ITERATOR_HPP

#include "Dice/hypertrie/internal/node_based/raw/storage/NodeContext.hpp"

namespace hypertrie::internal::node_based::raw {

	template<size_t depth,
			 NodeCompression compression,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			 typename = typename std::enable_if_t<(depth >= 1)>>
	class iterator;

	template<size_t depth_t,
			 HypertrieInternalTrait tri_t>
	class iterator<depth_t, NodeCompression::uncompressed, tri_t, std::enable_if_t<(depth_t >= 1)>> {
	public:
		using tri = tri_t;
		using tr = typename tri::tr;
		static constexpr const size_t depth = depth_t;
		/// public definitions
		using key_part_type = typename tri::key_part_type;
		using value_t = typename tri::value_type;

		using RawKey = typename tri::template RawKey<depth>;
		using Key = typename tr::Key;

	private:
		NodeContext<depth, tri> *node_context_;
		UncompressedNodeContainer<depth, tri> *nodec_;
		template<size_t depth_>
		using UncomressedChildren = typename UncompressedNode<depth_, tri>::ChildrenType;
		template<size_t depth_>
		using UncomressedChildrenIterator = typename UncomressedChildren<depth_>::const_iterator;
		template<size_t depth_>
		using ComressedChildren = CompressedNode<depth_ - 1, tri>;

		util::CountDownNTuple<UncomressedChildrenIterator, depth> iters;

		util::CountDownNTuple<UncomressedChildrenIterator, depth> ends;

		template<size_t child_depth>
		auto &getIter() { return std::get<child_depth>(iters); }

		template<size_t child_depth>
		auto &getEnd() { return std::get<child_depth>(ends); }

		using Entry = typename tr::IteratorEntry;
		Entry entry;
		bool ended_;

		Key &key() noexcept {
			if constexpr (tri::is_bool_valued) return entry;
			else
				return entry.first;
		}

		value_t &value() noexcept { if (not tri::is_bool_valued) return entry.second; else return true;}

	public:
		using self_type = iterator;
		using value_type = Entry;

	public:
		iterator(UncompressedNodeContainer<depth, tri> &nodec, NodeContext<depth, tri> &node_context)
			: node_context_(&node_context),
			  nodec_(&nodec),
			  ended_(nodec_->empty()) {
			if (not ended_){
				this->key().resize(depth);
				this->init_rek();
			}
		}

		inline self_type &operator++() {
			inc_rek();
			return *this;
		}

		static void inc(void *it_ptr) {
			auto &it = *static_cast<iterator *>(it_ptr);
			++it;
		}

		inline const value_type &operator*() const { return entry; }

		static const value_type &value(void const *it_ptr) {
			auto &it = *static_cast<iterator const *>(it_ptr);
			return *it;
		}

		inline operator bool() const { return not ended_; }

		static bool ended(void const *it_ptr) {
			auto &it = *static_cast<iterator const *>(it_ptr);
			return it.ended_;
		}

	private:
		/**
		 * Write the CompressedNode to the key and value currently pointed at by std::get<node_depth>(iters)
		 * @tparam node_depth the node depth of the CompressedNode to be written
		 */
		template <size_t node_depth>
		inline void writeCompressed() noexcept {
			auto &iter = std::get<node_depth>(iters);
			auto hash = iter->second;
			// child is compressed
			if constexpr (tri::is_bool_valued and tri::is_lsb_unused and node_depth == 1) {
				// set key_part stored in TaggedTensorHash
				key()[depth - 1] = hash.getKeyPart();
			} else {
				// set rest of the key (and value) with the data in the compressed node
				auto compressed_child_node = node_context_->storage.template getCompressedNode<node_depth>(hash);
				auto *child_node = compressed_child_node.node();
				const auto &compr_key = child_node->key();
				// copy the key of the compressed child node to the iterator key
				std::copy(compr_key.cbegin(), compr_key.cend(), std::next(key().begin(), depth - (node_depth)));
				if constexpr (not tri::is_bool_valued)
					value() = child_node->value();
			}
		}

		/**
		 * Write the UncompressedNode to the key (and evtl. value) currently pointed at by std::get<child_depth>(iters)
		 * @tparam child_depth the node depth of the UncompressedNode to be written
		 */
		template <size_t child_depth>
		inline void writeUncompressed() noexcept {
			auto &iter = getIter<child_depth>();
			// set the key at the corresponding position
			auto x = [&]() {
			  if constexpr (child_depth == 0 and tri::is_bool_valued)
				  return *iter;
			  else
				  return iter->first;
			}();


			key()[depth - (child_depth + 1)] = x;

			// write value
			if constexpr (child_depth == 0 and not tri::is_bool_valued)
				value() = iter->second;
		}

		/**
		 * Initialize for every level a iters and ends, as well as Entry (key; and, if not tri::is_bool_valued, value).
		 * Must only be called if current_nodec is not empty.
		 * @tparam current_depth  the depth of the node that is resolved in this step.
		 */
		template<pos_type current_depth = depth,
				 typename = std::enable_if_t<(current_depth <= depth and current_depth >= 1)>>
		inline void init_rek() {
			constexpr const size_t child_depth = current_depth - 1;
			// get the UncompressedNodeContainer
			auto current_nodec = [&]() {
				if constexpr (current_depth == depth) {
					return *nodec_;
				} else {
					auto &parent_iter = getIter<current_depth>();
					const auto hash = parent_iter->second;

					return node_context_->storage.template getUncompressedNode<current_depth>(hash);
				}
			}();

			// set the iterator for child elements
			auto &iter = this->getIter<child_depth>();
			iter = current_nodec.uncompressed_node()->edges(0).cbegin();

			auto &end = this->getEnd<child_depth>();
			end = current_nodec.uncompressed_node()->edges(0).cend();

			writeUncompressed<child_depth>();

			// call recursively, if no leaf is reached (depth == 1 or child is compressed)
			if constexpr (current_depth > 1)  {
				if (iter->second.isUncompressed())
					// child is uncompressed, go on recursively
					this->init_rek<child_depth>();
				else {
					// child is compressed
					writeCompressed<child_depth>();
				}
			}
		}

		template<pos_type current_depth = depth,
				 typename = std::enable_if_t<(current_depth <= depth and current_depth >= 1)>>
		inline bool inc_rek() {
			constexpr const size_t child_depth = current_depth - 1;
			auto &iter = getIter<child_depth>();
			bool child_it_done = [&]() {
				if constexpr (current_depth == 1)
					return true;
				else
					return iter->second.isCompressed();
			}();
			// if the child is compressed there is no child_it we can forward
			if constexpr (current_depth > 1)
				if (not child_it_done)
					child_it_done = inc_rek<current_depth - 1>();

			// if the child is at its end
			if (child_it_done) {
				auto &end = getEnd<child_depth>();
				// forward this it
				iter = ++iter;
				if (iter != end) { // this it has values left
					writeUncompressed<child_depth>();

					if constexpr (current_depth > 1)  {
						if (iter->second.isUncompressed())
							// child is uncompressed, initialize it recursively
							init_rek<current_depth - 1>();
						else
							// child is compressed
							writeCompressed<child_depth>();
					}
					return false;
				} else {
					if constexpr(current_depth == depth) ended_ = true;
					return true;
				}
			} else {
				return false;
			}
		}
	};
};// namespace hypertrie::internal::node_based::raw

#endif//HYPERTRIE_ITERATOR_HPP
