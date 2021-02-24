#ifndef HYPERTRIE_SUBSCRIPTUNION_HPP
#define HYPERTRIE_SUBSCRIPTUNION_HPP

#include "Dice/einsum/internal/new_subscript/Subscript.hpp"
namespace einsum::internal::new_subscript {
	class SubscriptUnion {
	protected:
		std::list<std::shared_ptr<Subscript>> union_operands;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTUNION_HPP
