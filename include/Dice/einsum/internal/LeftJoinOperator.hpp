#ifndef HYPERTRIE_LEFTJOINOPERATOR_HPP
#define HYPERTRIE_LEFTJOINOPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"
#include "Dice/einsum/internal/EntryGeneratorOperator.hpp"
#include <Dice/hypertrie/internal/HashJoin.hpp>
#include <Dice/hypertrie/internal/LeftHashJoin.hpp>

namespace einsum::internal {

	template<typename value_type, HypertrieTrait tr_t>
	class LeftJoinOperator : public Operator<value_type, tr_t> {
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"
		using LeftJoin_t = hypertrie::LeftHashJoin<tr>;
        using EntryGeneratorOperator_t = EntryGeneratorOperator<value_type, tr>;

		LeftJoin_t left_join;
		typename LeftJoin_t::iterator left_join_iterator;
		// the active label
        Label label = std::numeric_limits<Label>::max();// TODO: type the unusable value (max) explicitly
		// true: the active label appears in the result
        bool is_result_label = false;
        // the position of the active label in each operand
		LabelPossInOperands label_poss_in_ops;
		// the non-optional positions
		std::vector<OperandPos> non_optional_poss;
        // the position of the active label in the result
		LabelPos label_pos_in_result;
        // the positions of the operands of the original subscript in the next_subscript
        // it is used in order to pass the proper operands to the sub_operator
        std::vector<hypertrie::pos_type> pos_in_out;
		// keeps track of the operands that were joined
		// it is used in order to handle dependencies
        std::vector<std::size_t> joined;
		// the part of the key that was selected in the join
		key_part_type current_key_part;

		bool generate_optional_value;

		std::shared_ptr<Operator_t> sub_operator;
        std::shared_ptr<Subscript> next_subscript;

		// it is used to generate an entry
		// left join always generates an entry
		std::unique_ptr<Operator_t> entry_generator; // initialized in load_impl;

		bool ended_ = true;

