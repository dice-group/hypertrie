#ifndef HYPERTRIE_RAWITERATOR_HPP
#define HYPERTRIE_RAWITERATOR_HPP

#include "Dice/hypertrie/internal/ConfigHypertrieDepthLimit.hpp"
#include "Dice/hypertrie/internal/raw/storage/NodeContext.hpp"
#include "Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp"
namespace hypertrie::internal::node_based::raw {

	template<HypertrieInternalTrait tri_t>
	class base_iterator {
		using tri = tri_t;
		using tr = typename tri::tr;
	protected:
		template<size_t depth_>
		using UncomressedChildren = typename UncompressedNode<depth_, tri>::ChildrenType;
		template<size_t depth_>
		using UncomressedChildrenIterator = typename UncomressedChildren<depth_>::const_iterator;
		using Entry = typename tr::IteratorEntry;
		using Key = typename tr::Key;
		using value_t = typename tri::value_type;

		mutable void *node_context_;

		mutable void *nodec_;

		util::IntegralTemplatedTuple<UncomressedChildrenIterator, 1, hypertrie_depth_limit - 1> iters;

		util::IntegralTemplatedTuple<UncomressedChildrenIterator, 1, hypertrie_depth_limit - 1> ends;

		Entry entry;

		NodeCompression compression_;

		bool ended_;

		template<size_t child_depth>
		auto &getIter() { return iters.template get<child_depth + 1>(); }
		template<size_t child_depth>
		auto &getEnd() { return ends.template get<child_depth + 1>(); }

		Key &key() noexcept {
			if constexpr (tri::is_bool_valued) return entry;
			else
				return entry.first;
		}

		value_t &val() noexcept { if (not tri::is_bool_valued) return entry.second; }

	public:
		base_iterator(void *nodeContext, void *nodec, NodeCompression compression, bool ended) : node_context_(nodeContext), nodec_(nodec), compression_(compression), ended_(ended) {}
	};

	template<size_t depth_t,
			 HypertrieInternalTrait tri_t,
			typename = typename std::enable_if_t<(depth_t >= 1)>>
	class iterator : public base_iterator<tri_t> {
	public:

		/// public definitions
		static constexpr const size_t depth = depth_t;
		using tri = tri_t;
		using tr = typename tri::tr;
	private:
		using key_part_type = typename tri::key_part_type;

		using RawKey = typename tri::template RawKey<depth>;
		using Key = typename tr::Key;

		using Entry = typename tr::IteratorEntry;

		auto *nodec() const {
			return reinterpret_cast<NodeContainer<depth, tri>*>(this->nodec_);
		}

		auto *node_context() const {
			return reinterpret_cast<NodeContext<depth, tri>*>(this->node_context_);
		}

	public:
		/// iterator definitions
		using self_type = iterator;
		using value_type = Entry;

		iterator(NodeContainer<depth, tri> &nodec, NodeContext<hypertrie_depth_limit, tri> &node_context)
			: base_iterator<tri>(&node_context, &nodec, (NodeCompression)nodec.isCompressed(), nodec.empty()) {
			if (not this->ended_){
				this->key().resize(depth);
				this->init_rek();
			}
		}

		template<size_t any_depth>
		iterator(NodeContainer<depth, tri> &nodec, NodeContext<any_depth, tri> &node_context)
				: base_iterator<tri>(&node_context, &nodec, (NodeCompression)nodec.isCompressed(), nodec.empty()) {
			static_assert(any_depth < hypertrie_depth_limit);
			if (not this->ended_){
				this->key().resize(depth);
				this->init_rek();
			}
		}

		inline self_type &operator++() {
			inc_rek();
			return *this;
		}

		inline const value_type &operator*() const { return this->entry; }

		inline const value_type* operator-> () const{ return &this->entry; }

		inline operator bool() const { return not this->ended_; }

		/*
		 * Static Interface for usage with function pointers
		 */
		static void inc(void *it_ptr) {
			auto &it = *static_cast<iterator *>(it_ptr);
			++it;
		}

		static const value_type &value(void const *it_ptr) {
			auto &it = *static_cast<iterator const *>(it_ptr);
			return *it;
		}

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

			// child is compressed
			if constexpr (tri::is_bool_valued and tri::is_lsb_unused and node_depth == 1) {
				auto &iter = this->template getIter<node_depth>();
				auto hash = iter->second;
				// set key_part stored in TaggedTensorHash
				this->key()[depth - 1] = hash.getKeyPart();
			} else {
				// set rest of the key (and value) with the data in the compressed node
				auto *node = [&]() {
					if constexpr (node_depth == depth) {
						assert(this->compression_ == NodeCompression::compressed);
						return this->nodec()->compressed_node();
					} else {
						auto &iter = this->template getIter<node_depth>();
						auto hash = iter->second;
						return this->node_context()->storage.template getCompressedNode<node_depth>(hash).compressed_node();
					}
				}();
				const auto &compr_key = node->key();
				// copy the key of the compressed child node to the iterator key
				std::copy(compr_key.cbegin(), compr_key.cend(), std::next(this->key().begin(), depth - (node_depth)));
				if constexpr (not tri::is_bool_valued)
					this->val() = node->value();
			}
		}

		/**
		 * Write the UncompressedNode to the key (and evtl. value) currently pointed at by std::get<child_depth>(iters)
		 * @tparam child_depth the node depth of the UncompressedNode to be written
		 */
		template <size_t child_depth>
		inline void writeUncompressed() noexcept {
			auto &iter = this->template getIter<child_depth>();
			// set the key at the corresponding position
			auto x = [&]() {
			  if constexpr (child_depth == 0 and tri::is_bool_valued)
				  return *iter;
			  else
				  return iter->first;
			}();


			this->key()[depth - (child_depth + 1)] = x;

			// write value
			if constexpr (child_depth == 0 and not tri::is_bool_valued)
				this->val() = iter->second;
		}

		/**
		 * Initialize for every level a iters and ends, as well as Entry (key; and, if not tri::is_bool_valued, value).
		 * Must only be called if current_nodec is not empty.
		 * @tparam current_depth  the depth of the node that is resolved in this step.
		 */
		template<pos_type current_depth = depth,
				 typename = std::enable_if_t<(current_depth <= depth and current_depth >= 1)>>
		inline void init_rek() {
			if constexpr (current_depth == depth)
				if (this->compression_ == NodeCompression::compressed){
					writeCompressed<current_depth>();
					return;
				}

			constexpr const size_t child_depth = current_depth - 1;
			// get the UncompressedNodeContainer
			auto current_nodec = [&]() {
				if constexpr (current_depth == depth) {
					return *this->nodec();
				} else {
					auto &parent_iter = this->template getIter<current_depth>();
					const auto hash = parent_iter->second;

					return this->node_context()->storage.template getUncompressedNode<current_depth>(hash);
				}
			}();

			// set the iterator for child elements
			auto &iter = this->template getIter<child_depth>();
			iter = current_nodec.uncompressed_node()->edges(0).cbegin();

			auto &end = this->template getEnd<child_depth>();
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
			if constexpr (current_depth == depth)
				if (this->compression_ == NodeCompression::compressed){
					this->ended_ = true;
					return true;
				}

			constexpr const size_t child_depth = current_depth - 1;
			auto &iter = this->template getIter<child_depth>();
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
				auto &end = this->template getEnd<child_depth>();
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
					if constexpr(current_depth == depth) this->ended_ = true;
					return true;
				}
			} else {
				return false;
			}
		}
	};
};// namespace hypertrie::internal::node_based::raw

#endif//HYPERTRIE_RAWITERATOR_HPP
