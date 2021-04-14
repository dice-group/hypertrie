#include <Dice/einsum/internal/new_subscript/Subscript_all.hpp>

TEST_CASE("basic usage", "[new_subscript]") {
	using namespace einsum::internal::new_subscript;

	auto subscript = std::make_shared<SubscriptJoin>();
	// add triple patterns
	subscript->append(std::make_shared<SubscriptOperand>(OperandLabels{'a', 'b', 'c'}));
	subscript->append(std::make_shared<SubscriptOperand>(OperandLabels{'b', 'c'}));
	subscript->append(std::make_shared<SubscriptOperand>(OperandLabels{'c', 'e'}));

	// add a left join
	auto left_join = std::make_shared<SubscriptLeftJoin>();
	left_join->left_operand() = std::make_shared<SubscriptOperand>(OperandLabels{'a', 'c'});
	left_join->append_right_operand(std::make_shared<SubscriptOperand>(OperandLabels{'c', 'x'}));
	left_join->append_right_operand(SubscriptJoin::make_triple_pattern({'c', 'x'}));
	subscript->append(std::move(left_join));

	// add a union
	auto union_ = std::make_shared<SubscriptUnion>();
	union_->append_union_operand(SubscriptJoin::make_triple_pattern({'a', 'f'}));
	union_->append_union_operand(SubscriptJoin::make_basic_graph_pattern({{'a', 'f'}, {'a', 'g'}}));

	subscript->append(std::move(union_));

	subscript->result_labels() = {'a', 'e', 'f', 'x'};

	WARN(subscript->str());

	subscript->distinct_result() = true;

	WARN(subscript->str());
}