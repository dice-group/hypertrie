#ifndef HYPERTRIE_RAWHASHDIAGONAL_HPP
#define HYPERTRIE_RAWHASHDIAGONAL_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/RawDiagonalPositions.hpp"
#include "dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"
#include "dice/hypertrie/internal/raw/node_context/SliceResult.hpp"

namespace dice::hypertrie::internal::raw {

	namespace diagonal_detail {
		template<size_t diag_depth, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		struct RawHashDiagonalBase {
			static_assert(diag_depth >= 1);
			static_assert(diag_depth <= depth);

			static constexpr size_t result_depth = depth - diag_depth;

			using diagonal_type = std::conditional_t<
					(result_depth > 0),
					SliceResult<result_depth, htt_t, allocator_type>,
					typename htt_t::value_type>;

			using value_type = std::pair<typename htt_t::key_part_type, diagonal_type>;
			using result_buffer_type = SliceResultStorage<result_depth, htt_t, allocator_type>;

		protected:
			value_type *value_buffer_;
			result_buffer_type *result_buffer_;
			
		public:
			RawHashDiagonalBase(value_type *value_buffer, result_buffer_type *result_buffer) noexcept : value_buffer_{value_buffer}, result_buffer_{result_buffer} {}

			/**
			 * Advances the iterator to the diagonal at key_part
			 */
			virtual bool find(typename htt_t::key_part_type key_part) noexcept = 0;

			/**
			 * Advances the iterator one step
			 */
			virtual void advance() noexcept = 0;

			/**
			 * @return if there is no next diagonal
			 */
			virtual bool ended() const noexcept = 0;

			/**
			 * Upper bound to the number of non-zero slices
			 */
			virtual size_t size() const noexcept = 0;

			/**
		 	 * Check whether the hypertrie where the diagonal is applied is empty()
		 	 */
			virtual bool empty() const noexcept = 0;

			/**
			 * Clone this into dst
			 */
			virtual void clone_to(RawHashDiagonalBase *dst) const noexcept = 0;

			/**
			 * repoints the internal write buffers
			 */
			virtual void repoint_buffers(value_type *value_buffer, result_buffer_type *result_buffer) noexcept {
				value_buffer_ = value_buffer;
				result_buffer_ = result_buffer;
			}
		};

