#ifndef HYPERTRIE_VALIDATIONRAWHASHDIAGONAL_SLICE_HPP
#define HYPERTRIE_VALIDATIONRAWHASHDIAGONAL_SLICE_HPP
#include "ValidationRawNodeContext.hpp"
#include "ValidationRawNodeContext_slice.hpp"
#include <dice/hypertrie/Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/RawKey.hpp>
#include <dice/hypertrie/internal/raw/iteration/RawHashDiagonal.hpp>
#include <dice/hypertrie/internal/raw/node/SingleEntry.hpp>

namespace dice::hypertrie::tests::core::node {
	using namespace fmt::literals;

	using namespace ::dice::hypertrie::internal::raw;
	using namespace ::dice::hypertrie::internal;

	template<size_t diag_depth, size_t depth, HypertrieTrait htt_t>
	class ValidationRawHashDiagonal {
	public:
		static constexpr const size_t result_depth = depth - diag_depth;
		using EntriesType = std::vector<SingleEntry<depth, htt_t>>;

		using ResultEntriesType = std::vector<SingleEntry<result_depth, htt_t>>;
		using DiagonalPositions = RawKeyPositions<depth>;
		using key_part_type = typename htt_t::key_part_type;

	protected:
		std::unordered_map<key_part_type, ResultEntriesType> non_zero_diagonals_;
		std::unordered_map<key_part_type, RawIdentifier<result_depth, htt_t>> non_zero_diagonal_ids_;
		std::unordered_set<key_part_type> key_parts_;

	public:
		ValidationRawHashDiagonal(EntriesType const &entries, DiagonalPositions diag_poss) noexcept {
			for (auto const &entry : entries) {
				key_part_type key_part = entry.key()[diag_poss.first_pos()];
				auto opt_slice = diag_poss.template slice<diag_depth>(entry.key(), key_part);

				if (opt_slice.has_value()) {
					non_zero_diagonals_[key_part].emplace_back(*opt_slice, entry.value());
				}
			}

			for (auto const &[key_part, diag_entries] : non_zero_diagonals_) {
				key_parts_.insert(key_part);

				if (diag_entries.size() == 1) {
					non_zero_diagonal_ids_[key_part] = RawIdentifier<result_depth, htt_t>{diag_entries, IdentifierTag::SEN};
					continue;
				}

				if constexpr (result_depth > 1) {
					auto const operands = node_context::common_detail::inverse_cartesian_product(diag_entries);

					if constexpr (HypertrieTrait_bool_valued<htt_t>) {
						if (node_context::common_detail::is_general_cartesian(operands, diag_entries.size())) {
							non_zero_diagonal_ids_[key_part] = RawIdentifier<result_depth, htt_t>{diag_entries, IdentifierTag::XN};
							continue;
						}
					}

					if (node_context::common_detail::try_get_xfix_cartesian_properties(operands).has_value()) {
						non_zero_diagonal_ids_[key_part] = RawIdentifier<result_depth, htt_t>{diag_entries, IdentifierTag::XN};
						continue;
					}
				}

				non_zero_diagonal_ids_[key_part] = RawIdentifier<result_depth, htt_t>{diag_entries, IdentifierTag::FN};
			}
		}

		bool has_diagonal(key_part_type key_part) const noexcept {
			return non_zero_diagonals_.contains(key_part);
		}

		ResultEntriesType const &entries(key_part_type key_part) const {
			return non_zero_diagonals_.at(key_part);
		}

		RawIdentifier<result_depth, htt_t> const &raw_identifier(key_part_type key_part) const {
			return non_zero_diagonal_ids_.at(key_part);
		}

		const std::unordered_set<key_part_type> &key_parts() const noexcept {
			return key_parts_;
		}

		[[nodiscard]] size_t size() const noexcept {
			return non_zero_diagonals_.size();
		}

		friend std::ostream &operator<<(std::ostream &os, ValidationRawHashDiagonal<diag_depth, depth, htt_t> const &diag) {
			for (auto const &[key_part, entries] : diag.non_zero_diagonals_) {
				os << fmt::format("{}:\n\t{}\n", key_part, fmt::join(entries, "\n\t"));
			}

			return os;
		}
	};

}// namespace dice::hypertrie::tests::core::node

#endif//HYPERTRIE_VALIDATIONRAWHASHDIAGONAL_SLICE_HPP
