#ifndef HYPERTRIE_DIAGONAL_HPP
#define HYPERTRIE_DIAGONAL_HPP

#include "Dice/hypertrie/internal/ConfigHypertrieDepthLimit.hpp"
#include "Dice/hypertrie/internal/raw/storage/NodeContext.hpp"
#include "Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp"
#include "Dice/hypertrie/internal/raw/iterator/IterationNodeContainer.hpp"
#include <utility>
namespace hypertrie::internal::raw {

	template<size_t diag_depth, size_t depth, NodeCompression compression, HypertrieInternalTrait tri_t>
	class HashDiagonal;

	template<size_t diag_depth, size_t depth, HypertrieInternalTrait tri_t>
	class HashDiagonal<diag_depth, depth, NodeCompression::uncompressed, tri_t> {
//			static_assert(diag_depth <= depth);
//			stacic_assert(diag_depth > 0);

		static constexpr const size_t result_depth = depth - diag_depth;

		public:
		using tri = tri_t;
		using tr = typename tri::tr;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		template<typename key, typename value>
		using map_type = typename tri::template map_type<key, value>;
		template<typename key>
		using set_type = typename tri::template set_type<key>;

		using DiagonalPositions = typename tri::template DiagonalPositions<depth>;
		using SubDiagonalPositions = typename tri::template DiagonalPositions<(depth>1) ? depth - 1 : 0>;

	private:
		using child_iterator = typename UncompressedNode<depth, tri>::ChildrenType::const_iterator;

		using IterationNodeContainer_t = IterationNodeContainer<result_depth, tri>;
		using IterValue = std::conditional_t<
				(result_depth > 0),
				IterationNodeContainer_t,
				value_type>;

	private:
		UncompressedNodeContainer<depth, tri> *nodec_;
		NodeContext<hypertrie_depth_limit - 1, tri> *node_context_;
		DiagonalPositions diag_poss_;
		SubDiagonalPositions sub_diag_poss_;

		child_iterator iter_;
		child_iterator end_;
		CompressedNode<(result_depth>0)?result_depth :1, typename tri::with_std_alloctor> internal_compressed_node;
		IterValue value_;

	public:
		explicit HashDiagonal(UncompressedNodeContainer<depth, tri> &nodec, DiagonalPositions diag_poss, NodeContext<hypertrie_depth_limit - 1, tri> &node_context) : nodec_{&nodec}, node_context_(&node_context), diag_poss_{diag_poss} {
			assert(diag_depth == diag_poss_.count());
		}

		template<size_t any_depth>
		explicit HashDiagonal(UncompressedNodeContainer<depth, tri> &nodec, DiagonalPositions diag_poss, NodeContext<any_depth, tri> &node_context)
			: HashDiagonal{nodec, diag_poss, *reinterpret_cast<NodeContext<hypertrie_depth_limit - 1, tri> *>(&node_context)}{
			static_assert(any_depth >= depth);
		}

		HashDiagonal &begin() {
			if constexpr (depth > 1) {
				const size_t min_card_pos = [&]() {
					if constexpr (diag_depth > 1)
						return nodec_->uncompressed_node()->minCardPos(diag_poss_);
					else // diag_depth == 1
						for (const size_t pos : iter::range(diag_poss_.size()))
							if (diag_poss_[pos])
								return pos;
					assert(false); return 0UL;
				}();
				// generate the sub_diag_poss_ diagonal positions mask to apply the diagonal to the values of iter_
				if constexpr (diag_depth > 1) {
					bool skipped = false;
					for(auto pos : iter::range(diag_poss_.size()))
						if(pos == min_card_pos)
							skipped = true;
						else
							sub_diag_poss_[pos - skipped] = diag_poss_[pos];

				}

				const auto &min_dim_edges = nodec_->uncompressed_node()->edges(min_card_pos);
				iter_ = min_dim_edges.begin();
				end_ = min_dim_edges.end();
				if (not ended()) {
					if constexpr (diag_depth > 1) {
						if (not retrieveSubDiagonalValue())
							++(*this);
					} else {
						if constexpr (result_depth == 0) {
							if constexpr (tri::is_bool_valued)
								value_ = true;
							else
								value_ = iter_->second;
						} else {
							TensorHash next_result_hash{};
							if constexpr (result_depth == 1 and tri::is_lsb_unused) {
								if (iter_->second.isCompressed()){
									value_ = IterationNodeContainer_t::make_managed_tri_alloc(iter_->second);
									return *this;
								} else {
									next_result_hash = iter_->second.hash();
								}
							} else {
								next_result_hash = iter_->second;
							}
							NodeContainer<result_depth, tri> child_node_container = node_context_->storage.template getNode<result_depth>(next_result_hash);
							value_ = IterationNodeContainer_t::make_managed_tri_alloc(child_node_container);
						}
					}
				}
			} else { // depth == 1
				iter_ = nodec_->uncompressed_node()->edges(0).begin();
				end_ = nodec_->uncompressed_node()->edges(0).end();
				if constexpr (tri::is_bool_valued)
					value_ = not ended();
			}
			return *this;
		}

