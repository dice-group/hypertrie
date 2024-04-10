#ifndef QUERY_RESOLVEOPERATOR_HPP
#define QUERY_RESOLVEOPERATOR_HPP

#include "Operator_predeclare.hpp"

namespace dice::query::operators {
	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct ResolveOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;

		inline static std::generator<Entry<value_type, htt_t> const &>
		generator(OperandDependencyGraph &odg,
				  [[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				  Query<htt_t, allocator_type> const &query,
				  Entry<value_type, htt_t> &entry_arg) {
			assert(operands.size() == 1);// only one operand must be left to be resolved
			assert(not operands[0].empty());
			clear_used_entry_poss<value_type, htt_t, allocator_type>(entry_arg, odg, query);
			auto const &operand_vars = odg.operand_var_ids(0);
			auto const &projected_vars_positions = query.projected_vars_positions();
			for (const auto &operand_entry : operands[0]) {
				query.check_time_out();
				for (size_t i = 0; i < operand_entry.size(); ++i) {
					assert(projected_vars_positions.contains(operand_vars[i]));
					entry_arg[projected_vars_positions.find(operand_vars[i])->second] = operand_entry[i];
				}
				entry_arg.value(1);
				co_yield entry_arg;
			}
		}

		inline static Entry<value_type, htt_t> const &
		single_result([[maybe_unused]] OperandDependencyGraph &odg,
					  [[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					  [[maybe_unused]] Query<htt_t, allocator_type> const &query,
					  Entry<value_type, htt_t> &entry_arg) {
			assert(false);// should never be scheduled
			return entry_arg;
		}
	};

}// namespace dice::query::operators
#endif//QUERY_RESOLVEOPERATOR_HPP
