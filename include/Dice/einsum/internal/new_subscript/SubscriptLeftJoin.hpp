#ifndef HYPERTRIE_SUBSCRIPTLEFTJOIN_HPP
#define HYPERTRIE_SUBSCRIPTLEFTJOIN_HPP
#include "Dice/einsum/internal/new_subscript/Subscript.hpp"
namespace einsum::internal::new_subscript {
	class SubscriptLeftJoin {
	protected:
		std::shared_ptr<Subscript> left_operand;
		std::list<std::shared_ptr<Subscript>> right_operands;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTLEFTJOIN_HPP
