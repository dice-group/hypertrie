#ifndef HYPERTRIE_CARDINALITYESTIMATION_HPP
#define HYPERTRIE_CARDINALITYESTIMATION_HPP

#include <cmath>
#include "Dice/einsum/internal/Subscript.hpp"
#include "Dice/einsum/internal/Entry.hpp"

namespace einsum::internal {
	struct FullLabelPos {
		OperandPos op_pos;
		LabelPos label_pos;
	};
	using FullLabelPoss = std::vector<FullLabelPos>;
	struct LabelCardInfo {
		Label label;
		LabelPossInOperands label_poss_in_operands;
		double card;
	};

	template<typename key_part_type, template<typename, typename> class map_type,
			template<typename> class set_type>
	struct CardinalityEstimation {
		using const_BoolHypertrie_t = const_BoolHypertrie<key_part_type, map_type, set_type>;

		/**
		 *
		 * @param operands
		 * @param label_candidates
		 * @param sc
		 * @return
		 */
		static LabelCardInfo getMinCardLabel(const std::vector<const_BoolHypertrie_t> &operands,
											 const std::shared_ptr<Subscript> &sc,
											 std::shared_ptr<Context<key_part_type>> context) {
			const tsl::hopscotch_set<Label> &operandsLabelSet = sc->getOperandsLabelSet();
			const tsl::hopscotch_set<Label> &lonely_non_result_labels = sc->getLonelyNonResultLabelSet();

			std::vector<LabelCardInfo> label_candidates{};

			for (const Label label : operandsLabelSet) {
				label_candidates.push_back(calcCard(operands, label, sc, bool(context->getFixedLabel(label))));
			}
			std::sort(label_candidates.begin(), label_candidates.end(),
					  [&](const auto &left, const auto &right) -> bool { return left.card < right.card; });
			assert(not label_candidates.empty());
			return label_candidates[0];
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
		static LabelCardInfo
		calcCard(const std::vector<const_BoolHypertrie_t> &operands, const Label label,
				 const std::shared_ptr<Subscript> &sc, bool label_already_used) {
			// TODO: remove the additional information as soon as it clear that it is not needed.
			struct Stats {
				size_t dim_card;
				size_t op_card;
				OperandPos op_pos;
				LabelPos label_pos;

				[[nodiscard]] FullLabelPos getFullLabelPos() const {
					return {op_pos, label_pos};
				}
			};
			// get operands that have the label
			const std::vector<OperandPos> &op_poss = sc->getPossOfOperandsWithLabel(label);
			std::set<OperandPos> op_pos_set(op_poss.begin(), op_poss.end());
			const LabelPossInOperands &label_poss_in_operands = sc->getLabelPossInOperands(label);
			std::vector<Stats> label_stats{};
			// iterate the operands that hold the label
			for (auto[i, op_pos] : iter::enumerate(op_pos_set)) {
				const auto &operand = operands[op_pos];
				for (const auto &label_pos : label_poss_in_operands[op_pos]) {
					size_t card = operand.getCards({label_pos})[0];
					size_t op_card = operand.size();
					label_stats.emplace_back(Stats{card, op_card, op_pos, label_pos});
				}
			}
			std::sort(label_stats.begin(), label_stats.end(),
					  [&](const auto &left, const auto &right) -> bool { return left.dim_card < right.dim_card; });
			assert(label_stats.size() > 0);
			const Stats &min_card = label_stats.front();
			if (label_stats.size() == 1 or label_already_used) {
				LabelPossInOperands result_label_poss_in_operands(sc->operandsCount());
				result_label_poss_in_operands[min_card.op_pos].push_back(min_card.label_pos);
				return LabelCardInfo{label, result_label_poss_in_operands, 1};
			} else {
				const Stats &max_card = label_stats.back();
				LabelPossInOperands result_label_poss_in_operands(sc->operandsCount());
				result_label_poss_in_operands[min_card.op_pos].push_back(min_card.label_pos);
				result_label_poss_in_operands[max_card.op_pos].push_back(max_card.label_pos);
				return LabelCardInfo{label,
									 result_label_poss_in_operands,
									 ((double) min_card.dim_card) / ((double) max_card.dim_card)};
			}
		}
	};
}
#endif //HYPERTRIE_CARDINALITYESTIMATION_HPP

