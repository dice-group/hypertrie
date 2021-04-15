#ifndef HYPERTRIE_SUBSCRIPTOPERAND_HPP
#define HYPERTRIE_SUBSCRIPTOPERAND_HPP

#include <memory>

#include "Dice/einsum/internal/new_subscript/AbstractSubscript.hpp"
#include "Dice/einsum/internal/new_subscript/SubscriptCommons.hpp"

namespace einsum::internal::new_subscript {
	class SubscriptOperand : public AbstractSubscript, public std::enable_shared_from_this<SubscriptOperand> {
	protected:
		OperandLabels operand_labels_;

	public:
		explicit SubscriptOperand(OperandLabels operand_labels);

		explicit SubscriptOperand(std::initializer_list<Label> operand_labels);

		virtual ~SubscriptOperand();

		static std::shared_ptr<SubscriptOperand> make(std::initializer_list<Label> operand_labels = {});

		static std::shared_ptr<SubscriptOperand> make(OperandLabels operand_labels);

		const OperandLabels &operand_labels() const;

		OperandLabels &operand_labels();

		SubscriptOperand *operand_labels(std::initializer_list<Label> operand_labels);

		SubscriptOperand *operand_labels(OperandLabels operand_labels);

		std::size_t ndim() const noexcept;

		std::string str() const;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTOPERAND_HPP
