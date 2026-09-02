#ifndef HYPERTRIE_RAWITERATOR_HPP
#define HYPERTRIE_RAWITERATOR_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/RawDiagonalPositions.hpp"
#include "dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"
#include "dice/hypertrie/internal/raw/node_context/SliceResult.hpp"
#include "dice/template-library/integral_template_tuple.hpp"
#include "dice/template-library/switch_cases.hpp"
#include "dice/hypertrie/internal/util/Unreachable.hpp"

namespace dice::hypertrie::internal::raw {

	namespace iterator_detail {

		template<HypertrieTrait htt_t>
		struct NodeIteratorBase {
		protected:
			typename htt_t::key_part_type *key_buffer_base_;
			size_t key_buffer_off_;
			typename htt_t::value_type *value_buffer_;

			[[nodiscard]] typename htt_t::key_part_type *key_buffer() const noexcept {
				return key_buffer_base_ + key_buffer_off_;
			}

			[[nodiscard]] typename htt_t::value_type *value_buffer() const noexcept {
				return value_buffer_;
			}

			void repoint_local_buffers(typename htt_t::key_part_type *key_buffer_base, typename htt_t::value_type *value_buffer) noexcept {
				key_buffer_base_ = key_buffer_base;
				value_buffer_ = value_buffer;
			}

		public:
			NodeIteratorBase(typename htt_t::key_part_type *key_buffer_base, size_t key_buffer_off, typename htt_t::value_type *value_buffer) : key_buffer_base_{key_buffer_base},
																																				key_buffer_off_{key_buffer_off},
																																				value_buffer_{value_buffer} {
			}

			/**
			 * Should be called once after construction to write the initial value into the buffers.
			 */
			virtual void init() noexcept = 0;

			/**
			 * Advances the iterator onto the next element and writes to the buffers.
			 */
			virtual void advance() noexcept = 0;

			/**
			 * @return if this iterator is at the end
			 */
			[[nodiscard]] virtual bool ended() const noexcept = 0;

			/**
			 * Clones the current iterator into dst this should not commit anything to the buffers.
			 */
			virtual void clone_to(NodeIteratorBase *dst) const noexcept = 0;

			/**
			 * Point all internal buffers to a new location
			 */
			virtual void repoint_buffers(typename htt_t::key_part_type *key_buffer_base, typename htt_t::value_type *value_buffer) noexcept = 0;
		};

		template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
			requires (depth > 1)
		struct XNIterator;

		template<HypertrieTrait htt_t>
		struct KeyPartIterator;

		template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
			requires (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
		struct SENIterator;

		template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
			requires (depth > 0)
		struct FNIterator;

		template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
			requires (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
		struct SENIterator final : NodeIteratorBase<htt_t> {
			using Base = NodeIteratorBase<htt_t>;

		private:
			bool ended_ = false;
			SingleEntry<depth, htt_t> entry_;

		public:
			SENIterator(SENPtr<depth, htt_t, allocator_type> sen,
						typename htt_t::key_part_type *key_buffer_base,
						size_t key_buffer_off,
						typename htt_t::value_type *value_buffer) noexcept : Base{key_buffer_base, key_buffer_off, value_buffer},
																			 entry_{SingleEntry<depth, htt_t>{sen->key(), sen->value()}} {
			}

			void init() noexcept override {
				std::copy(entry_.key().begin(), entry_.key().end(), this->key_buffer());
				if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
					if (this->value_buffer() != nullptr) {
						*this->value_buffer() = entry_.value();
					}
				}
			}

			void advance() noexcept override {
				ended_ = true;
			}

			[[nodiscard]] bool ended() const noexcept override {
				return ended_;
			}

			void clone_to(Base *dst) const noexcept override {
				new (dst) SENIterator{*this};
			}

			void repoint_buffers(typename htt_t::key_part_type *key_buffer_base, typename htt_t::value_type *value_buffer) noexcept override {
				this->repoint_local_buffers(key_buffer_base, value_buffer);
			}
		};

		template<HypertrieTrait htt_t>
		struct ScalarIterator final : NodeIteratorBase<htt_t> {
			using Base = NodeIteratorBase<htt_t>;

		private:
			typename htt_t::value_type scalar_;
			bool ended_ = false;

		public:
			explicit ScalarIterator(typename htt_t::value_type scalar, typename htt_t::value_type *value_buffer) : Base{nullptr, 0, value_buffer},
																												   scalar_{scalar} {
				assert(scalar != typename htt_t::value_type{});

				if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
					assert(value_buffer != nullptr);
				}
			}

			void init() noexcept override {
				if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
					*this->value_buffer() = scalar_;
				}
			}

