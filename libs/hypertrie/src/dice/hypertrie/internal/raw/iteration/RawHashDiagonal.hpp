#ifndef HYPERTRIE_RAWHASHDIAGONAL_HPP
#define HYPERTRIE_RAWHASHDIAGONAL_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/RawDiagonalPositions.hpp"
#include "dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "dice/hypertrie/internal/raw/node/NodeContainer.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"
#include "dice/hypertrie/internal/raw/node_context/SliceResult.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t diag_depth, size_t depth, template<size_t, typename, typename> typename node_type, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	class RawHashDiagonal;

	template<size_t diag_depth, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	class RawHashDiagonal<diag_depth, depth, FullNode, htt_t, allocator_type, context_max_depth> {
		static_assert(diag_depth >= 1);
		static_assert(diag_depth <= depth);

	public:
		static constexpr const size_t result_depth = depth - diag_depth;
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;

		using DiagonalPositions = RawKeyPositions<depth>;
		using SubDiagonalPositions = RawKeyPositions<(depth > 1) ? depth - 1 : 0>;

	protected:
		using child_iterator = typename FullNode<depth, htt_t, allocator_type>::ChildrenType::const_iterator;

		using SliceResult_t = SliceResult<result_depth, htt_t, allocator_type>;
		using IterValue = std::conditional_t<
				(result_depth > 0),
				SliceResult_t,
				value_type>;

		FNContainer<depth, htt_t, allocator_type> const node_container_;
		RawHypertrieContext<context_max_depth, htt_t, allocator_type> *context_;
		DiagonalPositions diag_poss_;
		/**
		 * Only used for diag_depth >= 2
		 */
		SubDiagonalPositions sub_diag_poss_;

		child_iterator iter_;
		child_iterator end_;
		SingleEntryNode<result_depth, htt_t, std::allocator<std::byte>> sen_cache_;
		IterValue value_;

	public:
		RawHashDiagonal() = default;
		~RawHashDiagonal() = default;

		template<size_t any_depth>
		explicit RawHashDiagonal(FNContainer<depth, htt_t, allocator_type> const &nodec, DiagonalPositions diag_poss, RawHypertrieContext<any_depth, htt_t, allocator_type> &node_context) noexcept
			: node_container_(nodec), context_(&node_context), diag_poss_(diag_poss) {
			static_assert(any_depth >= depth);
		}

		void update_sen_cache_ptr() noexcept {
			if constexpr (result_depth > 0) {
				if (not value_.uses_provided_alloc() and not value_.empty()) {
					value_.get_stl_alloc_sen().node_ptr(&sen_cache_);
					assert(value_.get_stl_alloc_sen().node_ptr() != nullptr);
				}
			}
		}

		RawHashDiagonal(RawHashDiagonal const &other) noexcept
			: node_container_(other.node_container_),
			  context_(other.context_),
			  diag_poss_(other.diag_poss_),
			  sub_diag_poss_(other.sub_diag_poss_),
			  iter_(other.iter_),
			  end_(other.end_),
			  sen_cache_(other.sen_cache_),
			  value_(other.value_) {
			assert(this != &other);
			update_sen_cache_ptr();
		}

		RawHashDiagonal &operator=(RawHashDiagonal const &other) noexcept {
			if (this == &other)
				return *this;

			node_container_ = other.node_container_;
			context_ = other.context_;
			diag_poss_ = other.diag_poss_;
			sub_diag_poss_ = other.sub_diag_poss_;
			iter_ = other.iter_;
			end_ = other.end_;
			sen_cache_ = other.sen_cache_;
			value_ = other.value_;

			update_sen_cache_ptr();
			return *this;
		}

		RawHashDiagonal(RawHashDiagonal &&other) noexcept
			: node_container_(std::move(other.node_container_)),
			  context_(other.context_),
			  diag_poss_(other.diag_poss_),
			  sub_diag_poss_(other.sub_diag_poss_),
			  iter_(std::move(other.iter_)),
			  end_(std::move(other.end_)),
			  sen_cache_(other.sen_cache_),
			  value_(other.value_) {
			assert(this != &other);
			update_sen_cache_ptr();
			other.context_ = nullptr;
			other.diag_poss_ = {};
			other.sub_diag_poss_ = {};
			other.iter_ = {};
			other.end_ = {};
			other.sen_cache_ = {};
			other.value_ = {};
		}

		RawHashDiagonal &operator=(RawHashDiagonal &&other) noexcept {
			if (this == &other)
				return *this;

			node_container_ = other.node_container_;
			context_ = other.context_;
			diag_poss_ = other.diag_poss_;
			sub_diag_poss_ = other.sub_diag_poss_;
			iter_ = std::move(other.iter_);
			end_ = std::move(other.end_);
			sen_cache_ = other.sen_cache_;
			value_ = other.value_;

			update_sen_cache_ptr();

			other.node_container_ = {};
			other.context_ = nullptr;
			other.diag_poss_ = {};
			other.sub_diag_poss_ = {};
			other.iter_ = {};
			other.end_ = {};
			other.sen_cache_ = {};
			other.value_ = {};
			return *this;
		}


		/**
		 * this must not be empty()
		 */
		RawHashDiagonal &begin() noexcept {
			if constexpr (depth > 1) {
				const size_t min_card_pos = node_container_.node_ptr()->min_card_pos(diag_poss_);
				// generate the sub_diag_poss_ diagonal positions mask to apply the diagonal to the values of iter_
				if constexpr (diag_depth > 1)
					sub_diag_poss_ = diag_poss_.sub_raw_key_positions(min_card_pos);

				const auto &min_dim_edges = node_container_.node_ptr()->edges(min_card_pos);
				iter_ = min_dim_edges.begin();
				end_ = min_dim_edges.end();
			} else {// depth == 1 => diag_depth == 1
				iter_ = node_container_.node_ptr()->edges(0).begin();
				end_ = node_container_.node_ptr()->edges(0).end();
			}
			forward_until_result(false);
			return *this;
		}

		[[nodiscard]] bool end() const noexcept {
			return false;
		}

		[[nodiscard]] bool find(key_part_type key_part) noexcept {
			value_ = context_->template diagonal_slice<depth, diag_depth>(node_container_, diag_poss_, key_part,
																		  (result_depth > 0) ? &sen_cache_ : nullptr);
			if constexpr (result_depth == 0)
				return value_ != value_type{};
			else
				return not value_.empty();
		}

		/**
		 * Only valid when used as a iterator (not with find())
		 */
		[[nodiscard]] const key_part_type &current_key_part() const noexcept {
			if constexpr (depth == 1 and HypertrieTrait_bool_valued<htt_t>)
				return *iter_;
			else
				return iter_->first;
		}

		/**
		 * this must not be empty()
		 */
		[[nodiscard]] IterValue current_value() const noexcept {
			return value_;
		}

		/**
		 * Only valid when used as a iterator (not with find())
		 */
		[[nodiscard]] std::pair<key_part_type, IterValue> operator*() const noexcept {
			return std::make_pair(current_key_part(), current_value());
		}

		/**
		 * this must not be empty()
		 */
		RawHashDiagonal &operator++() noexcept {
			forward_until_result(true);
			return *this;
		}

		/**
		 * this must not be empty()
		 */
		RawHashDiagonal operator++(int) noexcept {
			RawHashDiagonal old = *this;
			++(*this);
			return old;
		}

		/**
		 * this must not be empty()
		 */
		operator bool() const noexcept {
			return iter_ != end_;
		}

		[[nodiscard]] bool ended() const noexcept {
			return not bool(*this);
		}

		/**
		 * Check whether the hypertrie where the diagonal is applied is empty()
		 */
		[[nodiscard]] bool empty() const noexcept {
			return node_container_.empty();
		}

		/**
		 * Upper bound to the number of non-zero slices
		 */
		[[nodiscard]] size_t size() const noexcept {
			if (empty()) {
				return 0UL;
			}
			if constexpr (depth > 1) {
				const auto min_card_pos = node_container_.node_ptr()->min_card_pos(diag_poss_);
				return node_container_.node_ptr()->edges(min_card_pos).size();
			} else {
				return node_container_.node_ptr()->size();
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
						if constexpr (HypertrieTrait_bool_valued<htt_t>)
							value_ = true;
						else
							value_ = iter_->second;
					} else {
						RawIdentifier<result_depth, htt_t> next_result_hash = iter_->second;
						if constexpr (result_depth == 1 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {// TODO: double check algorithm: was only bool valued before
							if (iter_->second.is_sen()) {
								value_ = SliceResult<result_depth, htt_t, allocator_type>::make_with_provided_alloc(next_result_hash);

							} else {
								auto child_node_ptr = context_->node_storage_.template lookup<result_depth, FullNode>(next_result_hash);
								value_ = SliceResult<result_depth, htt_t, allocator_type>::make_with_provided_alloc(next_result_hash, child_node_ptr);
							}
							return;
						} else {
							NodeContainer<result_depth, htt_t, allocator_type> child_node_container = context_->node_storage_.template lookup<result_depth>(next_result_hash);
							value_ = SliceResult<result_depth, htt_t, allocator_type>::make_with_provided_alloc(child_node_container);
						}
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

			if constexpr (HypertrieTrait_bool_valued_and_taggable_key_part<htt_t> and depth == 2) {
				if (iter_->second.is_sen()) {
					value_ = iter_->second.get_entry().key()[0] == key_part;

				} else {
					auto child_node_ptr = context_->node_storage_.template lookup<1, FullNode>(iter_->second);
					value_ = context_->template get<1>(FNContainer<1, htt_t, allocator_type>{iter_->second, child_node_ptr}, {{key_part}});
				}
				return value_;
			} else {

				NodeContainer<depth - 1, htt_t, allocator_type> child_node = context_->node_storage_.lookup(iter_->second);
				value_ = context_->template diagonal_slice<depth - 1, diag_depth - 1>(child_node, sub_diag_poss_, key_part,
																					  (result_depth > 0) ? &sen_cache_ : nullptr);
				if constexpr (result_depth == 0)
					return value_ != value_type{};
				else
					return not value_.empty();
			}
		}
	};

	template<size_t diag_depth, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	class RawHashDiagonal<diag_depth, depth, SingleEntryNode, htt_t, allocator_type, context_max_depth> {
		static_assert(diag_depth >= 1);
		static_assert(diag_depth <= depth);

	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		static constexpr const size_t result_depth = depth - diag_depth;

		using DiagonalPositions = RawKeyPositions<depth>;

	protected:
		using SliceResult_t = SliceResult<result_depth, htt_t, allocator_type>;
		using IterValue = std::conditional_t<
				(result_depth > 0),
				SliceResult_t,
				value_type>;

		SENContainer<depth, htt_t, allocator_type> nodec_;
		SingleEntryNode<result_depth, htt_t, std::allocator<std::byte>> sen_cache_;
		std::pair<key_part_type, IterValue> value_;
		bool ended_ = true;
		bool is_diagonal_ = false;
		DiagonalPositions diag_poss_;

	public:
		RawHashDiagonal() = default;
		~RawHashDiagonal() = default;

		RawHashDiagonal(SENContainer<depth, htt_t, allocator_type> const &nodec, DiagonalPositions diag_poss) noexcept
			: nodec_{nodec}, diag_poss_(diag_poss) {
			value_.first = [&]() {// key_part
				if constexpr (depth == 1 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
					return nodec_.raw_identifier().get_entry().key()[0];
				else
					return nodec_.node_ptr()->key()[diag_poss.first_pos()];
			}();
			if constexpr (depth == 1 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				is_diagonal_ = true;
				value_.second = true;
			} else {

				auto opt_slice = diag_poss.template slice<diag_depth>(nodec_.node_ptr()->key(), value_.first);

				if (opt_slice.has_value()) {
					is_diagonal_ = true;
					if constexpr (diag_depth == depth) {
						value_.second = nodec_.node_ptr()->value();
					} else {
						sen_cache_ = SingleEntryNode<result_depth, htt_t, std::allocator<std::byte>>{opt_slice.value(), nodec_.node_ptr()->value()};
						value_.second = SliceResult_t::make_sen_with_stl_alloc(true, RawIdentifier<result_depth, htt_t>{sen_cache_}, &sen_cache_);
					}
				} else {
					is_diagonal_ = false;
				}
			}
		}

		void update_sen_cache_ptr() noexcept {
			if constexpr (result_depth > 0) {
				if (not value_.second.uses_provided_alloc() and not value_.second.empty()) {
					value_.second.get_stl_alloc_sen().node_ptr(&sen_cache_);
					assert(value_.second.get_stl_alloc_sen().node_ptr() != nullptr);
				}
			}
		}

		RawHashDiagonal(RawHashDiagonal const &other) noexcept
			: nodec_(other.nodec_),
			  sen_cache_(other.sen_cache_),
			  value_(other.value_),
			  ended_(other.ended_),
			  is_diagonal_(other.is_diagonal_),
			  diag_poss_(other.diag_poss_) {
			assert(this != &other);

			update_sen_cache_ptr();
		}

		RawHashDiagonal &operator=(RawHashDiagonal const &other) noexcept {
			if (this == &other)
				return *this;

			nodec_ = other.nodec_;
			sen_cache_ = other.sen_cache_;
			value_ = other.value_;
			ended_ = other.ended_;
			is_diagonal_ = other.is_diagonal_;
			diag_poss_ = other.diag_poss_;

			update_sen_cache_ptr();
			return *this;
		}

		RawHashDiagonal(RawHashDiagonal &&other) noexcept
			: nodec_(other.nodec_),
			  sen_cache_(other.sen_cache_),
			  value_(other.value_),
			  ended_(other.ended_),
			  is_diagonal_(other.is_diagonal_),
			  diag_poss_(other.diag_poss_) {
			assert(this != &other);
			update_sen_cache_ptr();

			other.nodec_ = {};
			other.sen_cache_ = {};
			other.value_ = {};
			other.ended_ = true;
			other.is_diagonal_ = false;
			other.diag_poss_ = {};
		}

		RawHashDiagonal &operator=(RawHashDiagonal &&other) noexcept {
			if (this == &other)
				return *this;

			nodec_ = other.nodec_;
			sen_cache_ = other.sen_cache_;
			value_ = other.value_;
			ended_ = other.ended_;
			is_diagonal_ = other.is_diagonal_;
			diag_poss_ = other.diag_poss_;

			update_sen_cache_ptr();

			other.nodec_ = {};
			other.sen_cache_ = {};
			other.value_ = {};
			other.ended_ = true;
			other.is_diagonal_ = false;
			other.diag_poss_ = {};

			return *this;
		}

		RawHashDiagonal &begin() noexcept {
			if (is_diagonal_)
				ended_ = false;
			return *this;
		}

		[[nodiscard]] bool end() const noexcept {
			return false;
		}

		[[nodiscard]] bool find(key_part_type key_part) const noexcept {
			return is_diagonal_ and (key_part == value_.first);
		}

		/**
		 * Must only be called if not ended()
		 */
		[[nodiscard]] const key_part_type &current_key_part() const noexcept {
			return value_.first;
		}

		/**
		 * Must only be called if not ended()
		 */
		[[nodiscard]] auto current_value() const noexcept {
			return value_.second;
		}

		[[nodiscard]] const std::pair<key_part_type, IterValue> &operator*() const noexcept {
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

		[[nodiscard]] bool ended() const noexcept {
			return ended_;
		}

		[[nodiscard]] bool empty() const noexcept {
			return not is_diagonal_;
		}

		/**
		 * Upper bound to the number of non-zero slices
		 */
		[[nodiscard]] size_t size() const noexcept {
			return size_t(is_diagonal_);
		}
	};
}// namespace dice::hypertrie::internal::raw


#endif//HYPERTRIE_RAWHASHDIAGONAL_HPP
