#ifndef HYPERTRIE_SUBSCRIPTJOIN_HPP
#define HYPERTRIE_SUBSCRIPTJOIN_HPP

#include <list>

#include "Dice/einsum/internal/new_subscript/AbstractSubscript.hpp"
#include "Dice/einsum/internal/new_subscript/Subscript.hpp"

namespace einsum::internal::new_subscript {
	/**
	 * Example: xy,yz->yz
	 */
	class SubscriptJoin : public Subscript, public std::enable_shared_from_this<SubscriptJoin> {
	protected:
		std::list<std::shared_ptr<AbstractSubscript>> join_operands;

	public:
		virtual ~SubscriptJoin();

		SubscriptJoin *append(std::shared_ptr<AbstractSubscript> operand);
		SubscriptJoin *append(std::initializer_list<Label> operand_labels);
		SubscriptJoin *append(OperandLabels operand_labels);

		static std::shared_ptr<SubscriptJoin> make();

		static std::shared_ptr<SubscriptJoin> make_triple_pattern(OperandLabels operand_labels);
		static std::shared_ptr<SubscriptJoin> make_triple_pattern(std::initializer_list<Label> operand_labels);

		static std::shared_ptr<SubscriptJoin> make_basic_graph_pattern(std::initializer_list<OperandLabels> operands_labels);

		std::string str() const;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTJOIN_HPP