			void advance() noexcept override {
				ended_ = true;
			}

			[[nodiscard]] bool ended() const noexcept override {
				return ended_;
			}

			void clone_to(Base *dst) const noexcept override {
				new (dst) ScalarIterator{*this};
			}

			void repoint_buffers(typename htt_t::key_part_type *key_buffer_base, typename htt_t::value_type *value_buffer) noexcept override {
				this->repoint_local_buffers(key_buffer_base, value_buffer);
			}
		};

		template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
			requires (depth > 0)
		struct FNIterator final : NodeIteratorBase<htt_t> {
			using Base = NodeIteratorBase<htt_t>;

		private:
			static consteval size_t max_size() {
				auto max_size = sizeof(FNIterator<depth - 1, htt_t, allocator_type>);

				if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					max_size = std::max(max_size, sizeof(KeyPartIterator<htt_t>));
				} else {
					max_size = std::max(max_size, sizeof(SENIterator<depth - 1, htt_t, allocator_type>));
				}

				if constexpr (depth - 1 > 1) {
					max_size = std::max(max_size, sizeof(XNIterator<depth - 1, htt_t, allocator_type>));
				}

				return max_size;
			}

			static consteval size_t max_align() {
				auto max_align = alignof(FNIterator<depth - 1, htt_t, allocator_type>);

				if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					max_align = std::max(max_align, alignof(KeyPartIterator<htt_t>));
				} else {
					max_align = std::max(max_align, alignof(SENIterator<depth - 1, htt_t, allocator_type>));
				}

				if constexpr (depth - 1 > 1) {
					max_align = std::max(max_align, alignof(XNIterator<depth - 1, htt_t, allocator_type>));
				}