		bool end() const {
			return false;
		}

		auto operator[](key_part_type key_part){
			if constexpr (result_depth > 0)
				value_ = node_context_->template diagonal_slice<depth, diag_depth>(*nodec_, diag_poss_, key_part, &internal_compressed_node);
			else
				value_ = node_context_->template diagonal_slice<depth, diag_depth>(*nodec_, diag_poss_, key_part);
			return value_;
		}
		
	private:
		bool retrieveSubDiagonalValue() {
			static_assert(diag_depth > 1);
			const key_part_type key_part = iter_->first;
			if constexpr (not (tri::is_lsb_unused and depth - 1 == 1)){
				NodeContainer<depth - 1, tri> child_node = node_context_->storage.template getNode<depth - 1>(iter_->second);
				if constexpr (result_depth > 0)
					value_ = node_context_->template diagonal_slice<depth - 1, diag_depth - 1>(child_node, sub_diag_poss_, key_part, &internal_compressed_node);
				else
					value_ = node_context_->template diagonal_slice<depth - 1, diag_depth - 1>(child_node, sub_diag_poss_, key_part);
				if constexpr(result_depth == 0)
					return value_ != value_type{};
				else
					return not value_.empty();
			} else {
				static_assert(depth == 2);
				if (iter_->second.isCompressed()){
					value_ = iter_->second.getKeyPart() == key_part;
					return value_;
				} else {
					NodeContainer<depth - 1, tri> child_node = node_context_->storage.template getNode<depth - 1>(iter_->second.hash());
					value_ = node_context_->template diagonal_slice<depth - 1, diag_depth - 1>(child_node, sub_diag_poss_, key_part);
					return value_;
				}
			}
		} 
	public:
		bool find(key_part_type key_part) {
			this->operator[](key_part);
			if constexpr(result_depth == 0)
				return value_ != value_type{};
			else
				return not value_.empty();
		}


		const key_part_type &currentKeyPart() const {
			if constexpr (depth == 1 and tri::is_bool_valued)
				return *iter_;
			else
				return iter_->first;
		}

		auto currentValue() const {
			return value_;
		}

		std::pair<key_part_type,IterValue> operator*() const{
			return std::make_pair(currentKeyPart(), currentValue());
		}

		HashDiagonal& operator++(){
			if constexpr (diag_depth > 1) {
				assert(not empty());
				do {
					++iter_;
				} while (not ended() and not retrieveSubDiagonalValue());
			} else {
				++iter_;
				if (not ended()) {
					if constexpr (result_depth == 0) {
						if constexpr (tri::is_bool_valued)
							value_ = true;
						else
							value_ = iter_->second;
					} else {
						TensorHash next_result_hash{};
						if constexpr (result_depth == 1 and tri::is_lsb_unused) {
							if (iter_->second.isCompressed()){
								auto identifier = iter_->second;
								value_ = IterationNodeContainer_t::make_managed_tri_alloc(identifier);
								return *this;
							} else {
								next_result_hash = iter_->second.hash();
							}
						} else {
							next_result_hash = iter_->second;
						}
						NodeContainer<result_depth, tri> child_node_container = node_context_->storage.template getNode<result_depth>(next_result_hash);
						value_ = IterationNodeContainer_t::make_managed_tri_alloc(child_node_container);
					}
				}
			}
			return *this;
		}

		operator bool() const noexcept {
			return iter_ != end_;
		}

		bool ended() const noexcept {
			return not bool(*this);
		}

		bool empty() const noexcept {
			return nodec_->empty();
		}

		size_t size() const noexcept {

			if (not empty()) {
				if constexpr (depth > 1) {

					const auto min_card_pos = nodec_->uncompressed_node()->minCardPos(diag_poss_);
					return nodec_->uncompressed_node()->edges(min_card_pos).size();

				} else {
					return nodec_->uncompressed_node()->size();
				}
			} else {
				return 0UL;
			}
		}
	};

	template<size_t diag_depth, size_t depth, HypertrieInternalTrait tri_t>
	class HashDiagonal<diag_depth, depth, NodeCompression::compressed, tri_t> {
		static constexpr const size_t result_depth = depth - diag_depth;

	public:
		using tri = tri_t;
		using tr = typename tri::tr;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		template<typename key, typename value>
		using map_type = typename tri::template map_type<key, value>;
		template<typename key>
		using set_type = typename tri::template set_type<key>;

