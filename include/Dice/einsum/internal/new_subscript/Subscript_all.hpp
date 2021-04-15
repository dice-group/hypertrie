#ifndef HYPERTRIE_SUBSCRIPT_ALL_HPP
#define HYPERTRIE_SUBSCRIPT_ALL_HPP

#include "Dice/einsum/internal/new_subscript/AbstractSubscript.hpp"
#include "Dice/einsum/internal/new_subscript/Subscript.hpp"
#include "Dice/einsum/internal/new_subscript/SubscriptCartesian.hpp"
#include "Dice/einsum/internal/new_subscript/SubscriptJoin.hpp"
#include "Dice/einsum/internal/new_subscript/SubscriptLeftJoin.hpp"
#include "Dice/einsum/internal/new_subscript/SubscriptOperand.hpp"
#include "Dice/einsum/internal/new_subscript/SubscriptUnion.hpp"


namespace einsum::internal::new_subscript {

	std::shared_ptr<SubscriptJoin> SubscriptJoin::make_triple_pattern(OperandLabels operand_labels) {
		auto tmp = std::make_shared<SubscriptJoin>();
		tmp->append(std::make_shared<SubscriptOperand>(operand_labels));
		return tmp;
	}

	std::shared_ptr<SubscriptJoin> SubscriptJoin::make_basic_graph_pattern(std::initializer_list<OperandLabels> operands_labels) {
		auto tmp = std::make_shared<SubscriptJoin>();
		for (const OperandLabels &operand_labels : operands_labels)
			tmp->append(std::make_shared<SubscriptOperand>(operand_labels));
		return tmp;
	}

	auto SubscriptJoin::append(std::initializer_list<Label> operand_labels) {
		join_operands.push_back(SubscriptOperand::make(operand_labels));
		return this;
	}

}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPT_ALL_HPP
