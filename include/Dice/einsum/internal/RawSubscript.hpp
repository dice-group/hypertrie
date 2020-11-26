#ifndef HYPERTRIE_RAWSUBSCRIPT_HPP
#define HYPERTRIE_RAWSUBSCRIPT_HPP

#include <vector>
#include <tuple>
#include "Dice/hash/DiceHash.hpp"
#include <tsl/hopscotch_set.h>
#include <tsl/hopscotch_map.h>
#include <itertools.hpp>

namespace einsum::internal {
	/**
	 * A Einstein summation subscript label.
	 */
	using Label = char;
	/**
	 * The subscript of an operand in Einstein summation. This consists of a sequence of labels.
	 */
	using OperandSc = std::vector<Label>;
	/**
	 * The subscript of the result in Einstein summation. This consists of a sequence of labels.
	 */
	using ResultSc = OperandSc;
	/**
	 * The subscripts of all operands in Einstein summation.
	 */
	using OperandsSc = std::vector<OperandSc>;
	/**
	 * The position of an operand in Einstein summation. Starting at 0.
	 */
	using OperandPos = uint8_t;
	/**
	 * The position of an Label in the subscript of an operand or of the result Einstein summation. Starting at 0.
	 */
	using LabelPos = uint8_t;
	/**
	 * The positions of a single label in the subscript of a single operand.
	 */
	using LabelPossInOperand = std::vector<LabelPos>;
	/**
	 * For a single label, this stores for each operand the positions of the label in the Einstein summation subscript
	 * of that operand.
	 */
	using LabelPossInOperands = std::vector<LabelPossInOperand>;

	/**
	 * The RawSubscript represents the subscripts to the operands and result in Einstein summation.
	 * This class handles the actual storage, while Subscript provides a higher-level interface to the Subscript.
	 */
	class RawSubscript {
	public:
		/**
		 * The labels for each operand.
		 */
		mutable OperandsSc operands{};
		/**
		 * The labels for the result.
		 */
		mutable ResultSc result{};
		/**
		 * A hash of the Subscript.
		 */
		mutable std::size_t hash{};

	public:
		/**
		 * Generates a empty subscript. This subscript '<>-><>' has no operands and evaluates to the scalar 0.
		 */
		RawSubscript() = default;

		RawSubscript(RawSubscript &) = default;

		RawSubscript(const RawSubscript &) = default;

		RawSubscript(RawSubscript &&) = default;

		RawSubscript &operator=(const RawSubscript &) = default;

		RawSubscript &operator=(RawSubscript &&) = default;

		/**
		 * Constructs a RawSubscript.
		 * @param operands the labels of the operands
		 * @param result the labels of the result
		 */
		RawSubscript(const OperandsSc &operands, const ResultSc &result) :
				operands(operands), result(result),
				//hash(hypertrie::internal::robin_hood::rh_combine(operands, result)) {}
                hash(dice::hash::dice_hash(std::make_tuple(operands, result))) {}

		/**
		 * Constructs a RawSubscript.
		 * @param operands the labels of the operands
		 * @param result the labels of the result
		 */

		/**
		 * Constructs a RawSubscript.
		 * @tparam PAIR_t works at least with std::pair and std::tuple
		 * @param raw_subscript the labels of the operands and the labels of the result in a container that supports std::get
		 */
		template<template<typename, typename> typename PAIR_t>
		RawSubscript(const PAIR_t<OperandsSc, ResultSc> &raw_subscript) :
				RawSubscript(std::get<0>(raw_subscript), std::get<1>(raw_subscript)) {}


		/**
		 * @return number of operands
		 */
		[[nodiscard]] auto operandsCount() const noexcept {
			return operands.size();
		}

		/**
		 * @param operand_pos the position of a specific operand
		 * @return number of labels at the specific operand
		 */
		[[nodiscard]] auto labelCount(OperandPos operand_pos) const {
			return operands[operand_pos].size();
		}

		/**
		 * @return number of labels used in the result
		 */
		[[nodiscard]] auto resultLabelCount() const noexcept {
			return result.size();
		}

		/**
		 * @return set of labels used in the operands
		 */
		[[nodiscard]] auto getOperandsLabelSet() const noexcept {
			tsl::hopscotch_set<Label> operand_labels{};
			for (const auto &operand : operands)
				for (auto label : operand)
					operand_labels.insert(label);
			return operand_labels;
		}

		/**
		 * @return set of labels used in the result
		 */
		[[nodiscard]] auto getResultLabelSet() const noexcept {
			tsl::hopscotch_set<Label> result_labels{};
			for (auto label : result)
				result_labels.insert(label);
			return result_labels;
		}

		/**
		 * @param label the label to look for
		 * @return the positions of the given label in the operands' subscripts
		 */
		[[nodiscard]] auto getLabelPossInOperands(Label label) const noexcept {
			LabelPossInOperands label_poss_in_operands{};
			label_poss_in_operands.resize(operands.size());
			for (auto i : iter::range(operands.size())) {

				auto &label_poss_in_operand = label_poss_in_operands[i];
				const auto &operand = operands[i];

				for (const auto &[label_pos, current_label] : iter::enumerate(operand))
					if (current_label == label)
						label_poss_in_operand.push_back(label_pos);
			}

			return label_poss_in_operands;
		}

		/**
		 * @return a map from label to its position in the result. Only labels, that are in the result, are mapped.
		 */
		[[nodiscard]] auto getLabelPossInResult() const noexcept {
			tsl::hopscotch_map<Label, LabelPos> label_poss_in_result{};
			for (auto[pos, label]: iter::enumerate(result))
				label_poss_in_result.insert({label, pos});
			return label_poss_in_result;
		}

		/**
		 * Create a new RawSubscript without the given label.
		 * @param label the label to remove
		 * @return a new RawSubscript equal to this but without the given label.
		 */
		[[nodiscard]] auto removeLabel(Label label) const noexcept {
			assert(getOperandsLabelSet().count(label));
			OperandsSc next_operands{};
			for (const auto &operand: operands) {
				OperandSc new_operand{};
				for (auto current_label: operand)
					if (current_label != label)
						new_operand.push_back(current_label);
				if (not new_operand.empty()) {
					next_operands.push_back(std::move(new_operand));
				}
			}
			return RawSubscript(next_operands, result);
		}

		/**
		 * Check if another Subscript is different. It is also different if the labels are ordered alike but other
		 * labels are used.
		 * @param other another RawSubscript
		 * @return true if they are different, else false.
		 */
		[[nodiscard]] bool operator!=(const RawSubscript &other) const noexcept {
			// check the number of operands
			if (this->operands.size() != other.operands.size())
				return true;
			// check the number of labels in the result
			if (this->result.size() != other.result.size())
				return true;
			// for each operand ...
			for (const auto &[op1, op2] : iter::zip(this->operands, other.operands)) {
				// check the number of labels
				if (std::size(op1) != std::size(op2))
					return true;
				// check each label
				for (const auto &[label1, label2] : iter::zip(op1, op2))
					if (label1 != label2)
						return true;
			}

			// check each label of the result
			for (const auto &[label1, label2] : iter::zip(this->result, other.result))
				if (label1 != label2)
					return true;

			return false;
		};
	};
}

template<>
struct std::hash<einsum::internal::RawSubscript> {
	size_t operator()(const einsum::internal::RawSubscript &s) const { return s.hash; }
};

#endif //HYPERTRIE_RAWSUBSCRIPT_HPP
