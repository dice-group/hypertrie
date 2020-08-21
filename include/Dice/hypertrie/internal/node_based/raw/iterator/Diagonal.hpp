#ifndef HYPERTRIE_DIAGONAL_HPP
#define HYPERTRIE_DIAGONAL_HPP
#include <utility>
#include "Dice/hypertrie/internal/node_based/raw/storage/NodeContext.hpp"
#include "Dice/hypertrie/internal/node_based/ConfigHypertrieDepthLimit.hpp"
#include "Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp"
namespace hypertrie::internal::node_based::raw {

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

		using DiagonalPositions = std::array<bool, depth>;

	private:
		using child_iterator = typename UncompressedNode<depth, tri>::ChildrenType::const_iterator;

		using IterValue = std::conditional_t<
				(result_depth > 0),
				std::pair<NodeContainer<result_depth, tri>, bool>,
				value_type>;

	private:
		UncompressedNodeContainer<depth, tri> *nodec_;
		NodeContext<hypertrie_depth_limit, tri> node_context_;
		DiagonalPositions diag_poss_;
		DiagonalPositions sub_diag_poss_;

		child_iterator iter_;
		child_iterator end_;
		CompressedNode<result_depth, tri> internal_compressed_node;
		IterValue value_;

	public:
		explicit HashDiagonal(UncompressedNode<depth, tri> &nodec, DiagonalPositions diag_poss, UncompressedNodeContainer<hypertrie_depth_limit, tri> &node_context) : nodec_{&nodec}, diag_poss_{diag_poss} , node_context_(&node_context){
			assert(diag_depth == diag_poss_.count());
		}

		HashDiagonal &begin() {
			if constexpr (depth > 1) {
				const auto min_card_pos = [&]() {
					if constexpr (diag_depth > 1)
						return nodec_->uncompressed_node()->minCardPos(diag_poss_);
					else // diag_depth == 1
						for (auto pos : iter::range(diag_poss_.size()))
							if (diag_poss_[pos])
								return pos;
				}();
				// generate the diagonal positions mask to apply the diagonal to the values of iter_
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
					if (not retrieveSubDiagonalValue()) {
						++(*this);
					}
				}
			} else {
				iter_ = nodec_->uncompressed_node().edges(0).begin();
				end_ = nodec_->uncompressed_node().edges(0).end();
			}
			return *this;
		}

		bool end() const {
			return false;
		}

		auto operator[](key_part_type key_part){
			value_ = node_context_.template diagonal_slice<depth, diag_depth>(*nodec_, diag_poss_, key_part, *internal_compressed_node);
			return value_;
		}
		
	private:
		bool retrieveSubDiagonalValue() {
			static_assert(diag_depth > 1);
			NodeContainer<depth - 1, tri> child_node = node_context_.storage.getNode(iter_->second);
			key_part_type key_part = iter_->first;
			value_ = node_context_.template diagonal_slice<depth - 1, diag_depth>(child_node, sub_diag_poss_, key_part, *internal_compressed_node);
			if constexpr(result_depth == 0)
				return value_ != IterValue{};
			else
				return not value_.first.empty();
		} 
	public:
		bool find(key_part_type key_part) {
			this->operator[](key_part);
			if constexpr(result_depth == 0)
				return value_ != IterValue{};
			else
				return not value_.first.empty();
		}


		const key_part_type &currentKeyPart() const {
			if constexpr (depth == 1 and tri::is_bool_valued)
				return *iter_;
			else
				return iter_->first;
		}

		const IterValue &currentValue() const {
			return value_;
		}

		std::pair<std::reference_wrapper<const key_part_type>,std::reference_wrapper<const IterValue>> operator*() const{
			return std::make_pair(currentKeyPart(), value_);
		}

		HashDiagonal& operator++(){
			if constexpr (depth > 1) {
				assert(not empty());
				do {
					++iter_;
				} while (not ended() and not retrieveSubDiagonalValue());
			} else {
				++iter_;
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
	};

};

#endif//HYPERTRIE_DIAGONAL_HPP
