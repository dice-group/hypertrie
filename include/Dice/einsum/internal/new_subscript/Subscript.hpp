#ifndef HYPERTRIE_SUBSCRIPT_HPP
#define HYPERTRIE_SUBSCRIPT_HPP

#include <fmt/format.h>

namespace einsum::internal::new_subscript {

	using Label = u_int8_t;
	using ResultLabels = std::vector<Label>;
	using OperandLabels = std::vector<Label>;

	class SubscriptCartesian;
	class SubscriptJoin;
	class SubscriptLeftJoin;
	class SubscriptOperand;
	class SubscriptUnion;

	/**
	 * Bag-Semantics: xy,yz,f->x
	 * Set-Semantics: xy,yz,f-->x
	 */
	class Subscript {
	protected:
		bool distinct_result_;
		ResultLabels result_labels_;

	public:
		virtual ~Subscript() {}

		[[nodiscard]] bool distinct_result() const { return distinct_result_; }
		[[nodiscard]] bool &distinct_result() { return distinct_result_; }

		[[nodiscard]] const ResultLabels &result_labels() const { return result_labels_; }
		[[nodiscard]] ResultLabels &result_labels() { return result_labels_; }

		virtual std::string str(bool parent = true) const = 0;
	};
}// namespace einsum::internal::new_subscript
#endif//HYPERTRIE_SUBSCRIPT_HPP