		using DiagonalPositions = typename tri::template DiagonalPositions<depth>;

	private:
		using IterationNodeContainer_t = IterationNodeContainer<result_depth, tri>;

		using IterValue = std::conditional_t<
				(result_depth > 0),
				IterationNodeContainer_t,
				value_type>;

	private:
		CompressedNodeContainer<depth, tri> *nodec_;
		DiagonalPositions diag_poss_;

		// TODO: changed to with_std_allocator
		CompressedNode<(result_depth>0)?result_depth :1, typename tri::with_std_allocator> internal_compressed_node;
		std::pair<key_part_type, IterValue> value_;
		bool ended_ = true;
		bool contains;

	public:
		HashDiagonal(CompressedNodeContainer<depth, tri> &nodec, DiagonalPositions diag_poss)
			: nodec_{&nodec}, diag_poss_(diag_poss) {
			size_t any_diag_pos = 0;
			for (; any_diag_pos < depth; ++any_diag_pos)
				if (diag_poss_[any_diag_pos])
					break;
			value_.first = [&]() {// key_part
				if constexpr (depth == 1 and diag_depth == 1 and tri::is_bool_valued and tri::is_lsb_unused)
					return nodec_->hash().getKeyPart();
				else
					return nodec_->compressed_node()->key()[any_diag_pos];
			}();

			if constexpr (diag_depth == 1) {
				contains = true;
				if constexpr (result_depth > 0) {
					size_t res_pos = 0;
					for (auto pos : iter::range(depth))
						if (pos != any_diag_pos) {
							if constexpr (tri::is_bool_valued and tri::is_lsb_unused and result_depth == 1) {
								auto identifier = TaggedTensorHash<tri>(nodec_->compressed_node()->key()[pos]);
								value_.second = IterationNodeContainer_t::make_managed_tri_alloc(identifier);
								return;
							}
							internal_compressed_node.key()[res_pos++] = nodec_->compressed_node()->key()[pos];
						}
					auto identifier = TensorHash().addFirstEntry(internal_compressed_node.key(), internal_compressed_node.value());
					value_.second = IterationNodeContainer_t::make_managed_std_alloc(identifier, std::addressof(internal_compressed_node));
					if constexpr (not tri::is_bool_valued)
						internal_compressed_node.value() = nodec_->compressed_node()->value();
				} else {
					value_.second = nodec_->compressed_node()->value();
				}


			} else {

				contains = [&]() {
					// set key
					const auto &key = nodec_->compressed_node()->key();
					size_t res_pos = 0;
					for (auto pos : iter::range(depth))
						if (this->diag_poss_[pos]) {
							if (key[pos] != value_.first) {
								return false;// not contained
							}
						} else {
							if constexpr (result_depth != 0) {
								if constexpr (tri::is_bool_valued and tri::is_lsb_unused and result_depth == 1) {
									value_.second.identifier() = TaggedTensorHash<tri>(key[pos]);
								} else {
									internal_compressed_node.key()[res_pos++] = key[pos];
								}
							}
						}
					// set value
					if constexpr (not tri::is_bool_valued)
						internal_compressed_node.value() = nodec_->compressed_node()->value();

					if constexpr (result_depth > 0) {
						if constexpr (not(tri::is_bool_valued and tri::is_lsb_unused and result_depth == 1)) {
							auto identifier = TensorHash().addFirstEntry(internal_compressed_node.key(), internal_compressed_node.value());
							value_.second = IterationNodeContainer_t::make_managed_std_alloc(identifier, std::addressof(internal_compressed_node));
						}
					} else {
						value_.second = nodec_->compressed_node()->value();
					}
					return true;
				}();
			}
		}

		HashDiagonal &begin() {
			if (contains)
				ended_ = false;
			return *this;
		}

		bool end() const {
			return false;
		}

		IterValue operator[](key_part_type key_part){
			if (value_.first == key_part)
				return value_.second;
			else
				return {};
		}

	public:
		bool find(key_part_type key_part) {
			return  (key_part == value_.first);
		}


		const key_part_type &currentKeyPart() const {
			return value_.first;
		}

		auto currentValue() const {
			return value_.second;
		}

		const std::pair<key_part_type,IterValue> &operator*() const{
			return value_;
		}

		HashDiagonal& operator++(){
			ended_ = true;
			return *this;
		}

		operator bool() const noexcept {
			return not ended_;
		}

		bool ended() const noexcept {
			return ended_;
		}

		bool empty() const noexcept {
			return not contains;
		}

		size_t size() const noexcept {

			return size_t(contains);
		}
	};

};

#endif//HYPERTRIE_DIAGONAL_HPP
