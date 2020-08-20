#ifndef HYPERTRIE_DIAGONAL_HPP
#define HYPERTRIE_DIAGONAL_HPP

#include "Dice/hypertrie/internal/node_based/raw/storage/NodeContext.hpp"
#include "Dice/hypertrie/internal/node_based/ConfigHypertrieDepthLimit.hpp"
#include "Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp"
namespace hypertrie::internal::node_based::raw {

	template<size_t diag_depth, size_t depth, NodeCompression compression, HypertrieInternalTrait tri_t>
	class HashDiagonal

	template<size_t diag_depth, size_t depth, HypertrieInternalTrait tri_t>
	class HashDiagonal<diag_depth, depth, NodeCompression::uncompressed, tri_t> {
			static_assert(diag_depth <= depth);
			stacic_assert(diag_depth > 0);

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

		using IterValue = std::conditional_t<(result_depth > 0),
				std::pair<NodeContainer<result_depth, tri>, bool>,
		value_type>

	private:
		UncompressedNodeContainer<depth, tri> *nodec_;
		NodeContext<hypertrie_depth_limit, tri> node_context_;
		DiagonalPositions diag_poss_;
		DiagonalPositions sub_diag_poss_;

		child_iterator iter;
		child_iterator end;
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
				// generate the diagonal positions mask to apply the diagonal to the values of iter
				if constexpr (diag_depth > 1) {
					bool skipped = false;
					for(auto pos : iter::range(diag_poss_.size()))
						if(pos == min_card_pos)
							skipped = true;
						else
							sub_diag_poss_[pos - skipped] = diag_poss_[pos];

				}

				const auto &min_dim_edges = nodec_->uncompressed_node()->edges(min_card_pos);
				diag.iter = min_dim_edges.begin();
				diag.end = min_dim_edges.end();
				if (not ended()) {
					if (not retrieveSubDiagonalValue()) {
						inc(diag_ptr);
					}
				}
			} else {
				diag.iter = diag.rawboolhypertrie.edges.begin();
				diag.end = diag.rawboolhypertrie.edges.end();
			}
			return *this;
		}

		bool end() const {
			return false;
		}

		static void init(void *diag_ptr) {
			reinterpret_cast<HashDiagonal *>(diag_ptr)->begin();
		}

		auto operator[](key_part_type key_part){
			iter_value = node_context_.template diagonal_slice<depth, diag_depth>(*nodec_, diag_poss_, key_part, *internal_compressed_node);
			return iter_value;
		}
		
	private:
		bool retrieveSubDiagonalValue() {
			static_assert(diag_depth > 1);
			NodeContainer<depth - 1, tri> child_node = node_context_.storage.getNode(iter->second);
			key_part_type key_part = iter->first;
			iter_value = node_context_.template diagonal_slice<depth - 1, diag_depth>(child_node, sub_diag_poss_, key_part, *internal_compressed_node);
			if constexpr(result_depth == 0)
				return value_ != {};
			else
				return not value_.first.empty();
		} 
	public:
		static bool find(void *diag_ptr, key_part_type key_part) {
			auto &diag = *reinterpret_cast<HashDiagonal *>(diag_ptr);
			diag[key_part];
			if constexpr(result_depth == 0)
				return value_ != {};
			else
				return not value_.first.empty();
		}


		static key_part_type currentKeyPart(void const *diag_ptr) {
			auto &diag = *static_cast<RawHashDiagonal const *>(diag_ptr);
			if constexpr (depth == 1 and tri::is_bool_valued)
				return *diag.iter;
			else
				return diag.iter->first;

		}

		static value_type *currentValue(void const *diag_ptr) {
			auto &diag = *static_cast<RawHashDiagonal const *>(diag_ptr);
			return &diag.value_;
		}

		HashDiagonal& operator++(){
			if constexpr (depth > 1) {
				assert(not empty(diag_ptr));
				do {
					++iter;
				} while (not ended() and not retrieveSubDiagonalValue());
			} else {
				++diag.iter;
			}
			return *this;
		}

		std::pair<key_part_type, IterValue> operator*() const{
			return {iter->first, value_};
		}

	   static void inc(void *diag_ptr) {
		   reinterpret_cast<HashDiagonal *>(diag_ptr)->operator++();
		}

		operator(bool) const noexcept {
			return iter != end;
		}

		bool ended() const noexcept {
			return not bool(*this);
		}

		static bool ended_static(void const *diag_ptr) {
			return reinterpret_cast<HashDiagonal const *>(diag_ptr)->ended();
		}

		size_t size() const noexcept {

			if constexpr (depth > 1) {
				const auto min_card_pos = diag.rawboolhypertrie.minCardPos();
				return diag.rawboolhypertrie.edges[min_card_pos].size();
			} else {
				return diag.rawboolhypertrie.size();
			}
		}

		static size_t size_static(void const *diag_ptr) {
			reinterpret_cast<HashDiagonal const *>(diag_ptr)->size();
		}
	};

	template<size_t diag_depth, size_t depth, HypertrieInternalTrait tri_t>
	class HashDiagonal<diag_depth, depth, NodeCompression::compressed, tri_t> {
	};

};

#endif//HYPERTRIE_DIAGONAL_HPP
