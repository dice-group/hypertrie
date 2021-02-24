#ifndef HYPERTRIE_SUBSCRIPTJOIN_HPP
#define HYPERTRIE_SUBSCRIPTJOIN_HPP
#include "Dice/einsum/internal/new_subscript/Subscript.hpp"

namespace einsum::internal::new_subscript {
	class SubscriptJoin : public Subscript {
	protected:
		std::list<std::shared_ptr<Subscript>> join_operands;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTJOIN_HPP
