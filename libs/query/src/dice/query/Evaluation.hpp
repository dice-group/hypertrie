#ifndef QUERY_EVALUATION_HPP
#define QUERY_EVALUATION_HPP

#include <memory>
#include <utility>

#include <dice/hypertrie.hpp>
#include <dice/hypertrie/internal/commons/generator.hpp>

#include "Commons.hpp"
#include "OperandDependencyGraph.hpp"
#include "Query.hpp"
#include "operators/Operator.hpp"

namespace dice::query {

	class Evaluation {
	public:
		/**
		 * @brief Checks if the provided query has at least one solution.
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param query
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static bool
		evaluate_ask(Query<htt_t, allocator_type> &query) {
			auto [pruned_odg, pruned_ops] = prune_empty_operands(query.operand_dependency_graph(), query.operands());
			if (pruned_odg.size() == 0)
				return false;
			auto [finalized_odg, finalized_ops] = remove_rank0_operands(pruned_odg, pruned_ops);
			auto solution = Entry<bool, htt_t>::make_filled(query.projected_vars().size(), {});
			auto result = operators::get_sub_operator<bool, htt_t, allocator_type, true>(finalized_odg, finalized_ops, query, solution);
			return result.value();
		}

		/**
		 * @brief Entry point for the evaluation of queries.
		 * <p> It first removes operands that are empty along with their dependent operands. </p>
		 * <p> It then removes operands that are scalars, whose value is true. Such operands do not affect the evaluation. </p>
		 * <p> It is responsible for calling the appropriate eval function. </p>
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @tparam Distinct
		 * @param query
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool Distinct = false>
		static std::conditional_t<Distinct, std::generator<Entry<bool, htt_t> const &>, std::generator<Entry<std::size_t, htt_t> const &>>
		evaluate(Query<htt_t, allocator_type> &query) {
			auto [pruned_odg, pruned_ops] = prune_empty_operands(query.operand_dependency_graph(), query.operands());
			if (pruned_odg.size() == 0)
				co_return;
			auto [finalized_odg, finalized_ops] = remove_rank0_operands(pruned_odg, pruned_ops);
			if constexpr (Distinct) {
				if (query.all_result_done(finalized_odg))
					co_yield eval_distinct_single(finalized_odg, finalized_ops, query);
				else
					co_yield std::elements_of(eval_distinct(finalized_odg, finalized_ops, query));
			} else {
				if (query.all_result_done(finalized_odg))
					co_yield eval_single(finalized_odg, finalized_ops, query);
				else
					co_yield std::elements_of(eval(finalized_odg, finalized_ops, query));
			}
		}

	private:
		/**
		 * @brief Evaluates a query. Duplicates are allowed.
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param odg
		 * @param operands
		 * @param query
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static std::generator<Entry<std::size_t, htt_t> const &>
		eval(OperandDependencyGraph &odg,
			 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> operands,
			 Query<htt_t, allocator_type> &query) {
			auto solution = Entry<std::size_t, htt_t>::make_filled(query.projected_vars().size(), {});
			co_yield std::elements_of(operators::get_sub_operator<std::size_t, htt_t, allocator_type, false>(odg, operands, query, solution));
		}

		/**
		 * @brief Returns a single size_t entry.
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param odg
		 * @param operands
		 * @param query
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static Entry<std::size_t, htt_t>
		eval_single(OperandDependencyGraph &odg,
					std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> operands,
					Query<htt_t, allocator_type> &query) {
			auto solution = Entry<std::size_t, htt_t>::make_filled(query.projected_vars().size(), {});
			return operators::get_sub_operator<std::size_t, htt_t, allocator_type, true>(odg, operands, query, solution);
		}

		/**
		 * @brief Evaluates a query and ensures that all entries are returned only once.
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param odg
		 * @param operands
		 * @param query
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static std::generator<Entry<bool, htt_t> const &>
		eval_distinct(OperandDependencyGraph &odg,
					  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> operands,
					  Query<htt_t, allocator_type> &query) {
			auto solution = Entry<bool, htt_t>::make_filled(query.projected_vars().size(), {});
			robin_hood::unordered_set<size_t, std::identity> found_entries{};
			for (auto const &sol : operators::get_sub_operator<bool, htt_t, allocator_type, false>(odg, operands, query, solution)) {
				const size_t hash = dice::hash::DiceHashwyhash<Entry<bool, htt_t>>()(sol);
				auto [_, is_new_entry] = found_entries.emplace(hash);
				if (is_new_entry)
					co_yield sol;
			}
		}

		/**
		 * @brief Returns a single bool entry.
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param odg
		 * @param operands
		 * @param query
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static Entry<bool, htt_t>
		eval_distinct_single(OperandDependencyGraph &odg,
							 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> operands,
							 Query<htt_t, allocator_type> &query) {
			auto solution = Entry<bool, htt_t>::make_filled(query.projected_vars().size(), {});
			return operators::get_sub_operator<bool, htt_t, allocator_type, true>(odg, operands, query, solution);
		}

		/**
		 * @brief Removes operands that are empty and their dependent operands.
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param odg
		 * @param ops
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static std::pair<OperandDependencyGraph, std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>>
		prune_empty_operands(OperandDependencyGraph &odg,
							 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &ops) {
			std::vector<uint8_t> empty_ops{};
			for (uint8_t i = 0; i < ops.size(); i++) {
				if (ops[i].empty())
					empty_ops.push_back(i);
			}
			if (empty_ops.empty())
				return std::make_pair(odg, ops);
			auto new_odg = odg.prune_graph(empty_ops);
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> new_ops{};
			for (auto pos : new_odg.operands_original_positions()) {
				new_ops.push_back(ops[pos]);
			}
			return std::make_pair(new_odg, new_ops);
		}

		/**
		 * @brief Removes scalar operands.
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param odg
		 * @param ops
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static std::pair<OperandDependencyGraph, std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>>
		remove_rank0_operands(OperandDependencyGraph &odg,
							  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &ops) {
			std::vector<uint8_t> rank0_ops{};
			for (size_t i = 0; i < ops.size(); i++) {
				if (ops[i].depth() == 0)
					rank0_ops.push_back(i);
			}
			if (rank0_ops.empty())
				return std::make_pair(odg, ops);
			auto new_odg = odg.remove_vertices(rank0_ops);
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> new_ops{};
			for (auto pos : new_odg.operands_original_positions()) {
				new_ops.push_back(ops[pos]);
			}
			return std::make_pair(new_odg, new_ops);
		}
	};


}// namespace dice::query

#endif//QUERY_EVALUATION_HPP
