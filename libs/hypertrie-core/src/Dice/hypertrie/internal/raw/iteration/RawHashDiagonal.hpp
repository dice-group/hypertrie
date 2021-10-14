#ifndef HYPERTRIE_RAWHASHDIAGONAL_HPP
#define HYPERTRIE_RAWHASHDIAGONAL_HPP

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/RawDiagonalPositions.hpp>
#include <Dice/hypertrie/internal/raw/node/FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>

namespace hypertrie::internal::raw {

	template<size_t diag_depth, size_t depth, template<size_t, typename> typename node_type, HypertrieCoreTrait tri, size_t context_max_depth>
	class RawHashDiagonal;

	template<size_t diag_depth, size_t depth, HypertrieCoreTrait tri_t, size_t context_max_depth>
	class RawHashDiagonal<diag_depth, depth, FullNode, tri_t, context_max_depth> {
		static_assert(diag_depth >= 1);
		static_assert(diag_depth <= depth);

	public:
		static constexpr const size_t result_depth = depth - diag_depth;
		using tri = tri_t;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		using DiagonalPositions = RawKeyPositions<depth>;
		using SubDiagonalPositions = RawKeyPositions<(depth > 1) ? depth - 1 : 0>;

	protected:
		using child_iterator = typename FullNode<depth, tri>::ChildrenType::const_iterator;

		using SliceResult_t = SliceResult<result_depth, tri>;
		using IterValue = std::conditional_t<
				(result_depth > 0),
				SliceResult_t,
				value_type>;

		FNContainer<depth, tri> const *node_container_;
		RawHypertrieContext<context_max_depth, tri> *context_;
		DiagonalPositions diag_poss_;
		/**
		 * Only used for diag_depth >= 2
		 */
		SubDiagonalPositions sub_diag_poss_;

		child_iterator iter_;
		child_iterator end_;
		SingleEntryNode<result_depth, tri_with_stl_alloc<tri>> compressed_node_cache_;
		IterValue value_;

	public:
		template<size_t any_depth>
		explicit RawHashDiagonal(FNContainer<depth, tri> const &nodec, DiagonalPositions diag_poss, RawHypertrieContext<any_depth, tri> &node_context) noexcept
			: node_container_(&nodec), diag_poss_(diag_poss), context_(&node_context) {
			static_assert(any_depth >= depth);
		}

		RawHashDiagonal &begin() noexcept {
			if constexpr (depth > 1) {
				const size_t min_card_pos = node_container_->node_ptr()->min_card_pos(diag_poss_);
				// generate the sub_diag_poss_ diagonal positions mask to apply the diagonal to the values of iter_
				if constexpr (diag_depth > 1)
					sub_diag_poss_ = diag_poss_.template sub_raw_key_positions(min_card_pos);

				const auto &min_dim_edges = node_container_->node_ptr()->edges(min_card_pos);
				iter_ = min_dim_edges.begin();
				end_ = min_dim_edges.end();
			} else {// depth == 1 => diag_depth == 1
				iter_ = node_container_->node_ptr()->edges(0).begin();
				end_ = node_container_->node_ptr()->edges(0).end();
			}
			forward_until_result(false);
			return *this;
		}

		bool end() const noexcept {
			return false;
		}

		bool find(key_part_type key_part) noexcept {
			value_ = context_->template diagonal_slice<depth, diag_depth>(*node_container_, diag_poss_, key_part,
																		  (result_depth > 0) ? &compressed_node_cache_ : nullptr);
			if constexpr (result_depth == 0)
				return value_ != value_type{};
			else
				return not value_.empty();
		}


		const key_part_type &current_key_part() const noexcept {
			if constexpr (depth == 1 and HypertrieCoreTrait_bool_valued<tri>)
				return *iter_;
			else
				return iter_->first;
		}

		auto current_value() const noexcept {
			return value_;
		}

		std::pair<key_part_type, IterValue> operator*() const noexcept {
			return std::make_pair(current_key_part(), current_value());
		}


		RawHashDiagonal &operator++() noexcept {
			forward_until_result(true);
			return *this;
		}

		RawHashDiagonal operator++(int) noexcept {
			RawHashDiagonal old = *this;
			++(*this);
			return old;
		}

		operator bool() const noexcept {
			return iter_ != end_;
		}

		bool ended() const noexcept {
			return not bool(*this);
		}

		bool empty() const noexcept {
			return node_container_->empty();
		}

		size_t size() const noexcept {
			if (not empty()) {
				return 0UL;

			} else {
				if constexpr (depth > 1) {
					const auto min_card_pos = node_container_->uncompressed_node()->minCardPos(diag_poss_);
					return node_container_->uncompressed_node()->edges(min_card_pos).size();

				} else {
					return node_container_->uncompressed_node()->size();
				}
			}
		}

