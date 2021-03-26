#ifndef HYPERTRIE_CARDINALITYESTIMATION_HPP
#define HYPERTRIE_CARDINALITYESTIMATION_HPP

#include <cmath>
#include "Dice/einsum/internal/Subscript.hpp"
#include "Dice/einsum/internal/Entry.hpp"

namespace einsum::internal {


	template<HypertrieTrait tr>
	struct CardinalityEstimation {

		/**
		 *
		 * @param operands
		 * @param label_candidates
		 * @param sc
		 * @return
		 */
		static Label getMinCardLabel(const std::vector<const_Hypertrie<tr>> &operands,
		                             const std::shared_ptr<Subscript> &sc,
		                             [[maybe_unused]] std::shared_ptr<Context> context) {
			const tsl::hopscotch_set <Label> &operandsLabelSet = sc->getOperandsLabelSet();
			const tsl::hopscotch_set <Label> &lonely_non_result_labels = sc->getLonelyNonResultLabelSet();
			if (operandsLabelSet.size() == 1) {
				return *operandsLabelSet.begin();
			} else {

				Label min_label = *operandsLabelSet.begin();
				double min_cardinality = std::numeric_limits<double>::infinity();
				for (const Label label : operandsLabelSet) {
					if (lonely_non_result_labels.count(label))
						continue;
					const double label_cardinality = calcCard(operands, label, sc);
					if (label_cardinality < min_cardinality) {
						min_cardinality = label_cardinality;
						min_label = label;
					}
				}
				return min_label;
			}
		}


		static double estimate(
				const std::vector<const_Hypertrie<tr>> &operands,
				const std::shared_ptr<Subscript> &sc,
				[[maybe_unused]] std::shared_ptr<Context> context) {
			const tsl::hopscotch_set<Label> &operandsLabelSet = sc->getOperandsLabelSet();
			const tsl::hopscotch_set<Label> &lonely_non_result_labels = sc->getLonelyNonResultLabelSet();
			std::vector<double> operand_sizes(operands.size());
			for (auto [size, operand] : iter::zip(operand_sizes, operands))
				size = operand.size();

			std::vector<double> label_factors;
			for (const Label label :
				 iter::filterfalse([&](auto label) { return lonely_non_result_labels.count(label); },
							 operandsLabelSet))
				label_factors.push_back(calcCard(operands, label, sc));

			double card = 1;
			bool order = operand_sizes.size() < label_factors.size();
			auto &shorter_vec = order ? operand_sizes : label_factors;
			auto &longer_vec = order ? label_factors : operand_sizes;

			for (const auto &[shorter_val, longer_val] : iter::zip(shorter_vec, longer_vec))
				card *= (shorter_val * longer_val);

			for (const auto &longer_val : iter::slice(longer_vec, shorter_vec.size(), longer_vec.size()))
				card *= longer_val;

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
		static double calcCard(const std::vector<const_Hypertrie<tr>> &operands, const Label label,
		                       const std::shared_ptr<Subscript> &sc) {
			// get operands that have the label
			const std::vector<LabelPos> &op_poss = sc->getPossOfOperandsWithLabel(label);
			std::vector<double> op_dim_cardinalities(op_poss.size(), 1.0);
			auto label_count = 0;
			auto min_dim_card = std::numeric_limits<size_t>::max();
			tsl::hopscotch_set <size_t> sizes{};

			const LabelPossInOperands &label_poss_in_operands = sc->getLabelPossInOperands(label);
			// iterate the operands that hold the label
			for (auto[i, op_pos] : iter::enumerate(op_poss)) {
				const auto &operand = operands[op_pos];
				const auto op_dim_cards = operand.getCards(label_poss_in_operands[op_pos]);
				const auto min_op_dim_card = *std::min_element(op_dim_cards.cbegin(), op_dim_cards.cend());
				const auto max_op_dim_card = *std::max_element(op_dim_cards.cbegin(), op_dim_cards.cend());

				for (const auto &op_dim_card : op_dim_cards)
					sizes.insert(op_dim_card);

				label_count += op_dim_cards.size();
				// update minimal dimension cardinality
				if (min_op_dim_card < min_dim_card)
					min_dim_card = min_op_dim_card;

				op_dim_cardinalities[i] = double(max_op_dim_card); //
			}

			auto const min_dim_card_d = double(min_dim_card);

			double card = std::accumulate(op_dim_cardinalities.cbegin(), op_dim_cardinalities.cend(), double(1),
			                              [&](double a, double b) {
				                              return a * min_dim_card_d / b;
			                              }) / sizes.size();
			return card;
		}
	};
}
#endif //HYPERTRIE_CARDINALITYESTIMATION_HPP
