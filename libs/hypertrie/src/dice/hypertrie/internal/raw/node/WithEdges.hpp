#ifndef HYPERTRIE_WITHEDGES_HPP
#define HYPERTRIE_WITHEDGES_HPP

#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/RawDiagonalPositions.hpp"
#include "dice/hypertrie/internal/raw/node/NodePtr.hpp"
#include "dice/hypertrie/internal/raw/node/RawIdentifier.hpp"

#include "dice/hypertrie/internal/commons/PosType.hpp"

#include <limits>

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct WithEdges {
		using value_type = typename htt_t::value_type;
		using key_part_type = typename htt_t::key_part_type;

		using child_type = std::conditional_t<(depth > 1),
											  NodePtr<depth - 1, htt_t, allocator_type>,
											  typename htt_t::value_type>;

		using edge_type = std::conditional_t<(depth == 1 && HypertrieTrait_bool_valued<htt_t>),
											 key_part_type,
											 std::pair<key_part_type, child_type>>;

		using single_dim_edges_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<edge_type>;

		using single_dim_edges_type = std::conditional_t<(depth == 1 && HypertrieTrait_bool_valued<htt_t>),
														 typename htt_t::template set_type<key_part_type, single_dim_edges_allocator>,
														 typename htt_t::template map_type<key_part_type, child_type, single_dim_edges_allocator>>;

		using edges_type = std::conditional_t<(depth > 1),
											  std::array<single_dim_edges_type, depth>,
											  single_dim_edges_type>;

	private:
		edges_type edges_;

		template<size_t>
		static single_dim_edges_type make_single_dim_edges(allocator_type const &alloc) noexcept {
			return single_dim_edges_type{alloc};
		}

		template<size_t ...Ixs>
		static edges_type make_default_init_edges_high_depth(allocator_type const &alloc, std::index_sequence<Ixs...>) noexcept {
			return edges_type{make_single_dim_edges<Ixs>(alloc)...};
		}

		static edges_type make_default_init_edges(allocator_type const &alloc) noexcept {
			if constexpr (depth == 1) {
				return edges_type{alloc};
			} else {
				return make_default_init_edges_high_depth(alloc, std::make_index_sequence<depth>{});
			}
		}

	public:
		WithEdges() = delete;
		explicit WithEdges(allocator_type const &alloc) : edges_{make_default_init_edges(alloc)} {}

		[[nodiscard]] edges_type &edges() noexcept { return this->edges_; }
		[[nodiscard]] edges_type const &edges() const noexcept { return this->edges_; }

		[[nodiscard]] single_dim_edges_type &edges(size_t pos) noexcept {
			assert(pos < depth);
			if constexpr (depth > 1) {
				return this->edges_[pos];
			} else {
				return this->edges_;
			}
		}

		[[nodiscard]] single_dim_edges_type const &edges(size_t pos) const noexcept {
			assert(pos < depth);
			if constexpr (depth > 1) {
				return this->edges_[pos];
			} else {
				return this->edges_;
			}
		}

		[[nodiscard]] std::pair<bool, typename single_dim_edges_type::iterator> find(size_t pos, key_part_type key_part) noexcept {
			auto it = this->edges(pos).find(key_part);
			return {it != this->edges(pos).end(), it};
		}

		[[nodiscard]] std::pair<bool, typename single_dim_edges_type::const_iterator> find(size_t pos, key_part_type key_part) const noexcept {
			auto it = this->edges(pos).find(key_part);
			return {it != this->edges(pos).end(), it};
		}

		[[nodiscard]] child_type child(size_t pos, key_part_type key_part) const noexcept {
			if (auto [found, iter] = this->find(pos, key_part); found) {
				if constexpr ((depth == 1) and htt_t::is_bool_valued) {
					return true;
				} else {
					return iter->second;
				}
			}

			return child_type{};
		}

		[[nodiscard]] std::vector<size_t> get_cards(std::vector<internal::pos_type> const &positions) const noexcept {
			assert(positions.size() <= depth);

			std::vector<size_t> cards;
			cards.resize(positions.size());

			for (size_t ix = 0; ix < positions.size(); ++ix) {
				assert(positions[ix] < depth);
				cards[ix] = edges(positions[ix]).size();
			}
			return cards;
		}

		[[nodiscard]] size_t min_card_pos() const noexcept {
			if constexpr (depth == 1) {
				return 0;
			} else {
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

		[[nodiscard]] size_t min_card_pos(std::vector<size_t> const &positions) const noexcept {
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
	};

}// namespace dice::hypertrie::internal::raw


#endif//HYPERTRIE_WITHEDGES_HPP
