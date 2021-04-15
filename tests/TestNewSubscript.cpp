#include <Dice/einsum/internal/new_subscript/Subscript_all.hpp>

TEST_CASE("basic usage", "[new_subscript]") {
	using namespace einsum::internal::new_subscript;
	// subscripts are entirely based on pointers/shared-pointers. So don't use their the constructors but their make methods.
	// constructors might be set to private later.

	// add triple patterns
	auto subscript = SubscriptJoin::make()
							 ->append(SubscriptOperand::make({'c', 'e'}))// verbose way
							 ->append({'b', 'c'})                        // easier
							 ->append({'a', 'b', 'c'})
							 ->shared_from_this();// checkout https://en.cppreference.com/w/cpp/memory/enable_shared_from_this/shared_from_this

	// add "sub-query"
	// only result labels are relevant to the outside
	auto sub_subscript = SubscriptJoin::make_basic_graph_pattern({{'a', 'b', 'c'},
																  {'b', 'c'},
																  {'c', 'e'}});
	// it becomes a subquery by specifying result_labels and distinct
	// TODO: make set_result_subscript chainable (low priority)
	sub_subscript->set_result_subscript({'a'}, false);

	subscript->append(std::move(sub_subscript));

	// add a left join
	auto left_join =
			std::make_shared<SubscriptLeftJoin>()
					->left_operand(SubscriptJoin::make_triple_pattern(OperandLabels{'a', 'c'}))
					->append_right_operand(SubscriptJoin::make_triple_pattern({'c', 'x'}))
					->append_right_operand(SubscriptJoin::make_triple_pattern({'c', 'y'}))
					->shared_from_this();
	subscript->append(std::move(left_join));

	// add a union
	subscript->append(SubscriptUnion::make()
							  ->append_union_operand(SubscriptJoin::make_triple_pattern({'a', 'f'}))
							  ->append_union_operand(SubscriptJoin::make_basic_graph_pattern({{'a', 'f'}, {'a', 'g'}}))
							  ->shared_from_this());

	// add a minus
	subscript->append(SubscriptMinus::make()
							  ->subtrahend(SubscriptJoin::make_triple_pattern({'a', 'f'}))
							  ->append_minuend(SubscriptJoin::make_basic_graph_pattern({{'a', 'f'}, {'a', 'g'}}))
							  ->shared_from_this());

	subscript->set_result_subscript({'a', 'e', 'f', 'x'});

	WARN(subscript->str());

	subscript->distinct() = true;

	WARN(subscript->str());
}