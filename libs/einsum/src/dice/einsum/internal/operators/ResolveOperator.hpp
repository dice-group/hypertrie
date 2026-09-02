#ifndef HYPERTRIE_RESOLVEOPERATOR_HPP
#define HYPERTRIE_RESOLVEOPERATOR_HPP

#include "dice/einsum/internal/operators/Operator_predeclare.hpp"

namespace dice::einsum::internal::operators {
	/**
	 * This is the counter-part to the CountOperator. This operator processes a single operand that has only dimensions left that are part of the result, e.g. j->j, ij->ji.
	 * Because it always writes to the result it doesn't support retrieving only a single result. single_result is used only when all parts of the key are set already.
	 */
	template<typename value_type, hypertrie::HypertrieTrait htt_t, hypertrie::ByteAllocator allocator_type>
	struct ResolveOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;
		// TODO: make Count and Resolve a single operand
		/**
		 *
		 * @param entry_arg passed in as nothing set yet
		 * @return
		 */
		inline static std::generator<Entry<value_type, htt_t> const &> generator(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			// TODO: extend support also jj->j
			// clear_used_entry_poss is not necessary because the remaining positions are always overwritten
			assert(operands.size() == 1);// only one operand must be left to be resolved
			assert(not operands[0].empty());
			LabelPossInOperand const &label_poss_in_result = subscript->operand2resultMapping_ResolveType();
			for (auto const &operand_entry : operands[0]) {
				context->check_time_out();
				// TODO: support the same label multiple times in the result
				for (size_t i = 0; i < operand_entry.key().size(); ++i) {
					entry_arg.key()[label_poss_in_result[i]] = operand_entry.key()[i];
				}
				entry_arg.set_value(operand_entry.value());
				co_yield entry_arg;
			}
		}


		inline static Entry<value_type, htt_t> const &single_result(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				[[maybe_unused]] Entry<value_type, htt_t> &entry_arg) {
			assert(false);// should never be scheduled
			return entry_arg;
		}
	};

}// namespace dice::einsum::internal::operators
#endif//HYPERTRIE_RESOLVEOPERATOR_HPP
