#ifndef HYPERTRIE_SUBSCRIPTCARTESIAN_HPP
#define HYPERTRIE_SUBSCRIPTCARTESIAN_HPP
#include "Dice/einsum/internal/new_subscript/Subscript.hpp"

#include <list>

namespace einsum::internal::new_subscript {
	/**
	 * Example: xy,z->xz
	 */
	class SubscriptCartesian : public Subscript {
	protected:
		std::list<std::shared_ptr<Subscript>> cartesian_operands;

	public:
		virtual ~SubscriptCartesian() {}
		std::string str() const {
			return fmt::format("({}){}",
							   fmt::join(cartesian_operands | ranges::views::transform([&](auto &n) { return n->str(); }), ","),
							   (this->result_subscript()) ? this->result_subscript()->str() : "");
		}
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTCARTESIAN_HPP
