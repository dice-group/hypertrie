#ifndef HYPERTRIE_JOINOPERATOR_HPP
#define HYPERTRIE_JOINOPERATOR_HPP

#include "dice/einsum/internal/CardinalityEstimation.hpp"
#include "dice/einsum/internal/operators/Operator_predeclare.hpp"

#include <dice/hypertrie/HashJoin.hpp>

namespace dice::einsum::internal::operators {

	/**
	 * Executes a join (contraction) of a single label.
	 * @tparam value_type
	 * @tparam htt_t
	 * @tparam allocator_type
	 */
	template<typename value_type, hypertrie::HypertrieTrait htt_t, hypertrie::ByteAllocator allocator_type>
	struct JoinOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>; // true if resulting entries are bool-valued
		static constexpr bool bool_valued_ops = hypertrie::HypertrieTrait_bool_valued<htt_t>; // true if operands are bool-valued

		inline static std::generator<Entry<value_type, htt_t> const &> generator(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t>(entry_arg, subscript);
			Label const label = CardinalityEstimation<htt_t, allocator_type>::getMinCardLabel(operands, subscript, context);
			bool const is_result_label = subscript->isResultLabel(label);
			LabelPos label_pos_in_result;
			if (is_result_label) {
				label_pos_in_result = subscript->getLabelPosInResult(label);
			}
			std::shared_ptr<Subscript> const &next_subscript = subscript->removeLabel(label);
			if constexpr (bool_valued or bool_valued_ops) {// -> no need to track a value beyond existence
				for (auto &[current_key_part, sub_operands] : hypertrie::HashJoin<htt_t, allocator_type, false, value_type>{operands, subscript->getLabelPossInOperands(label)}) {
					context->check_time_out();
					if (is_result_label) {
						entry_arg.key()[label_pos_in_result] = current_key_part;
					}
					if (next_subscript->all_result_done) {
						auto const &entry = get_sub_operator<value_type, htt_t, allocator_type, true>(next_subscript, context, sub_operands, entry_arg);
						if (entry.value()) {
							co_yield entry;
						}
					} else {
						co_yield std::elements_of(get_sub_operator<value_type, htt_t, allocator_type, false>(next_subscript, context, sub_operands, entry_arg));
					}
				}
			} else {//  non-boolean ops and result not bool valued -> need to track exact count
				for (auto &[current_key_part, join_result] : hypertrie::HashJoin<htt_t, allocator_type, false, value_type>{operands, subscript->getLabelPossInOperands(label)}) {
					auto &&[sub_operands, value] = join_result;

					context->check_time_out();
					if (is_result_label) {
						entry_arg.key()[label_pos_in_result] = current_key_part;
					}
					if (next_subscript->all_result_done) {
						auto const &entry = get_sub_operator<value_type, htt_t, allocator_type, true>(next_subscript, context, sub_operands, entry_arg);
						if (entry.value()) {
							entry_arg.set_value(entry.value() * value);
							co_yield entry;
						}
					} else {
						for (auto const &entry : get_sub_operator<value_type, htt_t, allocator_type, false>(next_subscript, context, sub_operands, entry_arg)) {
							if (entry.value()) {
								entry_arg.set_value(entry.value() * value);
								co_yield entry;
							}
						}
					}
				}
			}
		}


		inline static Entry<value_type, htt_t> const &single_result(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t>(entry_arg, subscript);
			Label label = CardinalityEstimation<htt_t, allocator_type>::getMinCardLabel(operands, subscript, context);
			std::shared_ptr<Subscript> const &next_subscript = subscript->removeLabel(label);
			[[maybe_unused]] value_type value = 0;
			if constexpr (bool_valued or bool_valued_ops) {// -> don't need to track a value beyond existence
				for (auto &[current_key_part, sub_operands] : hypertrie::HashJoin<htt_t, allocator_type, false, value_type>{operands, subscript->getLabelPossInOperands(label)}) {
					auto const &entry = get_sub_operator<value_type, htt_t, allocator_type, true>(next_subscript, context, sub_operands, entry_arg);
					if constexpr (bool_valued) {
						if (entry.value()) {
							return entry;
						}
					} else {
						value += entry.value();
					}
				}
			} else {// non-boolean ops and result not bool valued -> need to track exact count
				for (auto &[current_key_part, join_result] : hypertrie::HashJoin<htt_t, allocator_type, false, value_type>{operands, subscript->getLabelPossInOperands(label)}) {
					auto &&[sub_operands, join_value] = join_result;
					context->check_time_out();
					auto const &entry = get_sub_operator<value_type, htt_t, allocator_type, true>(next_subscript, context, sub_operands, entry_arg);
					value += entry.value() * join_value;
				}
			}
			entry_arg.set_value(value);
			return entry_arg;
		}
	};

}// namespace dice::einsum::internal::operators
#endif//HYPERTRIE_JOINOPERATOR_HPP
