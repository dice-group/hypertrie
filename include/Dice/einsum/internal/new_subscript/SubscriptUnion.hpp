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
	class SubscriptUnion : public Subscript {
	protected:
		std::list<std::shared_ptr<Subscript>> union_operands_;

	public:
		virtual ~SubscriptUnion() {}


		[[nodiscard]] const std::list<std::shared_ptr<Subscript>> &union_operands() const {
			return union_operands_;
		}

		[[nodiscard]] std::list<std::shared_ptr<Subscript>> &union_operands() {
			return union_operands_;
		}

		void append_union_operand(std::shared_ptr<Subscript> union_operand) {
			union_operands_.push_back(union_operand);
		}

		std::string str() const {
			auto operands = fmt::join(union_operands_ | ranges::views::transform([&](auto &n) { return n->str(); }), "+");
			return fmt::format("({}){}",
							   fmt::to_string(operands),
							   (this->result_subscript()) ? this->result_subscript()->str() : "");
		}
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTUNION_HPP
