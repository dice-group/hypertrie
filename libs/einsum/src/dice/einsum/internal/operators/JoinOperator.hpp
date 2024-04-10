#ifndef HYPERTRIE_JOINOPERATOR_HPP
#define HYPERTRIE_JOINOPERATOR_HPP

#include "dice/einsum/internal/CardinalityEstimation.hpp"
#include "dice/einsum/internal/operators/Operator_predeclare.hpp"

#include <dice/hypertrie/HashJoin.hpp>

namespace dice::einsum::internal::operators {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct JoinOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;

		inline static std::generator<Entry<value_type, htt_t> const &> generator(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t>(entry_arg, subscript);
			Label label = CardinalityEstimation<htt_t, allocator_type>::getMinCardLabel(operands, subscript, context);
			bool is_result_label = subscript->isResultLabel(label);
			LabelPos label_pos_in_result;
			if (is_result_label) {
				label_pos_in_result = subscript->getLabelPosInResult(label);
			}
			std::shared_ptr<Subscript> const &next_subscript = subscript->removeLabel(label);
			for (auto &[current_key_part, sub_operands] : hypertrie::HashJoin<htt_t, allocator_type>{operands, subscript->getLabelPossInOperands(label)}) {
				context->check_time_out();
				if (is_result_label) {
					entry_arg[label_pos_in_result] = current_key_part;
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
			for (auto &[current_key_part, sub_operands] : hypertrie::HashJoin<htt_t, allocator_type>{operands, subscript->getLabelPossInOperands(label)}) {
				context->check_time_out();
				auto const &entry = get_sub_operator<value_type, htt_t, allocator_type, true>(next_subscript, context, sub_operands, entry_arg);
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

}// namespace dice::einsum::internal::operators
#endif//HYPERTRIE_JOINOPERATOR_HPP
