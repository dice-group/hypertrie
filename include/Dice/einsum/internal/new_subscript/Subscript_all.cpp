#include "Dice/einsum/internal/new_subscript/Subscript_all.hpp"

namespace einsum::internal::new_subscript {

	std::shared_ptr<SubscriptJoin> SubscriptJoin::make_basic_graph_pattern(std::initializer_list<OperandLabels> operands_labels) {
		auto tmp = std::make_shared<SubscriptJoin>();
		for (const OperandLabels &operand_labels : operands_labels)
			tmp->append(std::make_shared<SubscriptOperand>(operand_labels));
		return tmp;
	}

	SubscriptJoin *SubscriptJoin::append(std::initializer_list<Label> operand_labels) {
		join_operands.push_back(SubscriptOperand::make(operand_labels));
		return this;
	}
	SubscriptJoin *SubscriptJoin::append(OperandLabels operand_labels) {
		join_operands.push_back(SubscriptOperand::make(operand_labels));
		return this;
	}

	std::shared_ptr<SubscriptJoin> SubscriptJoin::make_triple_pattern(std::initializer_list<Label> operand_labels) {
		return SubscriptJoin::make()->append(operand_labels)->shared_from_this();
	}
	std::shared_ptr<SubscriptJoin> SubscriptJoin::make_triple_pattern(OperandLabels operand_labels) {
		return SubscriptJoin::make()->append(operand_labels)->shared_from_this();
	}

}// namespace einsum::internal::new_subscript