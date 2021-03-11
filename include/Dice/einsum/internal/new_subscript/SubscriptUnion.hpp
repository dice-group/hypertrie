#ifndef HYPERTRIE_SUBSCRIPTUNION_HPP
#define HYPERTRIE_SUBSCRIPTUNION_HPP

#include "Dice/einsum/internal/new_subscript/Subscript.hpp"
namespace einsum::internal::new_subscript {
	/**
	 * Example: ab,cd+ac,de+af->a
	 * interpret as: (ab,cd->abcd)+(ac,de->acde)+(af->af)->a
	 * TODO: check if that is in line with sparql semantics
	 * interesting cases for processing:
	 * - (x+y),x->x
	 * - ((x+x)+x)->x
	 */
	class SubscriptUnion {
	protected:
		std::list<std::shared_ptr<Subscript>> union_operands;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTUNION_HPP