		template<size_t diag_depth, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		struct FNRawHashDiagonal final : RawHashDiagonalBase<diag_depth, depth, htt_t, allocator_type> {
			using Base = RawHashDiagonalBase<diag_depth, depth, htt_t, allocator_type>;
			using value_type = typename Base::value_type;
			using diagonal_type = typename Base::diagonal_type;
			using result_buffer_type = typename Base::result_buffer_type;

		private:
			static constexpr const size_t result_depth = depth - diag_depth;

			using child_iterator = typename FullNode<depth, htt_t, allocator_type>::single_dim_edges_type::const_iterator;

			FNPtr<depth, htt_t, allocator_type> const node_;

			RawKeyPositions<depth> diag_poss_;
			RawKeyPositions<depth - 1> sub_diag_poss_; // Only used for diag_depth >= 2

			child_iterator iter_;
			child_iterator end_;

		public:
			FNRawHashDiagonal(FNPtr<depth, htt_t, allocator_type> fn,
							  RawKeyPositions<depth> diag_poss,
							  value_type *value_buffer,
							  result_buffer_type *result_buffer) noexcept : Base{value_buffer, result_buffer},
																			node_{fn},
																			diag_poss_{diag_poss} {
				if constexpr (depth > 1) {
					const size_t min_card_pos = fn->min_card_pos(diag_poss_);
					// generate the sub_diag_poss_ diagonal positions mask to apply the diagonal to the values of iter_
					if constexpr (diag_depth > 1) {
						sub_diag_poss_ = diag_poss_.sub_raw_key_positions(min_card_pos);
					}

					auto const &min_dim_edges = fn->edges(min_card_pos);
					iter_ = min_dim_edges.cbegin();
					end_ = min_dim_edges.cend();
				} else {// depth == 1 => diag_depth == 1
					iter_ = fn->edges(0).cbegin();
					end_ = fn->edges(0).cend();
				}
				advance_until_result(false);
			}

			bool find(typename htt_t::key_part_type key_part) noexcept override {
				this->value_buffer_->first = key_part;
				this->value_buffer_->second = node_context::slice_detail::diagonal_slice<diag_depth, depth, htt_t, allocator_type>(node_,
																																   diag_poss_,
																																   key_part,
																																   this->result_buffer_);

				if constexpr (result_depth == 0) {
					return this->value_buffer_->second != typename htt_t::value_type{};
				} else {
					return !this->value_buffer_->second.empty();
				}
			}

			void advance() noexcept override {
				advance_until_result(true);
			}

			[[nodiscard]] bool ended() const noexcept override {
				return iter_ == end_;
			}

			[[nodiscard]] bool empty() const noexcept override {
				return node_ == nullptr;
			}

			[[nodiscard]] size_t size() const noexcept override {
				if (empty()) {
					return 0;
				}

				if constexpr (depth > 1) {
					auto const min_card_pos = node_->min_card_pos(diag_poss_);
					return node_->edges(min_card_pos).size();
				} else {
					return node_->size();
				}
			}

			void clone_to(Base *dst) const noexcept override {
				new (dst) FNRawHashDiagonal{*this};
			}

		protected:
			void commit_keypart() noexcept {
				if constexpr (depth == 1 && HypertrieTrait_bool_valued<htt_t>) {
					this->value_buffer_->first = *iter_;
				} else {
					this->value_buffer_->first = iter_->first;
				}
			}

			void advance_until_result(bool ignore_current) noexcept {
				if (ignore_current) {
					++iter_;
				}

				if constexpr (diag_depth >= 2) {
					assert(!empty());
					while (!ended() && !retrieve_subdiagonal_value()) {
						++iter_;
					}

				} else {
					if (ended()) {
						return;
					}

					commit_keypart();
					if constexpr (result_depth == 0) {
						if constexpr (HypertrieTrait_bool_valued<htt_t>) {
							this->value_buffer_->second = true;
						} else {
							this->value_buffer_->second = iter_->second;
						}
					} else {
						if constexpr (result_depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
							if (iter_->second.is_sen()) {
								this->value_buffer_->second = SliceResult<result_depth, htt_t, allocator_type>{iter_->second.decode_key_part()};
								return;
							}
						}

						this->value_buffer_->second = SliceResult<result_depth, htt_t, allocator_type>{Ownership::ContextBorrowed, iter_->second};
					}
				}
			}

			/**
			 * Retrieves the value from the sub-diagonal.
			 * @return if a value was found
			 */
			bool retrieve_subdiagonal_value() noexcept {
				static_assert(diag_depth >= 2);
				auto const key_part = iter_->first;
				this->value_buffer_->first = key_part;

				if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					if (iter_->second.is_sen()) {
						this->value_buffer_->second = iter_->second.decode_key_part() == key_part;
						return this->value_buffer_->second;
					}
				}

				this->value_buffer_->second = node_context::slice_detail::diagonal_slice<diag_depth - 1>(iter_->second,
																										 sub_diag_poss_,
																										 key_part,
																										 this->result_buffer_);
				if constexpr (result_depth == 0) {
					return this->value_buffer_->second != typename htt_t::value_type{};
				} else {
					return !this->value_buffer_->second.empty();
				}
			}
		};