				return max_align;
			}

			using children_iterator = typename FullNode<depth, htt_t, allocator_type>::single_dim_edges_type::const_iterator;

			struct children_iterator_pair {
				children_iterator current;
				children_iterator end;
			};

			children_iterator_pair children_iters_;
			alignas(max_align()) std::byte child_iter_[max_size()];

			Base *child_iter() noexcept {
				return std::launder(reinterpret_cast<Base *>(child_iter_));
			}

			Base const *child_iter() const noexcept {
				return std::launder(reinterpret_cast<Base const *>(child_iter_));
			}

			void setup() {
				if (ended()) {
					return;
				}

				auto const child_ptr = children_iters_.current->second;
				switch (child_ptr.tag()) {
					case IdentifierTag::FN: {
						new (child_iter_) FNIterator<depth - 1, htt_t, allocator_type>{child_ptr.template specific_ptr<FullNode>(),
																					   this->key_buffer_base_,
																					   this->key_buffer_off_ + 1,
																					   this->value_buffer_};

						break;
					}
					case IdentifierTag::SEN: {
						if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
							new (child_iter_) KeyPartIterator<htt_t>{child_ptr.decode_key_part(),
																	 this->key_buffer_base_,
																	 this->key_buffer_off_ + 1};
						} else {
							new (child_iter_) SENIterator<depth - 1, htt_t, allocator_type>{child_ptr.template specific_ptr<SingleEntryNode>(),
																							this->key_buffer_base_,
																							this->key_buffer_off_ + 1,
																							this->value_buffer_};
						}
						break;
					}
					case IdentifierTag::XN: {
						if constexpr (depth - 1 > 1) {
							new (child_iter_) XNIterator<depth - 1, htt_t, allocator_type>{child_ptr.template specific_ptr<CartesianNode>(),
																						   this->key_buffer_base_,
																						   this->key_buffer_off_ + 1,
																						   this->value_buffer_};
							break;
						} else {
							HYPERTRIE_UNREACHABLE;
						}
					}
					case IdentifierTag::Indeterminate: {
						HYPERTRIE_UNREACHABLE;
					}
				}
			}

		public:
			FNIterator(FNPtr<depth, htt_t, allocator_type> fn,
					   typename htt_t::key_part_type *key_buffer_base,
					   size_t key_buffer_off,
					   typename htt_t::value_type *value_buffer) : Base{key_buffer_base, key_buffer_off, value_buffer} {

				children_iters_.current = fn->edges(0).cbegin();
				children_iters_.end = fn->edges(0).cend();
				setup();
			}

			FNIterator(FNIterator const &other) noexcept : Base{other.key_buffer_base_, other.key_buffer_off_, other.value_buffer_},
														   children_iters_{other.children_iters_} {

				other.child_iter()->clone_to(child_iter());
			}

			FNIterator &operator=(FNIterator const &other) noexcept {
				if (this == &other) {
					return *this;
				}

				Base::operator=(other);
				children_iters_ = other.children_iters_;
				other.child_iter()->clone_to(child_iter());

				return *this;
			}

			FNIterator(FNIterator &&other) = delete;
			FNIterator &operator=(FNIterator &&other) = delete;

			void init() noexcept override {
				if (ended()) {
					return;
				}

				this->key_buffer()[0] = children_iters_.current->first;
				child_iter()->init();
			}

			void advance() noexcept override {
				child_iter()->advance();

				if (child_iter()->ended()) {
					if (children_iters_.current == children_iters_.end) {
						return;
					}

					++children_iters_.current;
					setup();
					init();
				}
			}

			[[nodiscard]] bool ended() const noexcept override {
				return children_iters_.current == children_iters_.end;
			}

			void clone_to(Base *dst) const noexcept override {
				new (dst) FNIterator{*this};
			}

			void repoint_buffers(typename htt_t::key_part_type *key_buffer_base, typename htt_t::value_type *value_buffer) noexcept override {
				this->repoint_local_buffers(key_buffer_base, value_buffer);
				child_iter()->repoint_buffers(key_buffer_base, value_buffer);
			}
		};

		template<HypertrieTrait htt_t, ByteAllocator allocator_type>
		struct FNIterator<1, htt_t, allocator_type> final : NodeIteratorBase<htt_t> {
			using Base = NodeIteratorBase<htt_t>;

		private:
			using children_iterator = typename FullNode<1, htt_t, allocator_type>::single_dim_edges_type::const_iterator;

			struct children_iterator_pair {
				children_iterator current;
				children_iterator end;
			};

			children_iterator_pair children_iters_;

		public:
			FNIterator(FNPtr<1, htt_t, allocator_type> fn,
					   typename htt_t::key_part_type *key_buffer_base,
					   size_t key_buffer_off,
					   typename htt_t::value_type *value_buffer) noexcept : Base{key_buffer_base, key_buffer_off, value_buffer},
																			children_iters_{.current = fn->edges().cbegin(),
																							.end = fn->edges().cend()} {
			}

			void init() noexcept override {
				if (ended()) {
					return;
				}

				if constexpr (HypertrieTrait_bool_valued<htt_t>) {
					this->key_buffer()[0] = *children_iters_.current;
				} else {
					this->key_buffer()[0] = children_iters_.current->first;

					if (this->value_buffer() != nullptr) {
						*this->value_buffer() = children_iters_.current->second;
					}
				}
			}

			void advance() noexcept override {
				++children_iters_.current;
				init();
			}

			[[nodiscard]] bool ended() const noexcept override {
				return children_iters_.current == children_iters_.end;
			}

			void clone_to(Base *dst) const noexcept override {
				new (dst) FNIterator{*this};
			}

			void repoint_buffers(typename htt_t::key_part_type *key_buffer_base, typename htt_t::value_type *value_buffer) noexcept override {
				this->repoint_local_buffers(key_buffer_base, value_buffer);
			}
		};

		template<HypertrieTrait htt_t>
		struct KeyPartIterator final : NodeIteratorBase<htt_t> {
			using Base = NodeIteratorBase<htt_t>;

		private:
			bool ended_ = false;
			typename htt_t::key_part_type key_part_;

		public:
			KeyPartIterator(typename htt_t::key_part_type key_part,
							typename htt_t::key_part_type *key_buffer_base,
							size_t key_buffer_off) noexcept : Base{key_buffer_base, key_buffer_off, nullptr},
															  key_part_{key_part} {
			}

			void init() noexcept override {
				this->key_buffer()[0] = key_part_;
			}

			void advance() noexcept override {
				ended_ = true;
			}

			[[nodiscard]] bool ended() const noexcept override {
				return ended_;
			}

			void clone_to(Base *dst) const noexcept override {
				new (dst) KeyPartIterator{*this};
			}

			void repoint_buffers(typename htt_t::key_part_type *key_buffer_base, typename htt_t::value_type *value_buffer) noexcept override {
				this->repoint_local_buffers(key_buffer_base, value_buffer);
			}
		};

		// TODO optimize size, very big
		template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
			requires (depth > 1)
		struct XNIterator final : NodeIteratorBase<htt_t> {
			using Base = NodeIteratorBase<htt_t>;

		private:
			static consteval size_t max_size() {
				auto const max_1 = sizeof(FNIterator<depth - 1, htt_t, allocator_type>);

				if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
					return std::max(max_1, sizeof(KeyPartIterator<htt_t>));
				} else{
					return std::max(max_1, sizeof(SENIterator<1, htt_t, allocator_type>));
				}
			}

			static consteval size_t max_align() {
				auto const max_1 = alignof(FNIterator<depth - 1, htt_t, allocator_type>);

				if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
					return std::max(max_1, alignof(KeyPartIterator<htt_t>));
				} else{
					return std::max(max_1, alignof(SENIterator<1, htt_t, allocator_type>));
				}
			}

			struct child_iterator_pair {
				alignas(max_align()) std::byte begin_[max_size()];
				alignas(max_align()) std::byte current_[max_size()];

				Base *begin() noexcept {
					return reinterpret_cast<Base *>(begin_);
				}

				Base const *begin() const noexcept {
					return reinterpret_cast<Base const *>(begin_);
				}

				Base *current() noexcept {
					return reinterpret_cast<Base *>(current_);
				}

				Base const *current() const noexcept {
					return reinterpret_cast<Base const *>(current_);
				}
			};

			std::array<child_iterator_pair, depth> iters_;
			size_t iter_max_init_ix_;

			void inc_rek(size_t index) {
				auto &child_iter = iters_[index];

				child_iter.current()->advance();
				if (child_iter.current()->ended() && index > 0) {
					child_iter.begin()->clone_to(child_iter.current());

					child_iter.current()->init();
					inc_rek(index - 1);
				}
			}

		public:
			XNIterator(XNPtr<depth, htt_t, allocator_type> xn,
					   typename htt_t::key_part_type *key_buffer_base,
					   size_t key_buffer_off,
					   typename htt_t::value_type *value_buffer) noexcept : Base{key_buffer_base, key_buffer_off, value_buffer} {

				std::array<typename htt_t::value_type *, depth> value_buffers{}; {
					if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
						// must be xfix cartesian
						auto const high_order_operand_ix = xn->get_xfix_high_order_operand_index();
						assert(high_order_operand_ix.has_value());
						value_buffers[*high_order_operand_ix] = this->value_buffer_;
					}
				};

				xn->for_each_operand([&, write_ix = size_t{0}]<size_t cur, size_t operand_depth>(NodePtr<operand_depth, htt_t, allocator_type> const &operand) mutable noexcept {
					if constexpr (operand_depth > 0) {
						switch (operand.tag()) {
							case IdentifierTag::FN: {
								new (iters_[cur].begin()) FNIterator<operand_depth, htt_t, allocator_type>{operand.template specific_ptr<FullNode>(),
																										   this->key_buffer_base_,
																										   this->key_buffer_off_ + write_ix,
																										   value_buffers[cur]};

								iters_[cur].begin()->clone_to(iters_[cur].current());
								break;
							}
							case IdentifierTag::SEN: {
								if constexpr (operand_depth == 1) {
									if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
										new (iters_[cur].begin()) KeyPartIterator<htt_t>{operand.decode_key_part(),
																						 this->key_buffer_base_,
																						 this->key_buffer_off_ + write_ix};
									} else {
										new (iters_[cur].begin()) SENIterator<1, htt_t, allocator_type>{operand.template specific_ptr<SingleEntryNode>(),
																										this->key_buffer_base_,
																										this->key_buffer_off_ + write_ix,
																										nullptr};
									}

									iters_[cur].begin()->clone_to(iters_[cur].current());
									break;
								} else {
									HYPERTRIE_UNREACHABLE;
								}
							}
							default: {
								HYPERTRIE_UNREACHABLE;
							}
						}

						write_ix += operand_depth;
						iter_max_init_ix_ = cur;
					}
				});
			}

			XNIterator(XNIterator const &other) noexcept : Base{other},
														   iter_max_init_ix_{other.iter_max_init_ix_} {

				for (size_t ix = 0; ix <= iter_max_init_ix_; ++ix) {
					other.iters_[ix].begin()->clone_to(iters_[ix].begin());
					other.iters_[ix].current()->clone_to(iters_[ix].current());
				}
			}

			XNIterator &operator=(XNIterator const &other) noexcept {
				if (this == &other) {
					return *this;
				}

				Base::operator=(other);
				iter_max_init_ix_ = other.iter_max_init_ix_;

				for (size_t ix = 0; ix <= iter_max_init_ix_; ++ix) {
					other.iters_[ix].begin()->clone_to(iters_[ix].begin());
					other.iters_[ix].current()->clone_to(iters_[ix].current());
				}

				return *this;
			}

			XNIterator(XNIterator &&other) = delete;
			XNIterator &operator=(XNIterator &&other) = delete;

			void init() noexcept override {
				for (size_t ix = 0; ix <= iter_max_init_ix_; ++ix) {
					iters_[ix].current()->init();
				}
			}

			void advance() noexcept override {
				inc_rek(iter_max_init_ix_);
			}

			[[nodiscard]] bool ended() const noexcept override {
				return iters_[0].current()->ended();
			}

			void clone_to(Base *dst) const noexcept override {
				new (dst) XNIterator{*this};
			}

			void repoint_buffers(typename htt_t::key_part_type *key_buffer_base, typename htt_t::value_type *value_buffer) noexcept override {
				this->repoint_local_buffers(key_buffer_base, value_buffer);

				for (size_t ix = 0; ix <= iter_max_init_ix_; ++ix) {
					iters_[ix].begin()->repoint_buffers(key_buffer_base, value_buffer);
					iters_[ix].current()->repoint_buffers(key_buffer_base, value_buffer);
				}
			}
		};

		/**
		 * This is only used for RawHashDiagonal
		 */
		template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
			requires (depth > 0)
		struct FNEdgeKeyPartIterator final : NodeIteratorBase<htt_t> {
			using Base = NodeIteratorBase<htt_t>;

		private:
			using children_iterator = typename FullNode<depth, htt_t, allocator_type>::single_dim_edges_type::const_iterator;

			struct children_iterator_pair {
				children_iterator current;
				children_iterator end;
			};

			children_iterator_pair children_iters_;

		public:
			FNEdgeKeyPartIterator(FNPtr<depth, htt_t, allocator_type> fn,
								  size_t pos,
								  typename htt_t::key_part_type *key_buffer_base,
								  size_t key_buffer_off) noexcept : Base{key_buffer_base, key_buffer_off, nullptr},
																	children_iters_{.current = fn->edges(pos).cbegin(),
																					.end = fn->edges(pos).cend()} {

				assert(pos < depth);
			}

			void init() noexcept override {
				if (ended()) {
					return;
				}

				if constexpr (depth == 1 && HypertrieTrait_bool_valued<htt_t>) {
					this->key_buffer()[0] = *children_iters_.current;
				} else {
					this->key_buffer()[0] = children_iters_.current->first;
				}
			}

			void advance() noexcept override {
				++children_iters_.current;
				init();
			}

			[[nodiscard]] bool ended() const noexcept override {
				return children_iters_.current == children_iters_.end;
			}

			void clone_to(Base *dst) const noexcept override {
				new (dst) FNEdgeKeyPartIterator{*this};
			}

			void repoint_buffers(typename htt_t::key_part_type *key_buffer_base, typename htt_t::value_type *value_buffer) noexcept override {
				this->repoint_local_buffers(key_buffer_base, value_buffer);
			}
		};
	} // namespace iterator_detail

	template<size_t depth, bool use_raw_key, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct RawIterator {
	private:
		using value_storage_type = std::conditional_t<use_raw_key, SingleEntry<depth, htt_t>, NonZeroEntry<htt_t>>;
	public:
		using value_type = value_storage_type const;
		using reference = value_type &;
		using pointer = value_type *;
		using iterator_category = std::input_iterator_tag;
		using difference_type = std::ptrdiff_t;

	private:
		static consteval size_t max_size() {
			if constexpr (depth == 0) {
				return sizeof(iterator_detail::ScalarIterator<htt_t>);
			} else {
				auto max_size = sizeof(iterator_detail::FNIterator<depth, htt_t, allocator_type>);

				if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					max_size = std::max(max_size, sizeof(iterator_detail::KeyPartIterator<htt_t>));
				} else {
					max_size = std::max(max_size, sizeof(iterator_detail::SENIterator<depth, htt_t, allocator_type>));
				}

				if constexpr (depth > 1) {
					max_size = std::max(max_size, sizeof(iterator_detail::XNIterator<depth, htt_t, allocator_type>));
				}

				return max_size;
			}
		}

		static consteval size_t max_align() {
			if constexpr (depth == 0) {
				return alignof(iterator_detail::ScalarIterator<htt_t>);
			} else {
				auto max_align = alignof(iterator_detail::FNIterator<depth, htt_t, allocator_type>);

				if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					max_align = std::max(max_align, alignof(iterator_detail::KeyPartIterator<htt_t>));
				} else {
					max_align = std::max(max_align, alignof(iterator_detail::SENIterator<depth, htt_t, allocator_type>));
				}

				if constexpr (depth > 1) {
					max_align = std::max(max_align, alignof(iterator_detail::XNIterator<depth, htt_t, allocator_type>));
				}

				return max_align;
			}
		}

		value_storage_type value_ = []() noexcept(use_raw_key) {
			if constexpr (use_raw_key) {
				return value_storage_type{};
			} else {
				return value_storage_type::make_with_defaulted_key(depth, typename htt_t::value_type{1});
			}
		}();

		bool is_constructed_ = false;
		alignas(max_align()) std::byte inner_iter_[max_size()];

		iterator_detail::NodeIteratorBase<htt_t> *inner_iter() noexcept {
			return reinterpret_cast<iterator_detail::NodeIteratorBase<htt_t> *>(inner_iter_);
		}

		iterator_detail::NodeIteratorBase<htt_t> const *inner_iter() const noexcept {
			return reinterpret_cast<iterator_detail::NodeIteratorBase<htt_t> const *>(inner_iter_);
		}

		void repoint_buffers() noexcept {
			typename htt_t::key_part_type *key_buffer_ptr = value_.key().data();
			typename htt_t::value_type *value_buffer_ptr = [&]() -> typename htt_t::value_type * {
				if constexpr (HypertrieTrait_bool_valued<htt_t>) {
					return nullptr;
				} else {
					return &value_.value_mut();
				}
			}();

			inner_iter()->repoint_buffers(key_buffer_ptr, value_buffer_ptr);
		}

		[[nodiscard]] std::pair<typename htt_t::key_part_type *, typename htt_t::value_type *> get_buffer_ptrs() noexcept {
			typename htt_t::key_part_type *key_buffer_ptr = value_.key().data();
			typename htt_t::value_type *value_buffer_ptr = [&]() -> typename htt_t::value_type * {
				if constexpr (HypertrieTrait_bool_valued<htt_t>) {
					return nullptr;
				} else {
					return &value_.value_mut();
				}
			}();

			return std::make_pair(key_buffer_ptr, value_buffer_ptr);
		}

	public:
		RawIterator() noexcept = default;

		explicit RawIterator(typename htt_t::value_type scalar) noexcept requires (depth == 0) : is_constructed_{true} {
			auto [_, value_buffer_ptr] = get_buffer_ptrs();

			new (inner_iter_) iterator_detail::ScalarIterator<htt_t>{scalar, value_buffer_ptr};
			inner_iter()->init();
		}

		explicit RawIterator(NodePtr<depth, htt_t, allocator_type> const &node) noexcept requires (depth > 0) {
			if (node == nullptr) {
				return;
			}

			auto [key_buffer_ptr, value_buffer_ptr] = get_buffer_ptrs();

			switch (node.tag()) {
				case IdentifierTag::FN: {
					static_assert(std::is_trivially_destructible_v<iterator_detail::FNIterator<depth,  htt_t, allocator_type>>);
					new (inner_iter_) iterator_detail::FNIterator<depth, htt_t, allocator_type>{node.template specific_ptr<FullNode>(),
																								key_buffer_ptr,
																								0,
																								value_buffer_ptr};
					break;
				}
				case IdentifierTag::SEN: {
					if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						static_assert(std::is_trivially_destructible_v<iterator_detail::KeyPartIterator<htt_t>>);
						new (inner_iter_) iterator_detail::KeyPartIterator<htt_t>{node.decode_key_part(),
																				  key_buffer_ptr,
																				  0};
					} else {
						static_assert(std::is_trivially_destructible_v<iterator_detail::SENIterator<depth, htt_t, allocator_type>>);
						new (inner_iter_) iterator_detail::SENIterator<depth, htt_t, allocator_type>{node.template specific_ptr<SingleEntryNode>(),
																									 key_buffer_ptr,
																									 0,
																									 value_buffer_ptr};
					}
					break;
				}
				case IdentifierTag::XN: {
					if constexpr (depth > 1) {
						static_assert(std::is_trivially_destructible_v<iterator_detail::XNIterator<depth, htt_t, allocator_type>>);
						new (inner_iter_) iterator_detail::XNIterator<depth, htt_t, allocator_type>{node.template specific_ptr<CartesianNode>(),
																									key_buffer_ptr,
																									0,
																									value_buffer_ptr};
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
			inner_iter()->init();
		}

		RawIterator(RawIterator const &other) noexcept(use_raw_key) : value_{other.value_},
																	  is_constructed_{other.is_constructed_} {

			if (is_constructed_) {
				other.inner_iter()->clone_to(inner_iter());
				repoint_buffers();
			}
		}

		RawIterator(RawIterator &&other) noexcept : value_{std::move(other.value_)},
													is_constructed_{other.is_constructed_} {

			if (is_constructed_) {
				other.inner_iter()->clone_to(inner_iter());
				repoint_buffers();
			}
		}

		RawIterator &operator=(RawIterator const &other) noexcept(use_raw_key) {
			if (this == &other) {
				return *this;
			}

			value_ = other.value_;
			is_constructed_ = other.is_constructed_;

			if (is_constructed_) {
				other.inner_iter()->clone_to(inner_iter());
				repoint_buffers();
			}

			return *this;
		}

		RawIterator &operator=(RawIterator &&other) noexcept {
			if (this == &other) {
				return *this;
			}

			value_ = std::move(other.value_);
			is_constructed_ = other.is_constructed_;

			if (is_constructed_) {
				other.inner_iter()->clone_to(inner_iter());
				repoint_buffers();
			}

			return *this;
		}

		void advance() noexcept {
			inner_iter()->advance();
		}

		[[nodiscard]] bool ended() const noexcept {
			return !is_constructed_ || inner_iter()->ended();
		}

		[[nodiscard]] reference value() const noexcept {
			return value_;
		}

		RawIterator &operator++() noexcept {
			this->advance();
			return *this;
		}

		RawIterator operator++(int) noexcept(use_raw_key) {
			auto cpy = *this;
			this->advance();
			return cpy;
		}

		reference operator*() const noexcept {
			return value_;
		}

		pointer operator->() const noexcept {
			return &value_;
		}

		bool operator==(std::default_sentinel_t) const noexcept {
			return this->ended();
		}

		bool operator!=(std::default_sentinel_t) const noexcept {
			return !this->ended();
		}

		explicit operator bool() const noexcept {
			return !this->ended();
		}
	};

};// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_RAWITERATOR_HPP
