#ifndef HYPERTRIE_COUNTOPERATOR_HPP
#define HYPERTRIE_COUNTOPERATOR_HPP

#include "dice/einsum/internal/operators/Operator_predeclare.hpp"

namespace dice::einsum::internal::operators {

	/**
	 * This operand is used if only one operand is left where all dimensions use different labels and those labels are not in the result, e.g., i-> or ij-> .
	 * It must be only called if the operand has non-zero entries. This Operator produces a single result and supports only single_result retrieval.
	 */
	template<typename value_type, hypertrie::HypertrieTrait htt_t, hypertrie::ByteAllocator allocator_type>
	struct CountOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;

		inline static std::generator<Entry<value_type, htt_t> const &> generator(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				[[maybe_unused]] Entry<value_type, htt_t> &entry_arg) {
			assert(false);// only one operand must be left to be resolved
			co_yield entry_arg;
		}

		/**
		 * @precondition operands.size() == 1
		 * @precondition !operands[0].empty()
		 *
		 * Will set the entries value to the appropriate value:
		 *      If bool_valued, this is true.
		 *      If the operand is bool-valued and the result type is not bool, this is the size of the operand.
		 *      Else (i.e. operand is non-bool-valued and result_type is not bool), this is the sum of the entries' values of the operand.
		 *
		 * @param entry_arg passed in as nothing set yet
		 * @return reference to entry_arg with new value set
		 */
		inline static Entry<value_type, htt_t> const &single_result(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			assert(operands.size() == 1);// only one operand must be left to be resolved
			assert(not operands[0].empty());

			if constexpr (bool_valued) {
				entry_arg.set_value(true);
			} else {
				if constexpr (hypertrie::HypertrieTrait_bool_valued<htt_t>) {
					entry_arg.set_value(operands[0].size());
				} else {
					context->check_time_out();
					value_type value{};
					// TODO: It would make sense to store the sum already in the node.
					for (const auto &item : operands[0]) {
						value += item.value();
					}
					entry_arg.set_value(value);
				}
			}
			return entry_arg;
		}
	};

}// namespace dice::einsum::internal::operators
#endif//HYPERTRIE_COUNTOPERATOR_HPP
