#ifndef HYPERTRIE_WITHEDGES_HPP
#define HYPERTRIE_WITHEDGES_HPP

#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/hypertrie_allocator_trait.hpp"
#include "dice/hypertrie/internal/raw/RawDiagonalPositions.hpp"
#include "dice/hypertrie/internal/raw/node/Identifier.hpp"

#include "dice/hypertrie/internal/commons/PosType.hpp"

#include <limits>

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct WithEdges {
		using ht_allocator_trait = hypertrie_allocator_trait<allocator_type>;
		using value_type = typename htt_t::value_type;
		using key_part_type = typename htt_t::key_part_type;

		using ChildType = std::conditional_t<(depth > 1),
											 RawIdentifier<depth - 1, htt_t>,
											 value_type>;
		using collection_alloc = std::conditional_t<((depth == 1) and htt_t::is_bool_valued),
													typename ht_allocator_trait::template rebind_alloc<key_part_type>,
													typename ht_allocator_trait::template rebind_alloc<std::pair<typename htt_t::key_part_type, ChildType>>>;

		// allocator might need to be cast to specific type for set/map
		using ChildrenType = std::conditional_t<((depth == 1) and htt_t::is_bool_valued),
												typename htt_t::template set_type<key_part_type, collection_alloc>,
												typename htt_t::template map_type<typename htt_t::key_part_type, ChildType, collection_alloc>>;

		using EdgesType = std::conditional_t<(depth > 1),
											 std::array<ChildrenType, depth>,
											 ChildrenType>;

	protected:
		EdgesType edges_;

		/**
		 * Can be used with an index_sequence to generate multiple ChildrenType objects.
		 * @param alloc The allocator to pass to the ChildrenType constructor.
		 * @return
		 */
		template<size_t>
		static auto createChild(allocator_type const &alloc) { return ChildrenType(alloc); }

		template<size_t N, size_t... Is>
		static auto fill(allocator_type const &alloc, std::index_sequence<Is...>) noexcept {
			return std::array<ChildrenType, N>{createChild<Is>(alloc)...};
		}

		template<size_t N>
		static std::array<ChildrenType, N> fill(allocator_type const &alloc) noexcept {
			return fill<N>(alloc, std::make_index_sequence<N>());
		}

		static EdgesType init(allocator_type const &alloc) noexcept {
			if constexpr (depth == 1) {
				return EdgesType(alloc);
			} else {
				return fill<depth>(alloc);
			}
		}

	public:
		// TODO: does with make any problems?
		WithEdges() = delete;// default

		explicit WithEdges(allocator_type const &alloc) : edges_(init(alloc)) {}

		explicit WithEdges(EdgesType edges) noexcept : edges_(std::move(edges)) {}

		[[nodiscard]] EdgesType &edges() noexcept { return this->edges_; }

		[[nodiscard]] EdgesType const &edges() const noexcept { return this->edges_; }

		[[nodiscard]] ChildrenType &edges(size_t pos) noexcept {
			assert(pos < depth);
			if constexpr (depth > 1)
				return this->edges_[pos];
			else
				return this->edges_;
		}

		[[nodiscard]] ChildrenType const &edges(size_t pos) const noexcept {
			assert(pos < depth);
			if constexpr (depth > 1)
				return this->edges_[pos];
			else
				return this->edges_;
		}

		[[nodiscard]] std::pair<bool, typename ChildrenType::iterator> find(size_t pos, key_part_type key_part) noexcept {
			auto found = this->edges(pos).find(key_part);
			return {found != this->edges(pos).end(), found};
		}

		[[nodiscard]] std::pair<bool, typename ChildrenType::const_iterator> find(size_t pos, key_part_type key_part) const noexcept {
			auto found = this->edges(pos).find(key_part);
			return {found != this->edges(pos).end(), found};
		}

		[[nodiscard]] ChildType child(size_t pos, key_part_type key_part) const noexcept {
			if (auto [found, iter] = this->find(pos, key_part); found) {
				if constexpr ((depth == 1) and htt_t::is_bool_valued)
					return true;
				else
					return iter->second;
			} else {
				return ChildType{};// 0, 0.0, false
			}
		}

		[[nodiscard]] size_t minCardPos(std::vector<size_t> const &positions) const noexcept {
			assert(not positions.empty());
			auto min_pos = positions[0];
			auto min_card = std::numeric_limits<size_t>::max();
			for (const size_t pos : positions) {
				const size_t current_card = edges(pos).size();
				if (current_card < min_card) {
					min_card = current_card;
					min_pos = pos;
				}
			}
			return min_pos;
		}

		[[nodiscard]] size_t min_card_pos() const noexcept {
			if constexpr (depth == 1)
				return 0;
			else {
				pos_type min_pos = 0;
				auto min_card = std::numeric_limits<size_t>::max();
				for (size_t pos = 0; pos < depth; ++pos) {
					const size_t current_card = edges(pos).size();
					if (current_card < min_card) {
						min_card = current_card;
						min_pos = pos;
					}
				}
				return min_pos;
			}
		}


		template<size_t fixed_positions>
		[[nodiscard]] size_t min_fixed_keypart_i(RawSliceKey<fixed_positions, htt_t> const &raw_slicekey) const noexcept {
			static_assert(fixed_positions > 0);
			size_t min_i = 0;
			auto min_card = std::numeric_limits<size_t>::max();
			size_t i = 0;
			for (size_t pos = 0; pos < depth; ++pos) {
				if (pos == raw_slicekey[i].pos) {
					if (auto current_card = edges(pos).size(); current_card < min_card) {
						min_card = current_card;
						min_i = i;
					}
					i++;
				}
				if (i == fixed_positions)
					break;
			}
			return min_i;
		}

		[[nodiscard]] std::vector<size_t> getCards(std::vector<pos_type> const &positions) const noexcept {
			std::vector<size_t> cards(positions.size());
			for (size_t i = 0; i < positions.size(); ++i) {
				auto pos = positions[i];
				assert(pos < depth);
				cards[i] = edges(pos).size();
			}
			return cards;
		}

		[[nodiscard]] size_t min_card_pos(RawKeyPositions<depth> const &positions_mask) const noexcept {
			size_t min_pos = 0;
			auto min_card = std::numeric_limits<size_t>::max();
			for (size_t pos = 0; pos < depth; ++pos) {
				if (positions_mask[pos]) {
					const size_t current_card = edges(pos).size();
					if (current_card < min_card) {
						min_card = current_card;
						min_pos = pos;
					}
				}
			}
			return min_pos;
		}
	};

}// namespace dice::hypertrie::internal::raw


#endif//HYPERTRIE_WITHEDGES_HPP
