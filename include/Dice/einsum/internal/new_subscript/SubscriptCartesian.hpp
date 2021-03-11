#ifndef HYPERTRIE_SUBSCRIPTCARTESIAN_HPP
#define HYPERTRIE_SUBSCRIPTCARTESIAN_HPP
#include "Dice/einsum/internal/new_subscript/Subscript.hpp"


namespace einsum::internal::new_subscript {
	/**
	 * Example: xy,z->xz
	 */
	class SubscriptCartesian : public Subscript {
	protected:
		std::list<std::shared_ptr<Subscript>> cartesian_operands;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTCARTESIAN_HPP
