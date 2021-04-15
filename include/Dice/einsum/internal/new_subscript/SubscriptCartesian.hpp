#ifndef HYPERTRIE_SUBSCRIPTCARTESIAN_HPP
#define HYPERTRIE_SUBSCRIPTCARTESIAN_HPP

#include <list>


#include "Dice/einsum/internal/new_subscript/Subscript.hpp"

namespace einsum::internal::new_subscript {
	/**
	 * Example: xy,z->xz
	 */
	class SubscriptCartesian : public Subscript, public std::enable_shared_from_this<SubscriptCartesian> {
	protected:
		std::list<std::shared_ptr<Subscript>> cartesian_operands;

	public:
		virtual ~SubscriptCartesian();

		static std::shared_ptr<SubscriptCartesian> make();


		std::string str() const;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTCARTESIAN_HPP