	public:
		LeftJoinOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context> &context)
			: Operator_t(subscript->type, subscript, context, this) {
            auto entry_gen_sc = std::make_shared<Subscript>(std::vector<std::vector<Label>>(),
                                                            subscript->getRawSubscript().result);
            entry_generator = std::make_unique<EntryGeneratorOperator_t>(entry_gen_sc, this->context);
		}

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
			if(!self.sub_operator->ended())
			    self.sub_operator->next();
			self.find_next_valid();
		}

	private:

		void find_next_valid() {
			assert(left_join_iterator);
			while(sub_operator->ended() && !generate_optional_value) {
                ++left_join_iterator;
                if (left_join_iterator and not this->context->hasTimedOut()) {
					// clear entry: since in each iteration we initiate a new sub_operator, some keys of the entry will stay the same
					// use EntryGeneratorOperator to clear the entry
					entry_generator->load({}, *this->entry);
                    std::vector<std::optional<const_Hypertrie<tr>>> next_operands;
                    std::tie(next_operands, current_key_part, pos_in_out, joined) = *left_join_iterator;
                    // check if we can generate an optional entry for this operand
                    generate_optional_value = check_optional_generation();
                    init_load_sub_operator(&next_operands);
                } else {
                    ended_ = true;
                    return;
                }
			}
            if (is_result_label)
                this->entry->key[label_pos_in_result] = current_key_part;
			// this operand already has a result -> we do not need to generate an optional value anymore
			generate_optional_value = false;

			if constexpr (_debugeinsum_)
				fmt::print("[{}]->{} {}\n", fmt::join(this->entry->key, ","), this->entry->value, this->subscript);
		}

		inline void load_impl(std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			if constexpr (_debugeinsum_) fmt::print("LeftJoin {}\n", this->subscript);

			this->entry = &entry;
			ended_ = false;
			Label last_label = label;
			// check if a label for this operator has already been chosen by the JoinSelectionOperator
			if(this->context->sub_operator_label.contains(hash(this)))
				label = this->context->sub_operator_label[hash(this)];
			else
			    label = *(this->subscript->getLeftJoinLabels().begin()); // TODO: choose optimal label
			if (label != last_label) {
				label_poss_in_ops = this->subscript->getLabelPossInOperands(label);
				is_result_label = this->subscript->isResultLabel(label);
				non_optional_poss = this->subscript->getNonOptionalOperands(label);
				if (is_result_label)
					label_pos_in_result = this->subscript->getLabelPosInResult(label);
			}
			// prepare next_subscript by removing the active label from the current subscript
			next_subscript = this->subscript->removeLabel(label);
            // prepare left join
			left_join = LeftJoin_t{operands, label_poss_in_ops, non_optional_poss};
			left_join_iterator = left_join.begin();
			// check if left join has entries
			if (left_join_iterator) {
				std::vector<std::optional<const_Hypertrie<tr>>> right_operands;
				std::tie(right_operands, current_key_part, pos_in_out, joined) = *left_join_iterator;
				// check if we can generate an optional entry for this operand
				generate_optional_value = check_optional_generation();
				// initialize and load sub_operator
				init_load_sub_operator(&right_operands);
                find_next_valid();
			} else {
				this->ended_ = true;
			}
		}

        void init_load_sub_operator(std::vector<std::optional<const_Hypertrie<tr>>>* join_returned_operands) {
			// the labels to be removed
			std::set<Label> labels_to_remove{};
			// the labels of the operands to be considered
			auto original_operands_labels = next_subscript->getRawSubscript().operands;
			// the next operands to be passed to the sub_operator
			std::vector<const_Hypertrie<tr>> next_operands{};
            // the positions of the operands, which will be passed to the next operator, in the current operator
            std::vector<std::size_t> next_operands_poss{};
            // used to find dependencies between sub_operators
            std::vector<hypertrie::pos_type> pos_in_sub_op(pos_in_out.size(), std::numeric_limits<hypertrie::pos_type>::max());
			// stores the positions of the operands that will be removed
            std::set<std::size_t> operands_to_remove{};
			// iterate over all operands
            for(auto op_pos : iter::range(pos_in_out.size())) {
				// if an operand does not yield a result it should be removed along with its dependent operands
				if(joined[op_pos])
                    continue;
                operands_to_remove.insert(op_pos);
                labels_to_remove.insert(original_operands_labels[pos_in_out[op_pos]].begin(),
                                        original_operands_labels[pos_in_out[op_pos]].end());
				for(auto op_dependent_pos : this->subscript->getDependentOperands(op_pos)) {
                    operands_to_remove.insert(op_dependent_pos);
                    labels_to_remove.insert(original_operands_labels[pos_in_out[op_dependent_pos]].begin(),
                                            original_operands_labels[pos_in_out[op_dependent_pos]].end());
				}
			}
			// populate next_operands
			uint8_t pos = 0;
			for(auto op_pos : iter::range(pos_in_out.size())) {
				if(operands_to_remove.find(op_pos) == operands_to_remove.end()
					and pos_in_out[op_pos] < std::numeric_limits<hypertrie::pos_type>::max()) {
						next_operands.push_back((*join_returned_operands)[pos_in_out[op_pos]].value());
						pos_in_sub_op[op_pos] = pos++;
			    }
			}
            auto sub_op_subscript = next_subscript;
			for (auto label_to_remove : labels_to_remove) {
                sub_op_subscript = sub_op_subscript->removeLabel(label_to_remove);
            }
            // compute the dependencies between the sub_operators (only for Cartesian)
            if(sub_op_subscript->type == Subscript::Type::Cartesian) {
                this->context->sub_operator_dependency_map[sub_op_subscript->hash()];
                auto ops_sub_operator = sub_op_subscript->getSubOperatorOfOperands();
                for(auto next_op_pos : iter::range(pos_in_sub_op.size())) {
					if(pos_in_sub_op[next_op_pos] >= std::numeric_limits<hypertrie::pos_type>::max())
						continue;
                    std::vector<std::size_t> dependent_operands_poss{};
                    auto operand_sub_op = ops_sub_operator[pos_in_sub_op[next_op_pos]];
                    for(auto dependent_operand_pos : this->subscript->getDependentOperands(next_op_pos)) {
                        auto dependent_operand_sub_op = ops_sub_operator[pos_in_sub_op[dependent_operand_pos]];
                        if(operand_sub_op == dependent_operand_sub_op)
                            continue;
                        auto& sub_op_map = this->context->sub_operator_dependency_map[sub_op_subscript->hash()][operand_sub_op];
                        sub_op_map.insert(ops_sub_operator[pos_in_sub_op[dependent_operand_pos]]);
                    }
                }
            }
            sub_operator = Operator_t::construct(sub_op_subscript, this->context);
            sub_operator->load(std::move(next_operands), *this->entry);
		}

		bool check_optional_generation() {
            bool optional = true;
            for(auto non_optional_pos : non_optional_poss) {
                if(pos_in_out[non_optional_pos] < std::numeric_limits<hypertrie::pos_type>::max()) {
                    optional = false;
                    break;
                }
            }
			return optional;
		}

	};
}

#endif//HYPERTRIE_LEFTJOINOPERATOR_HPP