		template<size_t diag_depth, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		struct SENRawHashDiagonal final : RawHashDiagonalBase<diag_depth, depth, htt_t, allocator_type> {
			using Base = RawHashDiagonalBase<diag_depth, depth, htt_t, allocator_type>;
			using value_type = typename Base::value_type;
			using diagonal_type = typename Base::diagonal_type;
			using result_buffer_type = typename Base::result_buffer_type;

		private:
			static constexpr const size_t result_depth = depth - diag_depth;

			std::optional<std::pair<typename htt_t::key_part_type, SingleEntry<depth - diag_depth, htt_t>>> diagonal_ = std::nullopt;
			bool ended_ = false;

			void commit() noexcept {
				assert(diagonal_.has_value());

				this->value_buffer_->first = diagonal_->first;

				if constexpr (result_depth == 0) {
					this->value_buffer_->second = diagonal_->second.value();
				} else {
					if constexpr (result_depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						this->value_buffer_->second = diagonal_type{diagonal_->second.key()[0]};
					} else {
						if (this->result_buffer_ == nullptr) {
							auto *sen = new SingleEntryNode<result_depth, htt_t>{diagonal_->second, 0};
							this->value_buffer_->second = diagonal_type{Ownership::Owned,
																		sen};
						} else {
							auto *sen = &this->result_buffer_->sen;
							*sen = SingleEntryNode<result_depth, htt_t>{diagonal_->second, 0};
							this->value_buffer_->second = diagonal_type{Ownership::EphemeralBorrowed,
																		sen};
						}
					}
				}
			}

		public:
			SENRawHashDiagonal(typename htt_t::key_part_type key_part,
							   value_type *value_buffer,
							   result_buffer_type *result_buffer) noexcept requires (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
				: Base{value_buffer, result_buffer},
				  diagonal_{std::make_pair(key_part, SingleEntry<0, htt_t>{{}, true})} /*depth == 1 implies diag_depth == 1*/ {
				commit();
			}

			SENRawHashDiagonal(SENPtr<depth, htt_t, allocator_type> sen,
							   RawKeyPositions<depth> diag_poss,
							   value_type *value_buffer,
							   result_buffer_type *result_buffer) noexcept requires (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
				: Base{value_buffer, result_buffer} {

				auto const key_part = sen->key()[diag_poss.first_pos()];
				auto maybe_sliced_key = diag_poss.template slice<diag_depth>(sen->key(), key_part);

				if (maybe_sliced_key.has_value()) {
					diagonal_ = std::make_pair(key_part, SingleEntry<result_depth, htt_t>{*maybe_sliced_key, sen->value()});
				}

				ended_ = !diagonal_.has_value();

				if (diagonal_.has_value()) {
					commit();
				}
			}

			bool find(typename htt_t::key_part_type key_part) noexcept override {
				bool const ret = diagonal_.has_value() && key_part == this->value_buffer_->first;
				if (ret) {
					commit();
				}
				return ret;
			}

			void advance() noexcept override {
				ended_ = true;
			}

			[[nodiscard]] bool ended() const noexcept override {
				return ended_;
			}

			[[nodiscard]] bool empty() const noexcept override {
				return !diagonal_.has_value();
			}

			[[nodiscard]] size_t size() const noexcept override {
				return static_cast<size_t>(diagonal_.has_value());
			}

			void clone_to(Base *dst) const noexcept override {
				new (dst) SENRawHashDiagonal{*this};
			}
		};

		template<size_t diag_depth, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type> requires (depth >= 2)
		struct XNRawHashDiagonal final : RawHashDiagonalBase<diag_depth, depth, htt_t, allocator_type> {
			using Base = RawHashDiagonalBase<diag_depth, depth, htt_t, allocator_type>;
			using value_type = typename Base::value_type;
			using diagonal_type = typename Base::diagonal_type;
			using result_buffer_type = typename Base::result_buffer_type;

		private:
			static constexpr const size_t result_depth = depth - diag_depth;

			static consteval size_t max_iter_align() {
				auto const base_align = alignof(iterator_detail::FNIterator<1, htt_t, allocator_type>);

				if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
					return std::max(base_align, alignof(iterator_detail::KeyPartIterator<htt_t>));
				} else {
					return std::max(base_align, alignof(iterator_detail::SENIterator<1, htt_t, allocator_type>));
				}
			}

			static consteval size_t max_iter_size(){
				auto const base_align = sizeof(iterator_detail::FNIterator<1, htt_t, allocator_type>);

				if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
					return std::max(base_align, sizeof(iterator_detail::KeyPartIterator<htt_t>));
				} else {
					return std::max(base_align, sizeof(iterator_detail::SENIterator<1, htt_t, allocator_type>));
				}
			}

			XNPtr<depth, htt_t, allocator_type> cartesian_;
			RawKeyPositions<depth> diag_poss_;
			size_t size_upper_bound_;

			alignas(max_iter_align()) std::byte keypart_iter_[max_iter_size()];

			iterator_detail::NodeIteratorBase<htt_t> *keypart_iter() noexcept {
				return reinterpret_cast<iterator_detail::NodeIteratorBase<htt_t> *>(keypart_iter_);
			}

			iterator_detail::NodeIteratorBase<htt_t> const *keypart_iter() const noexcept {
				return reinterpret_cast<iterator_detail::NodeIteratorBase<htt_t> const *>(keypart_iter_);
			}

			void advance_until_result() {
				while (!keypart_iter()->ended() && !commit(this->value_buffer_->first)) {
					keypart_iter()->advance();
				}
			}

			bool commit(typename htt_t::key_part_type const key_part) noexcept {
				this->value_buffer_->second = node_context::slice_detail::diagonal_slice<diag_depth, depth, htt_t, allocator_type>(cartesian_,
																																   diag_poss_,
																																   key_part,
																																   this->result_buffer_);

				if constexpr (result_depth == 0) {
					return this->value_buffer_->second != typename htt_t::value_type{};
				} else {
					return !this->value_buffer_->second.empty();
				}
			}

		public:
			XNRawHashDiagonal(XNPtr<depth, htt_t, allocator_type> xn,
							  RawKeyPositions<depth> diag_poss,
							  value_type *value_buffer,
							  result_buffer_type *result_buffer) noexcept : Base{value_buffer, result_buffer},
																			cartesian_{xn},
																			diag_poss_{diag_poss} {
				auto const [min_card_pos, min_card] = xn->min_card_pos(diag_poss);
				size_upper_bound_ = min_card;
				auto const [slice_now, slice_rest] = xn->discriminant().slice_index(min_card_pos);

				auto const operand_depth = xn->discriminant()[slice_now];

				dice::template_library::switch_cases<1, depth>(operand_depth, [&, slice_now = slice_now, slice_rest = slice_rest](auto operand_depth) noexcept {
					auto const operand = static_cast<NodePtr<operand_depth, htt_t, allocator_type>>(xn->operand(slice_now));

					if constexpr (operand_depth == 1) {
						switch (operand.tag()) {
							case IdentifierTag::FN: {
								static_assert(std::is_trivially_destructible_v<iterator_detail::FNIterator<1, htt_t, allocator_type>>);
								new (keypart_iter_) iterator_detail::FNIterator<1, htt_t, allocator_type>{operand.template specific_ptr<FullNode>(),
																										  &this->value_buffer_->first,
																										  0,
																										  nullptr};
								break;
							}
							case IdentifierTag::SEN: {
								if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
									static_assert(std::is_trivially_destructible_v<iterator_detail::KeyPartIterator<htt_t>>);
									new (keypart_iter_) iterator_detail::KeyPartIterator<htt_t>{operand.decode_key_part(),
																								&this->value_buffer_->first,
																								0};
								} else {
									static_assert(std::is_trivially_destructible_v<iterator_detail::SENIterator<1, htt_t, allocator_type>>);
									new (keypart_iter_) iterator_detail::SENIterator<1, htt_t, allocator_type>{operand.template specific_ptr<SingleEntryNode>(),
																											   &this->value_buffer_->first,
																											   0,
																											   nullptr};
								}

								break;
							}
							default: {
								HYPERTRIE_UNREACHABLE;
							}
						}
					} else {
						static_assert(std::is_trivially_destructible_v<iterator_detail::FNEdgeKeyPartIterator<operand_depth, htt_t, allocator_type>>);
						new (keypart_iter_) iterator_detail::FNEdgeKeyPartIterator<operand_depth, htt_t, allocator_type>{operand.template specific_ptr<FullNode>(),
																														 slice_rest,
																														 &this->value_buffer_->first,
																														 0};
					}
				});

				keypart_iter()->init();
				advance_until_result();
			}

