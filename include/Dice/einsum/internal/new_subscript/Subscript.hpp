#ifndef HYPERTRIE_SUBSCRIPT_HPP
#define HYPERTRIE_SUBSCRIPT_HPP

#include <initializer_list>
#include <memory>

#include "Dice/einsum/internal/new_subscript/AbstractSubscript.hpp"
#include "Dice/einsum/internal/new_subscript/ResultSubscript.hpp"
#include "Dice/einsum/internal/new_subscript/SubscriptCommons.hpp"


namespace einsum::internal::new_subscript {

	/**
	 * Bag-Semantics: xy,yz,f->x
	 * Set-Semantics: xy,yz,f-->x
	 */
	class Subscript : public AbstractSubscript {
	protected:
		std::shared_ptr<ResultSubscript> result_subscript_;

	public:
		virtual ~Subscript();
		;

		void set_result_subscript(ResultLabels result_labels = {}, bool distinct = false);

		void set_result_subscript(std::initializer_list<Label> result_labels, bool distinct = false);

		[[nodiscard]] std::shared_ptr<ResultSubscript> &result_subscript();
		[[nodiscard]] const std::shared_ptr<ResultSubscript> &result_subscript() const;

		[[nodiscard]] bool distinct() const noexcept(false);
		[[nodiscard]] bool &distinct() noexcept(false);

		[[nodiscard]] const ResultLabels &result_labels() const noexcept(false);
		[[nodiscard]] ResultLabels &result_labels() noexcept(false);
	};
}// namespace einsum::internal::new_subscript
#endif//HYPERTRIE_SUBSCRIPT_HPP
