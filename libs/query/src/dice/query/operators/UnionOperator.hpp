#ifndef QUERY_UNIONOPERATOR_HPP
#define QUERY_UNIONOPERATOR_HPP

#include "Operator_predeclare.hpp"

namespace dice::query::operators {
	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct UnionOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;

	private:
		inline static std::pair<std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>>, std::vector<std::vector<size_t>>>
		init_union(std::vector<OperandDependencyGraph> &union_components,
				   std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				   Query<htt_t, allocator_type> const &query) {
			std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>> sub_operandss(union_components.size());
			std::vector<std::vector<size_t>> result_poss(union_components.size());
			for (size_t i = 0; i < union_components.size(); i++) {
				sub_operandss[i] = extract_operands(union_components[i], operands);
				for (auto const &label : union_components[i].operands_var_ids_set()) {
					if (query.contains_proj_var(label))
						result_poss[i].push_back(query.projected_var_position(label));
				}
			}
			return std::make_pair(sub_operandss, result_poss);
		}

	public:
		inline static std::generator<Entry<value_type, htt_t> const &>
		generator(OperandDependencyGraph &odg,
				  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				  Query<htt_t, allocator_type> const &query,
				  Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t, allocator_type>(entry_arg, odg, query);
			auto &union_comps = odg.union_components();
			auto [sub_operandss, result_poss] = init_union(union_comps, operands, query);
			for (size_t i = 0; i < union_comps.size(); i++) {
				query.check_time_out();
				if (not query.all_result_done(union_comps[i])) {
					co_yield std::elements_of(get_sub_operator<value_type, htt_t, allocator_type, false>(union_comps[i], sub_operandss[i], query, entry_arg));
				} else {
					const auto &entry = get_sub_operator<value_type, htt_t, allocator_type, true>(union_comps[i], sub_operandss[i], query, entry_arg);
					if (entry.value()) {
						co_yield entry_arg;
					}
				}
				clear_used_entry_poss<value_type, htt_t, allocator_type>(entry_arg, union_comps[i], query);
			}
		}

		inline static Entry<value_type, htt_t> const &
		single_result(OperandDependencyGraph &odg,
					  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					  Query<htt_t, allocator_type> const &query,
					  Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t, allocator_type>(entry_arg, odg, query);
			auto &union_comps = odg.union_components();
			auto [sub_operandss, result_poss] = init_union(union_comps, operands, query);
			[[maybe_unused]] value_type value = 0;
			for (size_t i = 0; i < union_comps.size(); i++) {
				query.check_time_out();
				const auto &entry = get_sub_operator<value_type, htt_t, allocator_type, true>(union_comps[i], sub_operandss[i], query, entry_arg);
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

#endif//QUERY_UNIONOPERATOR_HPP