			XNRawHashDiagonal(XNRawHashDiagonal const &other) noexcept : Base{other},
																		 cartesian_{other.cartesian_},
																		 diag_poss_{other.diag_poss_},
																		 size_upper_bound_{other.size_upper_bound_} {
				other.keypart_iter()->clone_to(keypart_iter());
			}

			XNRawHashDiagonal &operator=(XNRawHashDiagonal const &other) noexcept {
				if (this == &other) {
					return *this;
				}

				Base::operator=(other);
				cartesian_ = other.cartesian_;
				diag_poss_ = other.diag_poss_;
				size_upper_bound_ = other.size_upper_bound_;
				other.keypart_iter()->clone_to(keypart_iter());

				return *this;
			}

			XNRawHashDiagonal(XNRawHashDiagonal &&other) = delete;
			XNRawHashDiagonal &operator=(XNRawHashDiagonal &&other) = delete;

			void advance() noexcept override {
				keypart_iter()->advance();
				advance_until_result();
			}

			bool find(typename htt_t::key_part_type const key_part) noexcept override {
				commit(key_part);

				if constexpr (result_depth == 0) {
					return this->value_buffer_->second != typename htt_t::value_type{};
				} else {
					return !this->value_buffer_->second.empty();
				}
			}

			[[nodiscard]] bool ended() const noexcept override {
				return
				keypart_iter()->ended();
			}

