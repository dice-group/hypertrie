#ifndef HYPERTRIE_SUBSCRIPT_HPP
#define HYPERTRIE_SUBSCRIPT_HPP

namespace einsum::internal::new_subscript {

	using Label = u_int8_t;
	using ResultLabels = std::vector<Label>;
	using OperandLabels = std::vector<Label>;

	class SubscriptCartesian;
	class SubscriptJoin;
	class SubscriptLeftJoin;
	class SubscriptOperand;
	class SubscriptUnion;

	class Subscript {
	protected:
		bool distinct_result;
		ResultLabels result_labels;
	};
}// namespace einsum::internal::new_subscript
#endif//HYPERTRIE_SUBSCRIPT_HPP
