#ifndef HYPERTRIE_VALIDATIONRAWHASHDIAGONAL_SLICE_HPP
#define HYPERTRIE_VALIDATIONRAWHASHDIAGONAL_SLICE_HPP
#include "ValidationRawNodeContext.hpp"
#include "ValidationRawNodeContext_slice.hpp"
#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/RawKey.hpp>
#include <Dice/hypertrie/internal/raw/iteration/RawHashDiagonal.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleEntry.hpp>

namespace Dice::hypertrie::tests::core::node {
	using namespace fmt::literals;

	using namespace ::Dice::hypertrie::internal::raw;
	using namespace ::Dice::hypertrie::internal;

	template<size_t diag_depth, size_t depth, HypertrieCoreTrait tri>
	class ValidationRawHashDiagonal {
	public:
		static constexpr const size_t result_depth = depth - diag_depth;
		using EntriesType = std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>>;

		using ResultEntriesType = std::vector<SingleEntry<result_depth, tri_with_stl_alloc<tri>>>;
		using DiagonalPositions = RawKeyPositions<depth>;
		using key_part_type = typename tri::key_part_type;

	protected:
		std::unordered_map<key_part_type, ResultEntriesType> non_zero_diagonals_;
		std::unordered_map<key_part_type, RawIdentifier<result_depth, tri_with_stl_alloc<tri>>> non_zero_diagonal_ids_;
		std::unordered_set<key_part_type> key_parts_;

	public:
		ValidationRawHashDiagonal(EntriesType const &entries, DiagonalPositions diag_poss) noexcept {
			for (const auto &entry : entries) {
				key_part_type key_part = entry.key()[diag_poss.first_pos()];
				auto opt_slice = diag_poss.template slice<diag_depth>(entry.key(), key_part);
				if (opt_slice.has_value())
					non_zero_diagonals_[key_part].emplace_back(*opt_slice, entry.value());
			}

			for (const auto &[key_part, diag_entries] : non_zero_diagonals_) {
				non_zero_diagonal_ids_[key_part] = RawIdentifier<result_depth, tri_with_stl_alloc<tri>>{diag_entries};
				key_parts_.insert(key_part);
			}
		}

		auto has_diagonal(key_part_type key_part) const noexcept {
			return non_zero_diagonals_.contains(key_part);
		}

		auto &entries(key_part_type key_part) const {
			return non_zero_diagonals_.at(key_part);
		}

		auto &raw_identifier(key_part_type key_part) const {
			return non_zero_diagonal_ids_.at(key_part);
		}

		const std::unordered_set<key_part_type> &key_parts() const noexcept {
			return key_parts_;
		}

		[[nodiscard]] size_t size() const noexcept {
			return non_zero_diagonals_.size();
		}
	};
}// namespace Dice::hypertrie::tests::core::node

#endif//HYPERTRIE_VALIDATIONRAWHASHDIAGONAL_SLICE_HPP
