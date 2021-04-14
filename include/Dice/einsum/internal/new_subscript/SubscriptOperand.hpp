#ifndef HYPERTRIE_SUBSCRIPTOPERAND_HPP
#define HYPERTRIE_SUBSCRIPTOPERAND_HPP

#include "Dice/einsum/internal/new_subscript/Subscript.hpp"
namespace einsum::internal::new_subscript {
	class SubscriptOperand : public Subscript {
	protected:
		OperandLabels operand_labels_;

	public:
		explicit SubscriptOperand(OperandLabels operandLabels) : operand_labels_(std::move(operandLabels)) {}

		explicit SubscriptOperand(std::initializer_list<Label> operandLabels) : operand_labels_(std::move(operandLabels)) {}

		virtual ~SubscriptOperand() {}

		const OperandLabels &operand_labels() const { return operand_labels_; }

		OperandLabels &operand_labels() { return operand_labels_; }

		std::size_t ndim() const noexcept {
			return operand_labels_.size();
		}

		std::string str() const {
			return fmt::format("{}{}",
							   fmt::join(this->operand_labels() | ranges::views::transform([](auto c) { return std::string(1, c); }), ""),
							   (this->result_subscript()) ? this->result_subscript()->str() : "");
		}
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTOPERAND_HPP
