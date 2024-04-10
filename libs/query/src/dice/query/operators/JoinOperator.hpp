#ifndef QUERY_JOINOPERATOR_HPP
#define QUERY_JOINOPERATOR_HPP

#include "CardinalityEstimation.hpp"
#include "Operator_predeclare.hpp"

#include <dice/hypertrie/HashJoin.hpp>

namespace dice::query::operators {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct JoinOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;

		inline static std::generator<Entry<value_type, htt_t> const &>
		generator(OperandDependencyGraph &odg,
				  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				  Query<htt_t, allocator_type> const &query,
				  Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t, allocator_type>(entry_arg, odg, query);
			char eval_var = CardinalityEstimation<htt_t, allocator_type>::getMinCardLabel(odg, operands, query);
			bool is_proj_var = query.contains_proj_var(eval_var);
			uint8_t proj_var_pos;
			if (is_proj_var)
				proj_var_pos = query.projected_var_position(eval_var);
			auto &sub_odg = odg.remove_var_id(eval_var);
			auto sub_odg_all_result_done = query.all_result_done(sub_odg);
			for (auto &[current_key_part, sub_operands] : hypertrie::HashJoin<htt_t, allocator_type>{operands,
																									 odg.var_ids_positions_in_operands(eval_var)}) {
				query.check_time_out();
				if (is_proj_var)
					entry_arg[proj_var_pos] = current_key_part;
				if (sub_odg_all_result_done) {
					const auto &entry = get_sub_operator<value_type, htt_t, allocator_type, true>(sub_odg, sub_operands, query, entry_arg);
					if (entry.value())
						co_yield entry;
				} else {
					co_yield std::elements_of(get_sub_operator<value_type, htt_t, allocator_type, false>(sub_odg, sub_operands, query, entry_arg));
				}
			}
		}

		inline static Entry<value_type, htt_t> const &
		single_result(OperandDependencyGraph &odg,
					  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					  Query<htt_t, allocator_type> const &query,
					  Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t, allocator_type>(entry_arg, odg, query);
			auto eval_var = CardinalityEstimation<htt_t, allocator_type>::getMinCardLabel(odg, operands, query);
			auto &sub_odg = odg.remove_var_id(eval_var);
			[[maybe_unused]] value_type value = 0;
			for (auto &[current_key_part, sub_operands] : hypertrie::HashJoin<htt_t, allocator_type>{operands,
																									 odg.var_ids_positions_in_operands(eval_var)}) {
				query.check_time_out();
				const auto &entry = get_sub_operator<value_type, htt_t, allocator_type, true>(sub_odg, sub_operands, query, entry_arg);
				if (entry.value()) {
					if constexpr (bool_valued) {
						return entry;
					} else {
						value += entry.value();
					}
				}
			}
			entry_arg.value(value);
			return entry_arg;
		}
	};

}// namespace dice::query::operators
#endif//QUERY_JOINOPERATOR_HPP
