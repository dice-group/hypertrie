#ifndef HYPERTRIE_SUBSCRIPTOPERAND_HPP
#define HYPERTRIE_SUBSCRIPTOPERAND_HPP

#include "Dice/einsum/internal/new_subscript/Subscript.hpp"
namespace einsum::internal::new_subscript {
	class SubscriptOperand : public Subscript {
	protected:
		OperandLabels operand_labels_;

	public:
		explicit SubscriptOperand(OperandLabels operandLabels) : operand_labels_(std::move(operandLabels)) {}

		virtual ~SubscriptOperand() {}


		std::size_t ndim() const noexcept {
			return operand_labels_.size();
		}

		std::string str(bool parent = true) const {
			return "";
		}
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTOPERAND_HPP
