#ifndef HYPERTRIE_SUBSCRIPTOPERAND_HPP
#define HYPERTRIE_SUBSCRIPTOPERAND_HPP

#include "Dice/einsum/internal/new_subscript/Subscript.hpp"
namespace einsum::internal::new_subscript {
	class SubscriptOperand : Subscript {
	protected:
		OperandLabels operand_labels_;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTOPERAND_HPP