	protected:
		void forward_until_result(bool ignore_current) noexcept {
			if (ignore_current)
				++iter_;
			if constexpr (diag_depth >= 2) {
				assert(not empty());
				while (not ended() and not retrieve_subdiagonal_value()) {
					++iter_;
				}
			} else {
				if (not ended()) {
					if constexpr (result_depth == 0) {
						if constexpr (HypertrieCoreTrait_bool_valued<tri>)
							value_ = true;
						else
							value_ = iter_->second;
					} else {
						RawIdentifier<result_depth, tri> next_result_hash = iter_->second;
						if constexpr (result_depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) {// TODO: was only bool valued before
							if (iter_->second.is_sen()) {
								value_ = SliceResult<result_depth, tri>::make_managed_tri_alloc(next_result_hash);
								return *this;
							}
						}

						NodeContainer<result_depth, tri> child_node_container = context_->node_storage_.template lookup<result_depth>(next_result_hash);
						value_ = SliceResult<result_depth, tri>::make_with_tri_alloc(child_node_container);
					}
				}
			}
		}
		/**
		 * Retrieves the value from the sub-diagonal.
		 * @return if a value was found
		 */
		bool retrieve_subdiagonal_value() noexcept {
			static_assert(diag_depth >= 2);
			const key_part_type key_part = iter_->first;

			if constexpr (HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri> and depth == 2)
				if (iter_->second.is_sen()) {
					value_ = iter_->second.get_entry().key()[0] == key_part;
					return value_;
				}

			NodeContainer<depth - 1, tri> child_node = context_->node_storage_.lookup(iter_->second);
			value_ = context_->template diagonal_slice<depth - 1, diag_depth - 1>(child_node, sub_diag_poss_, key_part,
																				  (result_depth > 0) ? &compressed_node_cache_ : nullptr);
			if constexpr (result_depth == 0)
				return value_ != value_type{};
			else
				return not value_.empty();
		}
	};

	template<size_t diag_depth, size_t depth, HypertrieCoreTrait tri_t, size_t context_max_depth>
	class RawHashDiagonal<diag_depth, depth, SingleEntryNode, tri_t, context_max_depth> {
		static_assert(diag_depth >= 1);
		static_assert(diag_depth <= depth);

	public:
		using tri = tri_t;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		static constexpr const size_t result_depth = depth - diag_depth;

		using DiagonalPositions = RawKeyPositions<depth>;

	protected:
		using SliceResult_t = SliceResult<result_depth, tri>;
		using IterValue = std::conditional_t<
				(result_depth > 0),
				SliceResult_t,
				value_type>;

		SENContainer<depth, tri> *nodec_;
		SingleEntryNode<result_depth, tri_with_stl_alloc<tri>> internal_compressed_node;
		std::pair<key_part_type, IterValue> value_;
		bool ended_ = true;
		bool contains;
		DiagonalPositions diag_poss_;

	public:
		RawHashDiagonal(SENContainer<depth, tri> &nodec, DiagonalPositions diag_poss) noexcept
			: nodec_{&nodec}, diag_poss_(diag_poss) {
			value_.first = [&]() {// key_part
				if constexpr (depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>)
					return nodec_->raw_identifier().get_entry().key()[0];
				else
					return nodec_->compressed_node()->key()[diag_poss.first_pos()];
			}();
			if constexpr (depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) {
				contains = true;
				value_.second = true;
			} else {

				auto opt_slice = diag_poss.template slice<diag_depth>(nodec_->node_ptr()->key());

				if (opt_slice.has_value()) {
					contains = true;
					if constexpr (diag_depth == depth) {
						value_.second = nodec_->node_ptr().value();
					} else {
						internal_compressed_node = {opt_slice.value(), nodec_->node_ptr().value()};
						value_.second = SliceResult_t::make_with_stl_alloc(true, RawIdentifier<result_depth, tri_with_stl_alloc<tri>>{internal_compressed_node});
					}
				} else {
					contains = false;
				}
			}
		}

		RawHashDiagonal &begin() noexcept {
			if (contains)
				ended_ = false;
			return *this;
		}

		bool end() const noexcept {
			return false;
		}

		bool find(key_part_type key_part) noexcept {
			return (key_part == value_.first);
		}


		const key_part_type &currentKeyPart() const noexcept {
			return value_.first;
		}

		auto currentValue() const noexcept {
			return value_.second;
		}

		const std::pair<key_part_type, IterValue> &operator*() const noexcept {
			return value_;
		}

		RawHashDiagonal &operator++() noexcept {
			ended_ = true;
			return *this;
		}

		RawHashDiagonal operator++(int) noexcept {
			RawHashDiagonal old = *this;
			++(*this);
			return old;
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
}// namespace hypertrie::internal::raw


#endif//HYPERTRIE_RAWHASHDIAGONAL_HPP
