#ifndef HYPERTRIE_SUBSCRIPT_HPP
#define HYPERTRIE_SUBSCRIPT_HPP

#include <fmt/format.h>
#include <range/v3/all.hpp>

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

	protected:
		auto result_labels_str() const {
			return fmt::format("->{}",
							   fmt::join(this->result_labels() | ranges::views::transform([](auto c) { return std::string(1, c); }), ""));
		}
	};
}// namespace einsum::internal::new_subscript
#endif//HYPERTRIE_SUBSCRIPT_HPP
