#ifndef HYPERTRIE_SUBSCRIPTMINUS_HPP
#define HYPERTRIE_SUBSCRIPTMINUS_HPP

#include <list>

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
	class SubscriptMinus : public Subscript, public std::enable_shared_from_this<SubscriptMinus> {
	protected:
		std::shared_ptr<Subscript> subtrahend_;
		std::list<std::shared_ptr<Subscript>> minuends_;

	public:
		static std::shared_ptr<SubscriptMinus> make();

		virtual ~SubscriptMinus();

		const std::shared_ptr<Subscript> &subtrahend() const;

		std::shared_ptr<Subscript> &subtrahend();

		SubscriptMinus *subtrahend(std::shared_ptr<Subscript> subtrahend);

		const std::list<std::shared_ptr<Subscript>> &minuends() const;

		std::list<std::shared_ptr<Subscript>> &minuends();

		SubscriptMinus *minuends(std::list<std::shared_ptr<Subscript>> minuends);

		SubscriptMinus *append_minuend(std::shared_ptr<Subscript> operand);

		std::string str() const;
	};
}// namespace einsum::internal::new_subscript


#endif//HYPERTRIE_SUBSCRIPTMINUS_HPP
