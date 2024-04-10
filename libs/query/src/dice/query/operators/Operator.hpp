#ifndef QUERY_OPERATOR_HPP
#define QUERY_OPERATOR_HPP

#include "dice/query/operators/CartesianOperator.hpp"
#include "dice/query/operators/CountOperator.hpp"
#include "dice/query/operators/EntryGeneratorOperator.hpp"
#include "dice/query/operators/JoinOperator.hpp"
#include "dice/query/operators/LeftJoinOperator.hpp"
#include "dice/query/operators/ResolveOperator.hpp"
#include "dice/query/operators/UnionOperator.hpp"

#include "Operator_predeclare.hpp"

namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline Operation next_op(OperandDependencyGraph &,
							 Query<htt_t, allocator_type> const &);

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_result_done>
	inline std::conditional_t<all_result_done, Entry<value_type, htt_t> const &, std::generator<Entry<value_type, htt_t> const &>>
	get_sub_operator(OperandDependencyGraph &odg,
					 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					 Query<htt_t, allocator_type> const &query,
					 Entry<value_type, htt_t> &entry) {
		switch (next_op(odg, query)) {
			case Operation::Join: {
				if constexpr (all_result_done)
					return JoinOperator<value_type, htt_t, allocator_type>::single_result(odg, operands, query, entry);
				else
					return JoinOperator<value_type, htt_t, allocator_type>::generator(odg, operands, query, entry);
			}
			case Operation::LeftJoin: {
				if constexpr (all_result_done)
					return LeftJoinOperator<value_type, htt_t, allocator_type>::single_result(odg, operands, query, entry);
				else
					return LeftJoinOperator<value_type, htt_t, allocator_type>::generator(odg, operands, query, entry);
			}
			case Operation::Resolve: {
				if constexpr (all_result_done)
					return ResolveOperator<value_type, htt_t, allocator_type>::single_result(odg, operands, query, entry);
				else
					return ResolveOperator<value_type, htt_t, allocator_type>::generator(odg, operands, query, entry);
			}
			case Operation::Count: {
				if constexpr (all_result_done)
					return CountOperator<value_type, htt_t, allocator_type>::single_result(odg, operands, query, entry);
				else
					return CountOperator<value_type, htt_t, allocator_type>::generator(odg, operands, query, entry);
			}
			case Operation::Cartesian: {
				if constexpr (all_result_done) {
					if (not odg.optional_cartesian())
						return CartesianOperator<value_type, htt_t, allocator_type>::single_result(odg, operands, query, entry);
					else
						return CartesianOperator<value_type, htt_t, allocator_type, true>::single_result(odg, operands, query, entry);
				}
				else {
					if (not odg.optional_cartesian())
						return CartesianOperator<value_type, htt_t, allocator_type>::generator(odg, operands, query, entry);
					else
						return CartesianOperator<value_type, htt_t, allocator_type, true>::generator(odg, operands, query, entry);
				}
			}
			case Operation::Union: {
				if constexpr (all_result_done)
					return UnionOperator<value_type, htt_t, allocator_type>::single_result(odg, operands, query, entry);
				else
					return UnionOperator<value_type, htt_t, allocator_type>::generator(odg, operands, query, entry);
			}
			case Operation::EntryGenerator: {
				if constexpr (all_result_done)
					return EntryGeneratorOperator<value_type, htt_t, allocator_type>::single_result(odg, operands, query, entry);
				else
					return EntryGeneratorOperator<value_type, htt_t, allocator_type>::generator(odg, operands, query, entry);
			}
			default:
				throw std::invalid_argument{"subscript is of an undefined type."};
		}
	};

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline void clear_used_entry_poss(Entry<value_type, htt_t> &entry,
									  OperandDependencyGraph &graph,
									  Query<htt_t, allocator_type> const &query) noexcept {
		for (auto const &result_pos : query.get_odg_projected_vars_positions(graph)) {
				entry[result_pos] = {};
		}
	}

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline Operation next_op(OperandDependencyGraph &odg,
							 Query<htt_t, allocator_type> const &query) {
		auto &operation_type = query.get_odg_operator_type(odg);
		if (operation_type != Operation::NoOp)
			return operation_type;
		if (odg.size() == 0) { // e.g. ->
			operation_type = Operation::EntryGenerator;
			return Operation::EntryGenerator;
		}
		else if (odg.size() == 1) { // only one operand left
			auto contains_proj_var = [&](char var) { return query.contains_proj_var(var); };
			if (odg.operands_var_ids_set().size() == odg.operand_var_ids(0).size()) {
				if (std::none_of(odg.operands_var_ids_set().begin(), odg.operands_var_ids_set().end(), contains_proj_var)) {// e.g. b->
					operation_type = Operation::Count;
					return Operation::Count;
				}
				else if (std::all_of(odg.operands_var_ids_set().begin(), odg.operands_var_ids_set().end(), contains_proj_var)) {// e.g. a->a or aa->a
					operation_type = Operation::Resolve;
					return Operation::Resolve;
				}
			}
			// fallthrough, e.g. ab->a
		} else { // more than one operand left
			if (odg.union_components().size() > 1) {// e.g. {a,b}{ab}->ab
				operation_type = Operation::Union;
				return Operation::Union;
			}
			if (odg.cartesian_components().size() > 1) {// e.g. a,b->ab
				operation_type = Operation::Cartesian;
				return Operation::Cartesian;
			}
			if (odg.isc_operands().size() < odg.size()) {// e.g. a,[ab]->ab
				operation_type = Operation::LeftJoin;
				return Operation::LeftJoin;
			}
			// fallthrough
		}
		operation_type = Operation::Join;
		return Operation::Join;
	}

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline static std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>
	extract_operands(OperandDependencyGraph &odg,
					 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands) {
		std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> sub_operands;
		for (const auto &original_op_pos : odg.operands_original_positions())
			sub_operands.emplace_back(operands[original_op_pos]);
		return sub_operands;
	}

}// namespace dice::query::operators

#endif//QUERY_OPERATOR_HPP
