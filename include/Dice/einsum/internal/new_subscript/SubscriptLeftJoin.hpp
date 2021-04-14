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
		std::shared_ptr<Subscript> left_operand_;
		std::list<std::shared_ptr<Subscript>> right_operands_;

	public:
		virtual ~SubscriptLeftJoin() {}

		const std::shared_ptr<Subscript> &left_operand() const {
			return left_operand_;
		}

		std::shared_ptr<Subscript> &left_operand() {
			return left_operand_;
		}

		const std::list<std::shared_ptr<Subscript>> &right_operands() const {
			return right_operands_;
		}

		std::list<std::shared_ptr<Subscript>> &right_operands() {
			return right_operands_;
		}

		void append_right_operand(std::shared_ptr<Subscript> operand) {
			right_operands_.push_back(operand);
		}

		std::string str() const {
			return fmt::format("({},[{}]){}",
							   left_operand_->str(),
							   fmt::join(right_operands_ | ranges::views::transform([&](auto &n) { return n->str(); }), "],["),
							   (this->result_subscript()) ? this->result_subscript()->str() : "");
		}
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTLEFTJOIN_HPP