			[[nodiscard]] bool empty() const noexcept override {
				return size_upper_bound_ == 0;
			}

			[[nodiscard]] size_t size() const noexcept override {
				return size_upper_bound_;
			}

			void clone_to(Base *dst) const noexcept override {
				new (dst) XNRawHashDiagonal{*this};
			}

			void repoint_buffers(value_type *value_buffer, result_buffer_type *result_buffer) noexcept override {
				Base::repoint_buffers(value_buffer, result_buffer);
				keypart_iter()->repoint_buffers(&value_buffer->first, nullptr);
			}
		};
	} // namespace diagonal_detail

	template<size_t diag_depth, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct RawHashDiagonal {
		static_assert(diag_depth >= 1);
		static_assert(diag_depth <= depth);

	private:
		using RawHashDiagonalBase = diagonal_detail::RawHashDiagonalBase<diag_depth, depth, htt_t, allocator_type>;

	public:
		static constexpr size_t result_depth = depth - diag_depth;
		using value_type = typename RawHashDiagonalBase::value_type;
		using diagonal_type = typename RawHashDiagonalBase::diagonal_type;
		using result_buffer_type = typename RawHashDiagonalBase::result_buffer_type;

	private:
		static consteval size_t max_size() {
			auto const max_1 = std::max(sizeof(diagonal_detail::FNRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>),
										sizeof(diagonal_detail::SENRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>));

			if constexpr (depth >= 2) {
				return std::max(max_1,
								sizeof(diagonal_detail::XNRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>));
			} else {
				return max_1;
			}
		}

		static consteval size_t max_align() {
			auto const max_1 = std::max(alignof(diagonal_detail::FNRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>),
										alignof(diagonal_detail::SENRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>));

			if constexpr (depth > 1) {
				return std::max(max_1,
								alignof(diagonal_detail::XNRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>));
			} else {
				return max_1;
			}
		}

		alignas(max_align()) std::byte inner_[max_size()];
		bool is_constructed_ = false;

		result_buffer_type result_buffer_;
		value_type value_buffer_;

		RawHashDiagonalBase const *inner() const noexcept {
			return reinterpret_cast<RawHashDiagonalBase const *>(inner_);
		}

		RawHashDiagonalBase *inner() noexcept {
			return reinterpret_cast<RawHashDiagonalBase *>(inner_);
		}

		void repoint_buffers(RawHashDiagonal const &reference) noexcept {
			inner()->repoint_buffers(&value_buffer_, &result_buffer_);

			if constexpr (result_depth > 0) {
				auto &ptr = value_buffer_.second.node_ptr();

				if (std::to_address(ptr.ptr()) == &reference.result_buffer_.sen) {
					ptr = NodePtr<result_depth, htt_t, allocator_type>{&result_buffer_.sen};
				} else if (std::to_address(ptr.ptr()) == &reference.result_buffer_.xn) {
					ptr = NodePtr<result_depth, htt_t, allocator_type>{&result_buffer_.xn};
				}
			}
		}

	public:
		RawHashDiagonal() noexcept = default;

		RawHashDiagonal(NodePtr<depth, htt_t, allocator_type> const &node,
						RawKeyPositions<depth> diag_poss) noexcept {

			assert(diag_poss.count() == diag_depth);

			if (node == nullptr) {
				return;
			}

			switch (node.tag()) {
				case IdentifierTag::FN: {
					static_assert(std::is_trivially_destructible_v<diagonal_detail::FNRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>>);
					new (inner_) diagonal_detail::FNRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>{node.template specific_ptr<FullNode>(),
																											  diag_poss,
																											  &value_buffer_,
																											  &result_buffer_};
					break;
				}
				case IdentifierTag::SEN: {
					static_assert(std::is_trivially_destructible_v<diagonal_detail::SENRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>>);

					if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						new (inner_) diagonal_detail::SENRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>{node.decode_key_part(),
																												   &value_buffer_,
																												   &result_buffer_};
					} else {
						new (inner_) diagonal_detail::SENRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>{node.template specific_ptr<SingleEntryNode>(),
																												   diag_poss,
																												   &value_buffer_,
																												   &result_buffer_};
					}
					break;
				}
				case IdentifierTag::XN: {
					if constexpr (depth > 1) {
						static_assert(std::is_trivially_destructible_v<diagonal_detail::XNRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>>);
						new (inner_) diagonal_detail::XNRawHashDiagonal<diag_depth, depth, htt_t, allocator_type>{node.template specific_ptr<CartesianNode>(),
																												  diag_poss,
																												  &value_buffer_,
																												  &result_buffer_};
						break;
					} else {
						HYPERTRIE_UNREACHABLE;
					}
				}
				case IdentifierTag::Indeterminate: {
					HYPERTRIE_UNREACHABLE;
				}
			}

			is_constructed_ = true;
		}

		RawHashDiagonal(RawHashDiagonal const &other) noexcept : is_constructed_{other.is_constructed_},
																 result_buffer_{other.result_buffer_} {

			value_buffer_.first = other.value_buffer_.first;
			if constexpr (result_depth == 0) {
				value_buffer_.second = other.value_buffer_.second;
			} else {
				value_buffer_.second = other.value_buffer_.second.clone();
			}

			if (is_constructed_) {
				other.inner()->clone_to(inner());
				repoint_buffers(other);
			}
		}

		RawHashDiagonal(RawHashDiagonal &&other) noexcept : is_constructed_{other.is_constructed_},
															result_buffer_{std::move(other.result_buffer_)},
															value_buffer_{std::move(other.value_buffer_)} {
			if (is_constructed_) {
				other.inner()->clone_to(inner());
				repoint_buffers(other);
			}
		}

		RawHashDiagonal &operator=(RawHashDiagonal const &other) noexcept {
			if (this == &other) {
				return *this;
			}

			is_constructed_ = other.is_constructed_;

			value_buffer_.first = other.value_buffer_.first;
			if constexpr (result_depth == 0) {
				value_buffer_.second = other.value_buffer_.second;
			} else {
				value_buffer_.second = other.value_buffer_.clone();
			}

			result_buffer_ = other.result_buffer_;

			if (is_constructed_) {
				other.inner()->clone_to(inner());
				repoint_buffers(other);
			}

			return *this;
		}

		RawHashDiagonal &operator=(RawHashDiagonal &&other) noexcept {
			if (this == &other) {
				return *this;
			}

			is_constructed_ = other.is_constructed_;
			value_buffer_ = std::move(other.value_buffer_);
			result_buffer_ = std::move(other.result_buffer_);

			if (is_constructed_) {
				other.inner()->clone_to(inner());
				repoint_buffers(other);
			}

			return *this;
		}

		bool find(typename htt_t::key_part_type key_part) noexcept {
			return inner()->find(key_part);
		}

		void advance() noexcept {
			inner()->advance();
		}

		RawHashDiagonal &operator++() noexcept {
			advance();
			return *this;
		}

		RawHashDiagonal operator++(int) noexcept {
			auto cpy = *this;
			this->advance();
			return cpy;
		}

		[[nodiscard]] value_type const &operator*() const noexcept {
			return value_buffer_;
		}

		[[nodiscard]] value_type &operator*() noexcept {
			return value_buffer_;
		}

		[[nodiscard]] value_type const *operator->() const noexcept {
			return &value_buffer_;
		}

		[[nodiscard]] value_type *operator->() noexcept {
			return &value_buffer_;
		}

		[[nodiscard]] value_type const &current_value() const noexcept {
			return value_buffer_;
		}

		[[nodiscard]] value_type &current_value() noexcept {
			return value_buffer_;
		}

		[[nodiscard]] typename htt_t::key_part_type current_key_part() const noexcept {
			return value_buffer_.first;
		}

		[[nodiscard]] diagonal_type const &current_diagonal() const noexcept {
			return value_buffer_.second;
		}

		[[nodiscard]] diagonal_type &current_diagonal() noexcept {
			return value_buffer_.second;
		}

		[[nodiscard]] bool ended() const noexcept {
			return !is_constructed_ || inner()->ended();
		}

		operator bool() const noexcept {
			return !ended();
		}

		bool operator==(std::default_sentinel_t) const noexcept {
			return ended();
		}

		bool operator!=(std::default_sentinel_t) const noexcept {
			return !ended();
		}

		[[nodiscard]] size_t size() const noexcept {
			if (!is_constructed_) {
				return 0;
			}

			return inner()->size();
		}

		auto operator<=>(RawHashDiagonal const &other) const noexcept {
			return this->size() <=> other.size();
		}

		[[nodiscard]] bool empty() const noexcept {
			return !is_constructed_ || inner()->empty();
		}
	};

}// namespace dice::hypertrie::internal::raw


#endif//HYPERTRIE_RAWHASHDIAGONAL_HPP
