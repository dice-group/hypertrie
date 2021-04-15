#include <fmt/format.h>
#include <range/v3/all.hpp>

#include "Dice/einsum/internal/new_subscript/SubscriptOperand.hpp"
namespace einsum::internal::new_subscript {

	SubscriptOperand::SubscriptOperand(OperandLabels operand_labels) : operand_labels_(std::move(operand_labels)) {}
	SubscriptOperand::SubscriptOperand(std::initializer_list<Label> operand_labels) : operand_labels_(std::move(operand_labels)) {}
	SubscriptOperand::~SubscriptOperand() = default;
	std::shared_ptr<SubscriptOperand> SubscriptOperand::make(std::initializer_list<Label> operand_labels) {
		return std::make_shared<SubscriptOperand>(operand_labels);
	}
	std::shared_ptr<SubscriptOperand> SubscriptOperand::make(OperandLabels operand_labels) {
		return std::make_shared<SubscriptOperand>(operand_labels);
	}
	const OperandLabels &SubscriptOperand::operand_labels() const { return operand_labels_; }
	OperandLabels &SubscriptOperand::operand_labels() { return operand_labels_; }
	SubscriptOperand *SubscriptOperand::operand_labels(std::initializer_list<Label> operand_labels) {
		operand_labels_ = operand_labels;
		return this;
	}
	SubscriptOperand *SubscriptOperand::operand_labels(OperandLabels operand_labels) {
		operand_labels_ = std::move(operand_labels);
		return this;
	}
	std::size_t SubscriptOperand::ndim() const noexcept {
		return operand_labels_.size();
	}
	std::string SubscriptOperand::str() const {
		return fmt::format("{}",
						   fmt::join(this->operand_labels() | ranges::views::transform([](auto c) { return std::string(1, c); }), ""));
	}
}// namespace einsum::internal::new_subscript