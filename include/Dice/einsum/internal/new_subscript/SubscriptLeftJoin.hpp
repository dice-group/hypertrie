#ifndef HYPERTRIE_SUBSCRIPTLEFTJOIN_HPP
#define HYPERTRIE_SUBSCRIPTLEFTJOIN_HPP

#include <list>
#include <memory>

#include "Dice/einsum/internal/new_subscript/Subscript.hpp"

namespace einsum::internal::new_subscript {
	/**
	 * ab,[ac]->bc
	 * Optimization:
	 * - Remove right-sides without projected variables.
	 * - Later: AND without variables that influence left-side variables (via filters)
	 */

	class SubscriptLeftJoin : public Subscript, public std::enable_shared_from_this<SubscriptLeftJoin> {
	protected:
		std::shared_ptr<Subscript> left_operand_;
		std::list<std::shared_ptr<Subscript>> right_operands_;

	public:
		static std::shared_ptr<SubscriptLeftJoin> make();

		virtual ~SubscriptLeftJoin();

		const std::shared_ptr<Subscript> &left_operand() const;

		std::shared_ptr<Subscript> &left_operand();

		SubscriptLeftJoin *left_operand(std::shared_ptr<Subscript> left_operand);

		const std::list<std::shared_ptr<Subscript>> &right_operands() const;

		std::list<std::shared_ptr<Subscript>> &right_operands();

		SubscriptLeftJoin *right_operands(std::list<std::shared_ptr<Subscript>> right_operands);

		SubscriptLeftJoin *append_right_operand(std::shared_ptr<Subscript> operand);

		std::string str() const;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTLEFTJOIN_HPP
