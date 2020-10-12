#ifndef HYPERTRIE_LEFTJOINOPERATOR_HPP
#define HYPERTRIE_LEFTJOINOPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"
#include <Dice/hypertrie/internal/HashJoin.hpp>
#include <Dice/hypertrie/internal/LeftHashJoin.hpp>

namespace einsum::internal {

	template<typename value_type, HypertrieTrait tr_t>
	class LeftJoinOperator : public Operator<value_type, tr_t> {
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"
		using LeftJoin_t = hypertrie::LeftHashJoin<tr>;

		LeftJoin_t left_join;
		typename LeftJoin_t::iterator left_join_iterator;
		// the active label
        Label label = std::numeric_limits<Label>::max();// TODO: type the unusable value (max) explicitly
		// true: the active label appears in the result
        bool is_result_label = false;
        // the position of the active label in each operand
		LabelPossInOperands label_poss_in_ops;
        // the position of the active label in the result
		LabelPos label_pos_in_result;
		// the part of the key that was selected in the join
		key_part_type current_key_part;

		std::shared_ptr<Operator_t> sub_operator;
        std::shared_ptr<Subscript> next_subscript;

		bool ended_ = true;

	public:
		LeftJoinOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context> &context)
			: Operator_t(subscript->type, subscript, context, this) {}

		static bool ended(const void *self_raw) {
			auto &self = *static_cast<const LeftJoinOperator *>(self_raw);
			return self.ended_ or self.context->hasTimedOut();
		}

		static void load(void *self_raw, std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			static_cast<LeftJoinOperator *>(self_raw)->load_impl(std::move(operands), entry);
		}

		static std::size_t hash(const void *self_raw) {
			return static_cast<const LeftJoinOperator *>(self_raw)->subscript->hash();
		}

		static void next(void *self_raw) {
			LeftJoinOperator &self = *static_cast<LeftJoinOperator *>(self_raw);
			if constexpr (bool_value_type) {
				if (self.subscript->all_result_done) {
					assert(self.sub_operator);
					self.ended_ = true;
					return;
				}
			}
			self.sub_operator->next();
			self.find_next_valid();
		}

	private:

		void find_next_valid() {
			assert(left_join_iterator);
			while (sub_operator->ended()) {
				++left_join_iterator;
				if (left_join_iterator and not this->context->hasTimedOut()) {
					std::vector<std::optional<const_Hypertrie<tr>>> next_operands;
					std::tie(next_operands, current_key_part) = *left_join_iterator;
                    init_load_sub_operator(&next_operands);
				} else {
					ended_ = true;
					return;
				}
			}
			if (is_result_label)
				this->entry->key[label_pos_in_result] = current_key_part;

			if constexpr (_debugeinsum_)
				fmt::print("[{}]->{} {}\n", fmt::join(this->entry->key, ","), this->entry->value, this->subscript);
		}

		inline void load_impl(std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			if constexpr (_debugeinsum_) fmt::print("LeftJoin {}\n", this->subscript);

			this->entry = &entry;
			ended_ = false;
			Label last_label = label;
			label = this->subscript->getLeftJoinLabel();
			if (label != last_label) {
				label_poss_in_ops = this->subscript->getLabelPossInOperands(label);
				is_result_label = this->subscript->isResultLabel(label);
				if (is_result_label)
					label_pos_in_result = this->subscript->getLabelPosInResult(label);
			}
			// prepare next_subscript by removing the active label from the current subscript
			next_subscript = this->subscript->removeLabel(label);
            // prepare left join
			left_join = LeftJoin_t{operands, label_poss_in_ops};
			left_join_iterator = left_join.begin();
			// check if left join has entries
			if (left_join_iterator) {
				std::vector<std::optional<const_Hypertrie<tr>>> next_operands;
				std::tie(next_operands, current_key_part) = *left_join_iterator;
				// initialize and load sub_operator
                init_load_sub_operator(&next_operands);
				find_next_valid();
			} else {
				this->ended_ = true;
			}
		}

		void init_load_sub_operator(std::vector<std::optional<const_Hypertrie<tr>>>* join_returned_operands) {
			// the subscript for the sub_operator
            std::shared_ptr<Subscript> sub_op_subscript;
			// the labels of the operands of the sub_op_subscript
            std::vector<std::vector<Label>> sub_op_ops_labels{};
			// the positions of the operands that take part in the left join
			auto poss_operands_with_label = this->subscript->getPossOfOperandsWithLabel(label);
			// the number of operands that do not take part in the left join
			auto operands_without_label = this->subscript->getRawSubscript().operands.size() - poss_operands_with_label.size();
			// the operands of the next operator
			std::vector<const_Hypertrie<tr>> next_operands{};
            std::set<Label> not_joined_dependent_labels{};
            for(const auto[op_pos, join_ret_op] : iter::enumerate(*join_returned_operands)) {
				if(join_ret_op.has_value()) {
					next_operands.push_back(join_ret_op.value());
					sub_op_ops_labels.push_back(next_subscript->getRawSubscript().operands[op_pos]);
				}
				else {
                    not_joined_dependent_labels.insert(next_subscript->getRawSubscript().operands[op_pos].begin(),
													   next_subscript->getRawSubscript().operands[op_pos].end());
				}
			}
			/* all operands were joined or no operands were returned */
			if (next_operands.size() == join_returned_operands->size()) {
                sub_op_subscript = next_subscript;
			}
			/* no operands were joined */
			else if (next_operands.size() == operands_without_label) {
                sub_op_ops_labels.clear();
				next_operands.clear();
				sub_op_subscript = std::make_shared<Subscript>(sub_op_ops_labels, next_subscript->getRawSubscript().result);
			}
			/* partial join */
			else {
				remove_dependent_operands(not_joined_dependent_labels, &sub_op_ops_labels, &next_operands);
                sub_op_subscript = std::make_shared<Subscript>(sub_op_ops_labels, next_subscript->getRawSubscript().result);
			}
            sub_operator = Operator_t::construct(sub_op_subscript, this->context);
            sub_operator->load(std::move(next_operands), *this->entry);
		}

		// removes all operands and their labels which transitively depend on operands that were not joined
		// example: a,ab,ac,bd,de->abcde, if ab was not joined then: a,ac->abcde
        void remove_dependent_operands(const std::set<Label>& dependent_labels,
									   std::vector<std::vector<Label>>* ops_labels,
                                       std::vector<const_Hypertrie<tr>>* next_operands) {

			std::set<Label> to_be_checked{};
			for(auto dep_label : dependent_labels) {
                for(auto [op_pos, op_labels] : iter::enumerate(*ops_labels))
					if(std::find(op_labels.begin(), op_labels.end(), dep_label) != op_labels.end()) {
						to_be_checked.insert(op_labels.begin(), op_labels.end());
						ops_labels->erase(ops_labels->begin() + op_pos);
						next_operands->erase(next_operands->begin() + op_pos);
					}
			}
			while(to_be_checked.size()) {
				auto current_check = to_be_checked;
				to_be_checked.clear();
                for(auto dep_label : current_check) {
					for (auto [op_pos, op_labels] : iter::enumerate(*ops_labels)) {
						if (std::find(op_labels.begin(), op_labels.end(), dep_label) != op_labels.end()) {
							to_be_checked.insert(op_labels.begin(), op_labels.end());
							ops_labels->erase(ops_labels->begin() + op_pos);
                            next_operands->erase(next_operands->begin() + op_pos);
						}
					}
				}
            }
		}
	};
}

#endif//HYPERTRIE_LEFTJOINOPERATOR_HPP
