#ifndef HYPERTRIE_RAWSUBSCRIPT_HPP
#define HYPERTRIE_RAWSUBSCRIPT_HPP

#include <dice/hash/DiceHash.hpp>

#include <robin_hood.h>

#include <tuple>
#include <vector>
#include <cassert>

namespace dice::einsum::internal {
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

		RawSubscript(RawSubscript const &) = default;

		RawSubscript(RawSubscript &&) = default;

		RawSubscript &operator=(RawSubscript const &) = default;

		RawSubscript &operator=(RawSubscript &&) = default;

		/**
		 * Constructs a RawSubscript.
		 * @param operands the labels of the operands
		 * @param result the labels of the result
		 */
		RawSubscript(OperandsSc const &operands, ResultSc const &result) noexcept
			: operands(operands), result(result), hash(dice::hash::DiceHashwyhash<std::tuple<OperandsSc, ResultSc>>()(std::make_tuple(operands, result))) {}

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
		RawSubscript(PAIR_t<OperandsSc, ResultSc> const &raw_subscript) : RawSubscript(std::get<0>(raw_subscript), std::get<1>(raw_subscript)) {}


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
			::robin_hood::unordered_set<Label> operand_labels{};
			for (auto const &operand : operands) {
				for (auto label : operand) {
					operand_labels.insert(label);
				}
			}
			return operand_labels;
		}

		/**
		 * @return set of labels used in the result
		 */
		[[nodiscard]] auto getResultLabelSet() const noexcept {
			::robin_hood::unordered_set<Label> result_labels{};
			for (auto label : result) {
				result_labels.insert(label);
			}
			return result_labels;
		}

		/**
		 * @param label the label to look for
		 * @return the positions of the given label in the operands' subscripts
		 */
		[[nodiscard]] auto getLabelPossInOperands(Label label) const noexcept {
			LabelPossInOperands label_poss_in_operands{};
			label_poss_in_operands.resize(operands.size());
			for (size_t i = 0; i < operands.size(); ++i) {
				auto &label_poss_in_operand = label_poss_in_operands[i];
				auto const &operand = operands[i];
				for (size_t label_pos = 0; label_pos < operand.size(); ++label_pos) {
					if (operand[label_pos] == label) {
						label_poss_in_operand.push_back(label_pos);
					}
				}
			}
			return label_poss_in_operands;
		}

		/**
		 * @return a map from label to its position in the result. Only labels, that are in the result, are mapped.
		 */
		[[nodiscard]] auto getLabelPossInResult() const noexcept {
			::robin_hood::unordered_map<Label, LabelPos> label_poss_in_result{};
			for (size_t pos = 0; pos < result.size(); ++pos) {
				label_poss_in_result.insert({result[pos], pos});
			}
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
			for (auto const &operand : operands) {
				OperandSc new_operand{};
				for (auto current_label : operand) {
					if (current_label != label) {
						new_operand.push_back(current_label);
					}
				}
				if (not new_operand.empty()) {
					next_operands.push_back(std::move(new_operand));
				}
			}
			return RawSubscript(next_operands, result);
		}

		explicit operator std::string() const noexcept {
			std::string out;
			for (size_t op_pos = 0; op_pos < operands.size(); ++op_pos) {
				for (auto label : operands[op_pos]) {
					out.push_back(label);
				}
				if (op_pos != operands.size() - 1) {
					out.push_back(',');
				}
			}
			out += "->";
			for (auto label : result) {
				out.push_back(label);
			}
			return out;
		}

		/**
		 * Check if another Subscript is different. It is also different if the labels are ordered alike but other
		 * labels are used.
		 * @param other another RawSubscript
		 * @return true if they are different, else false.
		 */
		[[nodiscard]] bool operator!=(RawSubscript const &other) const noexcept {
			// check the number of operands
			if (this->operands.size() != other.operands.size()) {
				return true;
			}
			// check the number of labels in the result
			if (this->result.size() != other.result.size()) {
				return true;
			}
			// for each operand ...
			for (size_t op_pos = 0; op_pos < this->operands.size(); ++op_pos) {
				auto const &op1 = this->operands[op_pos];
				auto const &op2 = other.operands[op_pos];
				// check the number of labels
				if (std::size(op1) != std::size(op2)) {
					return true;
				}
				// check each label
				for (size_t label_pos = 0; label_pos < op1.size(); ++label_pos) {
					if (op1[label_pos] != op2[label_pos]) {
						return true;
					}
				}
			}
			// check each label of the result
			for (size_t label_pos = 0; label_pos < this->result.size(); ++label_pos) {
				if (this->result[label_pos] != other.result[label_pos]) {
					return true;
				}
			}
			return false;
		};
	};
}// namespace dice::einsum::internal

template<>
struct std::hash<::dice::einsum::internal::RawSubscript> {
	size_t operator()(::dice::einsum::internal::RawSubscript const &s) const noexcept { return s.hash; }
};

#endif//HYPERTRIE_RAWSUBSCRIPT_HPP
