#ifndef HYPERTRIE_SUBSCRIPTOPERAND_HPP
#define HYPERTRIE_SUBSCRIPTOPERAND_HPP

#include <memory>

#include <fmt/format.h>
#include <range/v3/all.hpp>

#include "Dice/einsum/internal/new_subscript/AbstractSubscript.hpp"

namespace einsum::internal::new_subscript {
	class SubscriptOperand : public AbstractSubscript, public std::enable_shared_from_this<SubscriptOperand> {
	protected:
		OperandLabels operand_labels_;

	public:
		explicit SubscriptOperand(OperandLabels operand_labels) : operand_labels_(std::move(operand_labels)) {}

		explicit SubscriptOperand(std::initializer_list<Label> operand_labels) : operand_labels_(std::move(operand_labels)) {}

		static std::shared_ptr<SubscriptOperand> make(std::initializer_list<Label> operand_labels = {}) {
			return std::make_shared<SubscriptOperand>(operand_labels);
		}

		static std::shared_ptr<SubscriptOperand> make(OperandLabels operand_labels) {
			return std::make_shared<SubscriptOperand>(operand_labels);
		}

		virtual ~SubscriptOperand() {}

		const OperandLabels &operand_labels() const { return operand_labels_; }

		OperandLabels &operand_labels() { return operand_labels_; }

		auto operand_labels(std::initializer_list<Label> operand_labels) {
			operand_labels_ = operand_labels;
			return this;
		}

		auto operand_labels(OperandLabels operand_labels) {
			operand_labels_ = std::move(operand_labels);
			return this;
		}

		std::size_t ndim() const noexcept {
			return operand_labels_.size();
		}

		std::string str() const {
			return fmt::format("{}",
							   fmt::join(this->operand_labels() | ranges::views::transform([](auto c) { return std::string(1, c); }), ""));
		}
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTOPERAND_HPP
