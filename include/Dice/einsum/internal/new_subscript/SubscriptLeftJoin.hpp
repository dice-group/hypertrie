#ifndef HYPERTRIE_SUBSCRIPTLEFTJOIN_HPP
#define HYPERTRIE_SUBSCRIPTLEFTJOIN_HPP
#include "Dice/einsum/internal/new_subscript/Subscript.hpp"
namespace einsum::internal::new_subscript {
	/**
	 * ab,[ac]->bc
	 * Optimization:
	 * - Remove right-sides without projected variables.
	 * - Later: AND without variables that influence left-side variables (via filters)
	 */

	class SubscriptLeftJoin : public Subscript {
	protected:
		std::shared_ptr<Subscript> left_operand;
		std::list<std::shared_ptr<Subscript>> right_operands;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTLEFTJOIN_HPP
