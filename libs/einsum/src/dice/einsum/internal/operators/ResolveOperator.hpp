#ifndef HYPERTRIE_RESOLVEOPERATOR_HPP
#define HYPERTRIE_RESOLVEOPERATOR_HPP

#include "dice/einsum/internal/operators/Operator_predeclare.hpp"

namespace dice::einsum::internal::operators {
	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct ResolveOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;

		inline static std::generator<Entry<value_type, htt_t> const &> generator(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			assert(operands.size() == 1);// only one operand must be left to be resolved
			assert(not operands[0].empty());
			clear_used_entry_poss<value_type, htt_t>(entry_arg, subscript);
			LabelPossInOperand const &label_poss_in_result = subscript->operand2resultMapping_ResolveType();
			for (auto const &operand_entry : operands[0]) {
				context->check_time_out();
				for (size_t i = 0; i < operand_entry.size(); ++i) {
					entry_arg[label_poss_in_result[i]] = operand_entry[i];
				}
				entry_arg.value(1);
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
