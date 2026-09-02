#ifndef HYPERTRIE_CARTESIANNODE_HPP
#define HYPERTRIE_CARTESIANNODE_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/internal/raw/node/NodePtr.hpp"
#include "dice/hypertrie/internal/raw/node/ReferenceCounted.hpp"
#include "dice/hypertrie/internal/raw/node/Hashed.hpp"
#include "dice/hypertrie/internal/raw/node/CartesianDiscriminant.hpp"
#include "dice/hypertrie/internal/raw/node/Sized.hpp"
#include "dice/hypertrie/internal/raw/RawDiagonalPositions.hpp"
#include "dice/template-library/switch_cases.hpp"

#include <numeric>
#include <cmath>
#include <utility>
#include <type_traits>

namespace dice::hypertrie::internal::raw {
	namespace cartesian_node_detail {
		template<typename Src, typename Dst>
		struct CopyConst {
			using type = Dst;
		};

		template<typename Src, typename Dst>
		struct CopyConst<Src const, Dst> {
			using type = Dst const;
		};
	} // namespace cartesian_node_detail

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct CartesianNode : Hashed<depth, htt_t>, ReferenceCounted, Sized {
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		using discriminant_type = CartesianDiscriminant<depth>;
		using operand_type = RawNodePtr<htt_t, allocator_type>;
		using operands_type = std::array<operand_type, depth>;

	private:
		template<size_t depth2, HypertrieTrait htt2_t, ByteAllocator allocator_type2>
		friend struct CartesianNode;

		discriminant_type discriminant_;
		operands_type operands_;

		template<typename Self, typename V, size_t ...ixs>
		static constexpr void for_each_operand_impl(Self &&self, V &&visitor, std::index_sequence<ixs...>) noexcept {
			auto const visit_single = [&]<size_t ix>() noexcept {
				dice::template_library::switch_cases<0, depth>(
						self.discriminant_[ix],
						[&](auto operand_depth) noexcept {
							using NodePtr_t = NodePtr<operand_depth, htt_t, allocator_type>;
							using NodePtr_ref_t = typename cartesian_node_detail::CopyConst<std::remove_reference_t<Self>, NodePtr_t>::type &;

							visitor.template operator()<ix>(static_cast<NodePtr_ref_t>(self.operands_[ix]));
						});
			};

			(visit_single.template operator()<ixs>(), ...);
		}

	public:
		constexpr CartesianNode() noexcept = default;

		constexpr CartesianNode(RawIdentifier<depth, htt_t> const &id, size_t ref_count) noexcept : Hashed<depth, htt_t>{id.hash()},
																									ReferenceCounted{ref_count},
																									Sized{id.size()} {
		}

		[[nodiscard]] discriminant_type &discriminant() noexcept {
			return discriminant_;
		}

		[[nodiscard]] discriminant_type discriminant() const noexcept {
			return discriminant_;
		}

		[[nodiscard]] constexpr operand_type &operand(size_t const ix) noexcept {
			return operands_[ix];
		}

		[[nodiscard]] constexpr operand_type operand(size_t const ix) const noexcept {
			return operands_[ix];
		}

		[[nodiscard]] constexpr size_t n_operands() const noexcept {
			return discriminant_.n_encoded_operands();
		}

		[[nodiscard]] constexpr operands_type &operands() noexcept {
			return operands_;
		}

		[[nodiscard]] constexpr operands_type const &operands() const noexcept {
			return operands_;
		}

		[[nodiscard]] RawIdentifier<depth, htt_t> identifier() const noexcept {
			return RawIdentifier<depth, htt_t>{this->hash(), this->size(), IdentifierTag::XN};
		}

		template<typename V>
		constexpr void for_each_operand(V &&visitor) const noexcept {
			return for_each_operand_impl(*this, std::forward<V>(visitor), std::make_index_sequence<depth>{});
		}

		template<typename V>
		constexpr void for_each_operand(V &&visitor) noexcept {
			return for_each_operand_impl(*this, std::forward<V>(visitor), std::make_index_sequence<depth>{});
		}

		[[nodiscard]] constexpr CartesianNode<depth - 1, htt_t, allocator_type> drop_operand(size_t const drop_ix,
																							 std::optional<size_t> const removed_operand_size_hint = std::nullopt) const noexcept {
			static_assert(depth != 0, "A subkey of a key of length 0 is not possible.");
			assert(n_operands() >= 2);

			CartesianNode<depth - 1, htt_t, allocator_type> sub_cartesian; {
				sub_cartesian.discriminant_ = discriminant_.drop(drop_ix);

				for (size_t ix = 0; ix < drop_ix; ++ix) {
					sub_cartesian.operands_[ix] = operands_[ix];
				}
				for (size_t ix = drop_ix + 1; ix < depth; ++ix) {
					sub_cartesian.operands_[ix - 1] = operands_[ix];
				}

				if (removed_operand_size_hint.has_value()) {
					sub_cartesian.size() = this->size() / *removed_operand_size_hint;
				}
			}

			return sub_cartesian;
		}

		template<size_t new_operand_depth>
		constexpr CartesianNode<depth - 1, htt_t, allocator_type> replace_operand(size_t const pos,
																				  NodePtr<new_operand_depth, htt_t, allocator_type> new_operand,
																				  std::optional<std::pair<size_t, size_t>> const old_new_operand_size_hints = std::nullopt) const noexcept {
			assert(n_operands() < depth);
			assert(pos < n_operands());
			assert(discriminant_[pos] > 1);
			assert(new_operand_depth == discriminant_[pos] - 1);

			CartesianNode<depth - 1, htt_t, allocator_type> sub_cartesian{}; {
				for (size_t ix = 0; ix < pos; ++ix) {
					sub_cartesian.discriminant_.set(ix, discriminant_[ix]);
					sub_cartesian.operands_[ix] = operands_[ix];
				}

				sub_cartesian.discriminant_.set(pos, new_operand_depth);
				sub_cartesian.operands_[pos] = new_operand;

				for (size_t ix = pos + 1; ix < n_operands(); ++ix) {
					sub_cartesian.discriminant_.set(ix, discriminant_[ix]);
					sub_cartesian.operands_[ix] = operands_[ix];
				}

				if (old_new_operand_size_hints.has_value()) {
					sub_cartesian.size() = this->size() / old_new_operand_size_hints->first * old_new_operand_size_hints->second;
				}
			}

			return sub_cartesian;
		}

		template<size_t new_operand_depth>
		constexpr CartesianNode<depth - 1, htt_t, allocator_type> replace_operand_flatten(size_t const pos,
																						  CartesianNode<new_operand_depth, htt_t, allocator_type> const &other,
																						  std::optional<std::pair<size_t, size_t>> const old_new_operand_size_hints = std::nullopt) const noexcept {
			assert(this->n_operands() < depth);
			assert(pos < this->n_operands());

			CartesianNode<depth - 1, htt_t, allocator_type> sub_cartesian{}; {
				for (size_t ix = 0; ix < pos; ++ix) {
					sub_cartesian.discriminant_.set(ix, discriminant_[ix]);
					sub_cartesian.operands_[ix] = operands_[ix];
				}

				for (size_t ix = 0; ix < other.n_operands(); ++ix) {
					sub_cartesian.discriminant_.set(pos + ix, other.discriminant_[ix]);
					sub_cartesian.operands_[pos + ix] = other.operands_[ix];
				}

				for (size_t ix = 0; ix < depth - pos - other.n_operands() - 1; ++ix) {
					sub_cartesian.discriminant_.set(pos + other.n_operands() + ix, discriminant_[pos + 1 + ix]);
					sub_cartesian.operands_[pos + other.n_operands() + ix] = operands_[pos + 1 + ix];
				}

				if (old_new_operand_size_hints.has_value()) {
					sub_cartesian.size() = this->size() / old_new_operand_size_hints->first * old_new_operand_size_hints->second;
				}
			}

			return sub_cartesian;
		}

		[[nodiscard]] constexpr size_t count_high_order_operands() const noexcept {
			size_t count = 0;
			for (size_t ix = 0; ix < n_operands(); ++ix) {
				if (!operand(ix).is_sen()) {
					count += 1;
				}
			}

			return count;
		}

		[[nodiscard]] constexpr bool is_xfix_cartesian() const noexcept {
			return count_high_order_operands() == 1;
		}

		[[nodiscard]] constexpr bool is_general_cartesian() const noexcept {
			return discriminant_.is_fully_depth1() && count_high_order_operands() > 1;
		}

		[[nodiscard]] constexpr std::optional<size_t> get_xfix_high_order_operand_index() const noexcept {
			for (size_t ix = 0; ix < n_operands(); ++ix) {
				if (!operand(ix).is_sen()) {
					return ix;
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] constexpr bool is_fully_sen() const noexcept {
			return !get_xfix_high_order_operand_index().has_value();
		}

		[[nodiscard]] std::vector<size_t> get_cards() const noexcept {
			std::vector<size_t> tmp;
			tmp.resize(depth);

			for_each_operand([&, write_ix = size_t{0}]<size_t, size_t operand_depth>(NodePtr<operand_depth, htt_t, allocator_type> const operand) mutable noexcept {
				if constexpr (operand_depth > 0) {
					if constexpr (operand_depth == 1) {
						if (operand.is_sen()) {
							tmp[write_ix] = 1;
						} else {
							tmp[write_ix] = operand.template specific_ptr<FullNode>()->size();
						}
					} else {
						auto const fn_ptr = operand.template specific_ptr<FullNode>();
						for (size_t pos = 0; pos < operand_depth; ++pos) {
							tmp[write_ix + pos] = fn_ptr->edges(pos).size();
						}
					}

					write_ix += operand_depth;
				}
			});

			return tmp;
		}

		[[nodiscard]] std::vector<size_t> get_cards(std::vector<pos_type> const &positions) const noexcept {
			auto const cards = get_cards();
			std::vector<size_t> ret;
			ret.resize(positions.size());

			for (size_t ix = 0; ix < positions.size(); ++ix) {
				assert(positions[ix] < depth);
				ret[ix] = cards[positions[ix]];
			}

			return ret;
		}

		[[nodiscard]] std::pair<size_t, size_t> min_card_pos() const noexcept {
			auto const cards = get_cards();
			auto it = std::min_element(cards.begin(), cards.end());
			assert(it != cards.end());
			return std::make_pair(it - cards.begin(), *it);
		}

		[[nodiscard]] std::pair<size_t, size_t> min_card_pos(RawKeyPositions<depth> const positions) const noexcept {
			auto const cards = get_cards();

			size_t min_card_pos = 0;
			size_t min_card = static_cast<size_t>(-1);

			for (size_t pos = 0; pos < depth; ++pos) {
				if (positions[pos]) {
					if (cards[pos] < min_card) {
						min_card_pos = pos;
						min_card = cards[pos];
					}
				}
			}

			return std::make_pair(min_card_pos, min_card);
		}
	};

}// namespace dice::hypertrie::internal::raw


#endif//HYPERTRIE_CARTESIANNODE_HPP
