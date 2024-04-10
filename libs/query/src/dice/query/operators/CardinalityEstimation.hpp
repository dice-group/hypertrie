#ifndef QUERY_CARDINALITYESTIMATION_HPP
#define QUERY_CARDINALITYESTIMATION_HPP

#include <cmath>

#include <boost/container/flat_set.hpp>

#include "Operator_predeclare.hpp"

namespace dice::query::operators {


	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool Optional = false>
	struct CardinalityEstimation {

		static char getMinCardLabel(OperandDependencyGraph &odg,
									const std::vector<::dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> &operands,
									Query<htt_t, allocator_type> const &query) {
			boost::container::flat_set<char> const *var_ids_set = nullptr;
			if constexpr (not Optional)
				var_ids_set = &odg.operands_var_ids_set();
			else
				var_ids_set = &odg.non_optional_var_ids_set();
			if (var_ids_set->size() == 1)
				return *var_ids_set->begin();
			char min_var = *var_ids_set->begin();
			double min_cardinality = std::numeric_limits<double>::infinity();
			for (auto const &var : *var_ids_set) {
				if (odg.lonely_var_ids().contains(var) and
					std::find(query.projected_vars().begin(), query.projected_vars().end(), var) == query.projected_vars().end())
					continue;
				const double label_cardinality = calcCard(odg, operands, var);
				if (label_cardinality < min_cardinality) {
					min_cardinality = label_cardinality;
					min_var = var;
				}
			}
			return min_var;
		}

		static double estimate(OperandDependencyGraph &odg,
							   const std::vector<::dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> &operands,
							   Query<htt_t, allocator_type> const &q) {
			auto const &operandsLabelSet = odg.operands_var_ids_set();
			std::vector<double> operand_sizes(operands.size());
			for (size_t i = 0; i < operands.size(); ++i)
				operand_sizes[i] = operands[i].size();

			std::vector<double> var_factors;
			for (auto const &var : operandsLabelSet)
				if (not odg.lonely_var_ids().contains(var) or
					std::find(q.projected_vars().begin(), q.projected_vars().end(), var) != q.projected_vars().end())
					var_factors.push_back(calcCard(odg, operands, var));

			double card = 1;
			bool order = operand_sizes.size() < var_factors.size();
			auto &shorter_vec = order ? operand_sizes : var_factors;
			auto &longer_vec = order ? var_factors : operand_sizes;

			size_t i = 0;
			for (; i < shorter_vec.size(); ++i)
				card *= shorter_vec[i] * longer_vec[i];

			for (; i < longer_vec.size(); ++i)
				card *= longer_vec[i];

			return card;
		}

	protected:
		static double calcCard(OperandDependencyGraph &odg,
							   const std::vector<::dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> &operands,
							   const char var) {
			// get operands that have the label
			std::vector<uint8_t> const *operands_positions = nullptr;
			if constexpr (not Optional)
				operands_positions = &odg.operands_with_var_id(var);
			else
				operands_positions = &odg.isc_operands_with_var_id(var);
			std::vector<double> op_dim_cardinalities(operands_positions->size(), 1.0);
			auto label_count = 0;
			auto min_dim_card = std::numeric_limits<size_t>::max();
			boost::container::flat_set<size_t> sizes{};

			auto const &var_positions = odg.var_ids_positions_in_operands(var);
			// iterate the operands that hold the label
			for (size_t i = 0; i < operands_positions->size(); ++i) {
				auto const &op_pos = (*operands_positions)[i];
				auto const &operand = operands[op_pos];
				auto const op_dim_cards = operand.get_cards(var_positions[op_pos]);
				auto const min_op_dim_card = *std::min_element(op_dim_cards.cbegin(), op_dim_cards.cend());
				auto const max_op_dim_card = *std::max_element(op_dim_cards.cbegin(), op_dim_cards.cend());

				for (const auto &op_dim_card : op_dim_cards)
					sizes.insert(op_dim_card);

				label_count += op_dim_cards.size();
				// update minimal dimension cardinality
				if (min_op_dim_card < min_dim_card)
					min_dim_card = min_op_dim_card;

				op_dim_cardinalities[i] = double(max_op_dim_card);//
			}

			auto const min_dim_card_d = double(min_dim_card);

			double card = std::accumulate(op_dim_cardinalities.cbegin(), op_dim_cardinalities.cend(), double(1),
										  [&](double a, double b) {
											  return a * min_dim_card_d / b;
										  }) /
						  sizes.size();
			return card;
		}
	};
}// namespace dice::query::operators
#endif//QUERY_CARDINALITYESTIMATION_HPP
