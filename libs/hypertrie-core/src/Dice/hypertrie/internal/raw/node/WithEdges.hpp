#ifndef HYPERTRIE_WITHEDGES_HPP
#define HYPERTRIE_WITHEDGES_HPP

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/RawDiagonalPositions.hpp>
#include <Dice/hypertrie/internal/raw/node/Identifier.hpp>
#include <range.hpp>

#include <Dice/hypertrie/internal/commons/PosType.hpp>

namespace hypertrie::internal::raw {

	// TODO: review this class
	template<size_t depth, HypertrieCoreTrait tri>
	struct WithEdges {
		using allocator_type = typename tri::allocator_type;
		using value_type = typename tri::value_type;
		using key_part_type = typename tri::key_part_type;

		using ChildType = std::conditional_t<(depth > 1),
											 RawIdentifier<depth - 1, tri>,
											 value_type>;
		using collection_alloc = std::conditional_t<((depth == 1) and tri::is_bool_valued),
													typename std::allocator_traits<allocator_type>::template rebind_alloc<key_part_type>,
													typename std::allocator_traits<allocator_type>::template rebind_alloc<std::pair<typename tri::key_part_type, ChildType>>>;

		// allocator might need to be cast to specific type for set/map
		using ChildrenType = std::conditional_t<((depth == 1) and tri::is_bool_valued),
												typename tri::template set_type<key_part_type>,
												typename tri::template map_type<typename tri::key_part_type, ChildType>>;

		using EdgesType = std::conditional_t<(depth > 1),
											 std::array<ChildrenType, depth>,
											 ChildrenType>;

	protected:
		// TODO: check if ChildrenType is destructed correctly with custom allocator (instance)
		EdgesType edges_;

		// TODO: move to util
		template<size_t I>
		static inline auto f(const allocator_type &alloc) { return ChildrenType(alloc); }

		template<size_t N, size_t... Is>
		static inline auto fill(const allocator_type &alloc, std::index_sequence<Is...>) {
			return std::array<ChildrenType, N>{f<Is>(alloc)...};
		}

		template<size_t N>
		static inline std::array<ChildrenType, N> fill(const allocator_type &alloc) {
			return fill<N>(alloc, std::make_index_sequence<N>());
		}


		static inline EdgesType init([[maybe_unused]] const allocator_type &alloc) {
			if constexpr (depth == 1) {
				return EdgesType(alloc);
			} else {
				return fill<depth>(alloc);
			}
		}

	public:
		WithEdges() = default;

		explicit WithEdges(const allocator_type &alloc) : edges_(init(alloc)) {}

		explicit WithEdges(EdgesType edges) noexcept : edges_(std::move(edges)) {}

		EdgesType &edges() { return this->edges_; }

		const EdgesType &edges() const { return this->edges_; }

		ChildrenType &edges(size_t pos) {
			assert(pos < depth);
			if constexpr (depth > 1)
				return this->edges_[pos];
			else
				return this->edges_;
		}

		const ChildrenType &edges(size_t pos) const {
			assert(pos < depth);
			if constexpr (depth > 1)
				return this->edges_[pos];
			else
				return this->edges_;
		}

		std::pair<bool, typename ChildrenType::iterator> find(size_t pos, key_part_type key_part) {
			auto found = this->edges(pos).find(key_part);
			return {found != this->edges(pos).end(), found};
		}

		std::pair<bool, typename ChildrenType::const_iterator> find(size_t pos, key_part_type key_part) const {
			auto found = this->edges(pos).find(key_part);
			return {found != this->edges(pos).end(), found};
		}

		ChildType child(size_t pos, key_part_type key_part) const {
			if (auto [found, iter] = this->find(pos, key_part); found) {
				if constexpr ((depth == 1) and tri::is_bool_valued)
					return true;
				else
					return iter->second;
			} else {
				return ChildType{};// 0, 0.0, false
			}
		}

		[[nodiscard]] size_t minCardPos(const std::vector<size_t> &positions) const {
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
			pos_type min_pos = 0;
			auto min_card = std::numeric_limits<size_t>::max();
			for (const pos_type pos : iter::range(depth)) {
				const size_t current_card = edges(pos).size();
				if (current_card < min_card) {
					min_card = current_card;
					min_pos = pos;
				}
			}
			return min_pos;
		}


		template<size_t fixed_positions>
		[[nodiscard]] size_t min_fixed_keypart_i(const RawSliceKey<fixed_positions, tri> &raw_slicekey) const noexcept {
			static_assert(fixed_positions > 0);
			size_t min_i = 0;
			auto min_card = std::numeric_limits<size_t>::max();
			size_t i = 0;
			for (pos_type pos : iter::range(depth)) {
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

		[[nodiscard]] std::vector<size_t> getCards(const std::vector<pos_type> &positions) const {
			std::vector<size_t> cards(positions.size());
			for (auto i : iter::range(positions.size())) {
				auto pos = positions[i];
				assert(pos < depth);
				cards[i] = edges(pos).size();
			}
			return cards;
		}

		[[nodiscard]] size_t minCardPos(const RawKeyPositions<depth> &positions_mask) const {
			assert(positions_mask.any());
			auto min_pos = 0;
			auto min_card = std::numeric_limits<size_t>::max();
			for (const size_t pos : iter::range(depth)) {
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

}// namespace hypertrie::internal::raw


#endif//HYPERTRIE_WITHEDGES_HPP
