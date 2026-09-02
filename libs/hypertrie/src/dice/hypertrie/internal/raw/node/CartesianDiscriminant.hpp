#ifndef HYPERTRIE_CARTESIANDISCRIMINANT_HPP
#define HYPERTRIE_CARTESIANDISCRIMINANT_HPP

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <numeric>

namespace dice::hypertrie::internal::raw {
	namespace cartesian_discriminant_detail {
		template<size_t required_bits>
		struct SelectInteger {
			static_assert(required_bits > 64,
						  "Could not select fitting integer for given required_bits, reason: does not fit into largest available integer");
		};

		template<size_t required_bits> requires (required_bits <= 8)
		struct SelectInteger<required_bits> {
			using type = uint8_t;
		};

		template<size_t required_bits> requires (required_bits > 8 && required_bits <= 16)
		struct SelectInteger<required_bits> {
			using type = uint16_t;
		};

		template<size_t required_bits> requires (required_bits > 16 && required_bits <= 32)
		struct SelectInteger<required_bits> {
			using type = uint32_t;
		};

		template<size_t required_bits> requires (required_bits > 32 && required_bits <= 64)
		struct SelectInteger<required_bits> {
			using type = uint64_t;
		};

		constexpr size_t pow(size_t base, size_t power) noexcept {
			if (power == 0) {
				return 1;
			}

			return base * pow(base, power - 1);
		}
	}  //namespace cartesian_discriminant_detail

	template<size_t depth>
	struct CartesianDiscriminant {
		static constexpr size_t required_discriminant_bits_per_operand = std::bit_width(depth);
		static constexpr size_t required_discriminant_bits = depth * required_discriminant_bits_per_operand;
		using repr_type = typename cartesian_discriminant_detail::SelectInteger<required_discriminant_bits>::type;
		static constexpr size_t n_distinct_discriminants = cartesian_discriminant_detail::pow(2, required_discriminant_bits);

	private:
		repr_type repr_ = 0;

		static constexpr repr_type shift_into_position(repr_type const value, size_t ix) {
			return value << ((depth - ix - 1) * required_discriminant_bits_per_operand);
		}

	public:
		constexpr CartesianDiscriminant() noexcept = default;
		constexpr CartesianDiscriminant(repr_type const repr) noexcept : repr_{repr} {
			assert(valid());
		}

		constexpr CartesianDiscriminant(std::array<repr_type, depth> const &children_depths) {
			assert((std::accumulate(children_depths.begin(), children_depths.end(), 0)) == depth);

			repr_ = 0;
			for (auto const child_d : children_depths) {
				assert(child_d < depth);
				repr_ <<= required_discriminant_bits_per_operand;
				repr_ |= child_d;
			}
		}

		[[nodiscard]] static constexpr CartesianDiscriminant for_general_cartesian() noexcept {
			std::array<repr_type, depth> children_depths;
			children_depths.fill(1);
			return CartesianDiscriminant{children_depths};
		}

		[[nodiscard]] constexpr repr_type &repr() noexcept {
			return repr_;
		}

		[[nodiscard]] constexpr repr_type repr() const noexcept {
			return repr_;
		}

		[[nodiscard]] constexpr size_t n_encoded_operands() const noexcept {
			return depth - (std::countr_zero(repr_) / required_discriminant_bits_per_operand);
		}

		[[nodiscard]] constexpr bool is_fully_depth1() const noexcept {
			if (n_encoded_operands() < depth) {
				return false;
			}

			for (size_t ix = 0; ix < depth; ++ix) {
				if (get(ix) != 1) {
					return false;
				}
			}

			return true;
		}

		constexpr repr_type operator[](size_t const ix) const noexcept {
			return get(ix);
		}

		constexpr repr_type get(size_t const ix) const noexcept {
			constexpr repr_type mask = (1 << required_discriminant_bits_per_operand) - 1;
			return (repr_ >> ((depth - ix - 1) * required_discriminant_bits_per_operand)) & mask;
		}

		constexpr void set(size_t const ix, repr_type const new_depth) noexcept {
			constexpr repr_type clear_mask = (1 << required_discriminant_bits_per_operand) - 1;
			repr_ &= ~shift_into_position(clear_mask, ix);
			repr_ |= shift_into_position(new_depth, ix);
		}

		[[nodiscard]] constexpr bool valid() const noexcept {
			size_t depth_acc = 0;
			for (size_t ix = 0; ix < depth; ++ix) {
				auto const op_depth = get(ix);
				if (op_depth >= depth) {
					return false;
				}

				depth_acc += op_depth;
			}

			return depth_acc == depth;
		}

		[[nodiscard]] constexpr std::pair<size_t, size_t> slice_index(size_t pos) const noexcept {
			assert(pos < depth);

			for (size_t ix = 0; ix < depth; ++ix) {
				auto const op_depth = get(ix);
				if (op_depth > pos) {
					return std::make_pair(ix, pos);
				}

				pos -= op_depth;
			}

			HYPERTRIE_UNREACHABLE;
		}

		constexpr CartesianDiscriminant<depth - 1> drop(size_t const drop_pos) const noexcept {
			assert(drop_pos < depth);

			CartesianDiscriminant<depth - 1> sub_discr; {
				for (size_t ix = 0; ix < drop_pos; ++ix) {
					sub_discr.set(ix, get(ix));
				}

				for (size_t ix = drop_pos + 1; ix < depth; ++ix) {
					sub_discr.set(ix - 1, get(ix));
				}
			}

			return sub_discr;
		}

		constexpr bool operator==(CartesianDiscriminant const &other) const noexcept = default;
	};

}  //namespace dice::hypertrie::internal::raw

namespace dice::hash {
	template<typename Policy, size_t depth>
	struct dice_hash_overload<Policy, ::dice::hypertrie::internal::raw::CartesianDiscriminant<depth>> {
		static size_t dice_hash(::dice::hypertrie::internal::raw::CartesianDiscriminant<depth> const discr) noexcept {
			return ::dice::hash::dice_hash_templates<Policy>::dice_hash(discr.repr());
		}
	};
} // namespace dice::hash

#endif//HYPERTRIE_CARTESIANDISCRIMINANT_HPP
