#ifndef HYPERTRIE_CARDINALITYESTIMATION_HPP
#define HYPERTRIE_CARDINALITYESTIMATION_HPP

#include "dice/einsum/Commons.hpp"
#include "dice/einsum/Subscript.hpp"
#include "dice/einsum/internal/Context.hpp"

#include <dice/hypertrie/Hypertrie.hpp>

#include <robin_hood.h>

#include <cmath>

namespace dice::einsum::internal {


	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct CardinalityEstimation {

		/**
		 *
		 * @param operands
		 * @param label_candidates
		 * @param sc
		 * @return
		 */
		static Label getMinCardLabel(std::vector<::dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
									 std::shared_ptr<Subscript> const &sc,
									 [[maybe_unused]] std::shared_ptr<Context> const &context) {
			::robin_hood::unordered_set<Label> const &operandsLabelSet = sc->getOperandsLabelSet();
			::robin_hood::unordered_set<Label> const &lonely_non_result_labels = sc->getLonelyNonResultLabelSet();
			if (operandsLabelSet.size() == 1) {
				return *operandsLabelSet.begin();
			}
			Label min_label = *operandsLabelSet.begin();
			double min_cardinality = std::numeric_limits<double>::infinity();
			for (Label const label : operandsLabelSet) {
				if (lonely_non_result_labels.count(label)) {
					continue;
				}
				double const label_cardinality = calcCard(operands, label, sc);
				if (label_cardinality < min_cardinality) {
					min_cardinality = label_cardinality;
					min_label = label;
				}
			}
			return min_label;
		}


		static double estimate(
				std::vector<::dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				std::shared_ptr<Subscript> const &sc,
				[[maybe_unused]] std::shared_ptr<Context> const &context) {
			::robin_hood::unordered_set<Label> const &operandsLabelSet = sc->getOperandsLabelSet();
			::robin_hood::unordered_set<Label> const &lonely_non_result_labels = sc->getLonelyNonResultLabelSet();
			std::vector<double> operand_sizes(operands.size());
			for (size_t i = 0; i < operands.size(); ++i) {
				operand_sizes[i] = operands[i].size();
			}
			std::vector<double> label_factors;
			for (Label const label : operandsLabelSet) {
				if (not lonely_non_result_labels.count(label)) {
					label_factors.push_back(calcCard(operands, label, sc));
				}
			}
			double card = 1;
			bool order = operand_sizes.size() < label_factors.size();
			auto &shorter_vec = order ? operand_sizes : label_factors;
			auto &longer_vec = order ? label_factors : operand_sizes;
			size_t i = 0;
			for (; i < shorter_vec.size(); ++i) {
				card *= shorter_vec[i] * longer_vec[i];
			}
			for (; i < longer_vec.size(); ++i) {
				card *= longer_vec[i];
			}
			return card;
		}

	protected:
		/**
		 * Calculates the cardinality of an Label in an Step.
		 * @tparam T type of the values hold by processed Tensors (Tensor).
		 * @param operands Operands for this Step.
		 * @param step current step
		 * @param label the label
		 * @return label's cardinality in current step.
		 */
		static double calcCard(std::vector<::dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands, Label const label,
							   std::shared_ptr<Subscript> const &sc) {
			// get operands that have the label
			std::vector<LabelPos> const &op_poss = sc->getPossOfOperandsWithLabel(label);
			std::vector<double> op_dim_cardinalities(op_poss.size(), 1.0);
			auto label_count = 0;
			auto min_dim_card = std::numeric_limits<size_t>::max();
			::robin_hood::unordered_set<size_t> sizes{};
			const LabelPossInOperands &label_poss_in_operands = sc->getLabelPossInOperands(label);
			// iterate the operands that hold the label
			for (size_t i = 0; i < op_poss.size(); ++i) {
				auto const &op_pos = op_poss[i];
				auto const &operand = operands[op_pos];
				auto const op_dim_cards = operand.get_cards(label_poss_in_operands[op_pos]);
				auto const min_op_dim_card = *std::min_element(op_dim_cards.cbegin(), op_dim_cards.cend());
				auto const max_op_dim_card = *std::max_element(op_dim_cards.cbegin(), op_dim_cards.cend());
				for (const auto &op_dim_card : op_dim_cards) {
					sizes.insert(op_dim_card);
				}
				label_count += op_dim_cards.size();
				// update minimal dimension cardinality
				if (min_op_dim_card < min_dim_card) {
					min_dim_card = min_op_dim_card;
				}
				op_dim_cardinalities[i] = double(max_op_dim_card);
			}
			auto const min_dim_card_d = double(min_dim_card);
			double card = std::accumulate(op_dim_cardinalities.cbegin(), op_dim_cardinalities.cend(), double(1),
										  [&](double a, double b) { return a * min_dim_card_d / b; }) /
						  sizes.size();
			return card;
		}
	};
}// namespace dice::einsum::internal
#endif//HYPERTRIE_CARDINALITYESTIMATION_HPP
