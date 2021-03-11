#ifndef HYPERTRIE_SUBSCRIPTNESTED_HPP
#define HYPERTRIE_SUBSCRIPTNESTED_HPP

#include "Dice/einsum/internal/new_subscript/Subscript.hpp"

/**
 * Example: xy,yz,(ab,cd->y)->yz
 * Notes on nested subscripts:
 *
 * in general:
 * - only result labels of nested subscript interacts with parent subscript
 * - for processing: normalize result labels so that each label is unique -> they can be joined through nesting
 *
 * Influence of outer and inner subscript semantics:
 * - outer: set -> inner: -- -> inner can be treated as set (if they don't use aggregates)
 * - outer: bag ; inner: bag. -> nothing to take care of
 * - outer: bag -> inner: set -> adjustments to joins needed
 * - If outer is bag-sem and inner is bag-sem. -> use normal joins
 */
namespace einsum::internal::new_subscript {
	class SubscriptNested : public SubscriptOperand {
	protected:
		OperandLabels operand_labels_;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTNESTED_HPP
