#ifndef HYPERTRIE_SUBSCRIPTUNION_HPP
#define HYPERTRIE_SUBSCRIPTUNION_HPP

#include <list>
#include <memory>

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
	class SubscriptUnion : public Subscript, public std::enable_shared_from_this<SubscriptUnion> {
	protected:
		std::list<std::shared_ptr<Subscript>> union_operands_;

	public:
		SubscriptUnion();

		virtual ~SubscriptUnion();

		static std::shared_ptr<SubscriptUnion> make();

		[[nodiscard]] const std::list<std::shared_ptr<Subscript>> &union_operands() const;
		[[nodiscard]] std::list<std::shared_ptr<Subscript>> &union_operands();

		SubscriptUnion *append_union_operand(std::shared_ptr<Subscript> union_operand);

		std::string str() const;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTUNION_HPP
