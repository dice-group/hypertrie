#ifndef HYPERTRIE_HASHDIAGONAL_HPP
#define HYPERTRIE_HASHDIAGONAL_HPP

#include "dice/hypertrie/HypertrieContext.hpp"
#include "dice/hypertrie/Hypertrie_predeclare.hpp"
#include "dice/hypertrie/internal/raw/iteration/RawHashDiagonal.hpp"
#include "dice/template-library/switch_cases.hpp"

namespace dice::hypertrie {

	/**
	 * A Diagonal implementation for hypertries which use a map_type in htt_t that is similar to std::unordered_map (typically a hash map).
	 * The diagonal provides access to non-zero slices where given position (diag_poss) have the same key_part.
	 * <p>Example: Consider diag_poss = [0,3,4] for a depth 5 hypertrie T. T has a diagonal with key_part 11 if the slice T[11,:,:,11,11] is not empty.</p>
	 * We say "full diagonal", if diag_poss contains all possible positions (|diag_poss| = depth of hypertrie).
	 * <p>HashDiagonal has two modes of operation for a given hypertrie and diagonal positions:</p>
	 * <p>(1) Iterate through the non-zero diagonal slices.</p>
	 * <p>(2) Probe for given key_parts, if a non-zero diagonal slice exists.<p>
	 *
	 * @tparam htt_t
	 * @tparam allocator_type
	 */
	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	class HashDiagonal {
	public:
		using RawKeyPositions_t = internal::raw::RawKeyPositions<hypertrie_max_depth>;

		using key_part_type = typename htt_t::key_part_type;
		using diagonal_type = const_Hypertrie<htt_t, allocator_type>;
		using value_type = std::pair<key_part_type, diagonal_type>;

	private:
		template<size_t diag_depth, size_t depth>
		using RawHashDiagonal_t = typename internal::raw::template RawHashDiagonal<diag_depth, depth, htt_t, allocator_type>;
		using max_sized_RawHashDiagonal_t = RawHashDiagonal_t<1, hypertrie_max_depth>;

	protected:
		struct VTable {
			void (*construct)(void *location, const_Hypertrie<htt_t, allocator_type> const &hyp, RawKeyPositions_t const &diag_poss) noexcept;
			void (*copy)(void *location, void const *self) noexcept;
			void (*move)(void *location, void *self) noexcept;
			void (*destroy)(void *location) noexcept;

			value_type (*current_value)(void *self, HypertrieContext_ptr<htt_t, allocator_type> context) noexcept;

			bool (*find)(void *self, typename htt_t::key_part_type key_part) noexcept;
			void (*advance)(void *self) noexcept;
			bool (*ended)(void const *self) noexcept;
			size_t (*size)(void const *self) noexcept;
			bool (*empty)(void const *self) noexcept;

			template<size_t hypertrie_depth_ix, size_t diag_depth_ix>
			static consteval VTable make() {
				using namespace ::dice::hypertrie::internal::raw;
				using namespace ::dice::hypertrie::internal::util;

				constexpr size_t diag_depth = diag_depth_ix + 1;
				constexpr size_t hypertrie_depth = hypertrie_depth_ix + 1;
				constexpr size_t result_depth = hypertrie_depth - diag_depth;

				using RawHashDiagonal_tt = RawHashDiagonal_t<diag_depth, hypertrie_depth>;
				return VTable{
						.construct = [](void *location, const_Hypertrie<htt_t, allocator_type> const &hyp, RawKeyPositions_t const &max_sized_diag_poss) noexcept {
							if (hyp.empty()) {
								new (location) RawHashDiagonal_tt{};
							} else {
								auto const &diag_poss = reinterpret_cast<RawKeyPositions<hypertrie_depth> const &>(max_sized_diag_poss);
								new (location) RawHashDiagonal_tt{hyp.template node_ptr<hypertrie_depth>(), diag_poss};
							}
						},
						.copy = [](void *location, void const *self) noexcept {
							new (location) RawHashDiagonal_tt{*reinterpret_cast<RawHashDiagonal_tt const *>(self)};
						},
						.move = [](void *location, void *self) noexcept {
							new (location) RawHashDiagonal_tt{std::move(*reinterpret_cast<RawHashDiagonal_tt *>(self))};
						},
						.destroy = [](void *location) noexcept {
							reinterpret_cast<RawHashDiagonal_tt *>(location)->~RawHashDiagonal_tt();
						},
						.current_value = [](void *void_self, HypertrieContext_ptr<htt_t, allocator_type> context) noexcept -> value_type {
							auto *self = reinterpret_cast<RawHashDiagonal_tt *>(void_self);

							auto const key_part = self->current_key_part();
							auto &diagonal = self->current_diagonal();

							if constexpr (diag_depth == hypertrie_depth) {
								return value_type{key_part, const_Hypertrie<htt_t, allocator_type>::from_scalar(diagonal)};
							} else {
								auto const ownership = diagonal.ownership();
								auto node = diagonal.release_node_ptr();

								return value_type{key_part, const_Hypertrie<htt_t, allocator_type>{result_depth,
																								   context,
																								   ownership,
																								   node}};
							}
						},
						.find = [](void *self, typename htt_t::key_part_type key_part) noexcept -> bool {
							return reinterpret_cast<RawHashDiagonal_tt *>(self)->find(key_part);
						},
						.advance = [](void *self) noexcept {
							reinterpret_cast<RawHashDiagonal_tt *>(self)->advance();
						},
						.ended = [](void const *self) noexcept -> bool {
							return reinterpret_cast<RawHashDiagonal_tt const *>(self)->ended();
						},
						.size = [](void const *self) noexcept -> size_t {
							return reinterpret_cast<RawHashDiagonal_tt const *>(self)->size();
						},
						.empty = [](void const *self) noexcept -> bool {
							return reinterpret_cast<RawHashDiagonal_tt const *>(self)->empty();
						}};
			}
		};

		// depth = 1 ... hypertrie_max_depth
		// diag_depth = 1 ... depth
		using VTables_t = std::array<std::array<VTable, hypertrie_max_depth>, hypertrie_max_depth>;

		template<size_t hypertrie_depth_ix, size_t ...diag_depth_ixs>
		static consteval std::array<VTable, hypertrie_max_depth> make_vtable_for_depth(std::index_sequence<diag_depth_ixs...>) {
			return {VTable::template make<hypertrie_depth_ix, diag_depth_ixs>()...};
		}

		template<size_t ...hypertrie_depth_ixs>
		static consteval VTables_t make_vtables(std::index_sequence<hypertrie_depth_ixs...>) {
			return {make_vtable_for_depth<hypertrie_depth_ixs>(std::make_index_sequence<hypertrie_depth_ixs + 1>{})...};
		}

		static constexpr VTables_t vtables_ = make_vtables(std::make_index_sequence<hypertrie_max_depth>{});
		VTable const *vtable_ = nullptr;

		HypertrieContext_ptr<htt_t, allocator_type> context_;
		value_type current_value_;

		alignas(max_sized_RawHashDiagonal_t) std::byte inner_[sizeof(max_sized_RawHashDiagonal_t)];

		void drop() noexcept {
			if (vtable_ != nullptr) {
				vtable_->destroy(inner_);
			}
		}

	public:
		/**
		 * Constructor for a HashDiagonal.
		 * @param hypertrie The hypertrie from that the diagonal is extracted.
		 * @param diag_poss The Key position of the diagonal.
		 */
		HashDiagonal(const_Hypertrie<htt_t, allocator_type> const &hypertrie, const RawKeyPositions_t &diag_poss) noexcept
			: vtable_{&vtables_[hypertrie.depth() - 1][diag_poss.count() - 1]},
			  context_{hypertrie.context()},
			  current_value_{typename htt_t::key_part_type{}, const_Hypertrie<htt_t, allocator_type>{hypertrie.depth() - diag_poss.count()}} {

			assert(hypertrie.depth() >= 1);
			assert(diag_poss.count() >= 1);

			vtable_->construct(inner_, hypertrie, diag_poss);
			current_value_ = vtable_->current_value(inner_, context_);
		}

		HashDiagonal(HashDiagonal const &other) noexcept : vtable_{other.vtable_},
														   context_{other.context_},
														   current_value_{other.current_value_} {
			vtable_->copy(inner_, other.inner_);
		}

		HashDiagonal(HashDiagonal &&other) noexcept : vtable_{std::exchange(other.vtable_, nullptr)},
													  context_{other.context_},
													  current_value_{std::move(other.current_value_)} {
			vtable_->move(inner_, other.inner_);
		}

		HashDiagonal &operator=(HashDiagonal const &other) noexcept {
			if (this == &other) {
				return *this;
			}

			drop();
			vtable_ = other.vtable_;
			context_ = other.context_;
			current_value_ = other.current_value_;
			vtable_->copy(inner_, other.inner_);

			return *this;
		}

		HashDiagonal &operator=(HashDiagonal &&other) noexcept {
			if (this == &other) {
				return *this;
			}

			drop();
			vtable_ = std::exchange(other.vtable_, nullptr);
			context_ = other.context_;
			current_value_ = std::move(other.current_value_);
			vtable_->move(inner_, other.inner_);

			return *this;
		}

		~HashDiagonal() noexcept {
			drop();
		}

		[[nodiscard]] value_type const &current_value() const noexcept {
			return current_value_;
		}

		/**
		 * @return The key part for which the Diagonal iterator currently exposes the diagonal slice.
		 */
		[[nodiscard]] key_part_type current_key_part() const noexcept {
			return current_value_.first;
		}

		/**
		 * @return the hypertrie that results from slicing the the hypertrie with current_key_part() at diagonal positions (diagonal_poss).
		 */
		[[nodiscard]] diagonal_type const &current_diagonal() const noexcept {
			return current_value_.second;
		}

		/**
		 * Try to find the non-zero diagonal for key_part
		 * @return if there is a non-zero diagonal for key_part
		 */
		[[nodiscard]] bool find(key_part_type key_part) noexcept {
			auto ret = vtable_->find(inner_, key_part);
			current_value_ = vtable_->current_value(inner_, context_);
			return ret;
		}

		void advance() noexcept {
			vtable_->advance(inner_);
			current_value_ = vtable_->current_value(inner_, context_);
		}

		HashDiagonal &operator++() noexcept {
			advance();
			return *this;
		}

		HashDiagonal operator++(int) noexcept {
			auto cpy = *this;
			this->advance();
			return cpy;
		}

		value_type const &operator*() const noexcept {
			return current_value_;
		}

		value_type const *operator->() const noexcept {
			return &current_value_;
		}

		/**
		 * @return if iterator is at end
		 */
		[[nodiscard]] bool ended() const noexcept {
			return vtable_->ended(inner_);
		}

		/**
		 * inverse of ended()
		 */
		[[nodiscard]] explicit operator bool() const noexcept {
			return !ended();
		}

		bool operator==(std::default_sentinel_t) const noexcept {
			return ended();
		}

		bool operator!=(std::default_sentinel_t) const noexcept {
			return !ended();
		}

		/**
		 * @return an upper bound for the number of diagonal slices
		 */
		[[nodiscard]] size_t size() const noexcept {
			return vtable_->size(inner_);
		}

		/**
		 * @return if the hypertrie being iterated is empty
		 */
		[[nodiscard]] bool empty() const noexcept {
			return vtable_->empty(inner_);
		}

		/**
		 * comparison by size
		 */
		auto operator<=>(HashDiagonal const &other) const noexcept {
			return this->size() <=> other.size();
		}
	};
}// namespace dice::hypertrie

#endif//HYPERTRIE_HASHDIAGONAL_HPP
