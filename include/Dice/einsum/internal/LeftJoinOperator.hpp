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
		// the part of the key that was selected in the join
		key_part_type current_key_part;

		std::shared_ptr<Operator_t> sub_operator;
        std::shared_ptr<Subscript> next_subscript;

		// it is used to generate an entry
		// left join always generates an entry
		std::unique_ptr<Operator_t> entry_generator; // initialized in load_impl;

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
			if(!self.sub_operator->ended())
			    self.sub_operator->next();
			if(self.sub_operator->ended()) {
				++self.left_join_iterator;
				if(!self.left_join_iterator) {
					self.ended_ = true;
				}
				else {
                    // generate new entry for the current result
                    self.entry_generator->load({}, *self.entry);
                    std::vector<std::optional<const_Hypertrie<tr>>> next_operands;
                    // the positions of the operands of the original subscript in the next_subscript
                    // it is used in order to handle dependencies
                    std::vector<hypertrie::pos_type> pos_in_out;
                    std::tie(next_operands, self.current_key_part, pos_in_out) = *(self.left_join_iterator);
                    self.init_load_sub_operator(&next_operands, pos_in_out);
				}
			}
            if (self.is_result_label)
                self.entry->key[self.label_pos_in_result] = self.current_key_part;
		}

	private:

		void find_next_valid() {
			assert(left_join_iterator);
			while (sub_operator->ended()) {
				++left_join_iterator;
				if (left_join_iterator and not this->context->hasTimedOut()) {
					std::vector<std::optional<const_Hypertrie<tr>>> next_operands;
                    // the positions of the operands of the original subscript in the next_subscript
                    // it is used in order to handle dependencies
                    std::vector<hypertrie::pos_type> pos_in_out;
					std::tie(next_operands, current_key_part, pos_in_out) = *left_join_iterator;
                    init_load_sub_operator(&next_operands, pos_in_out);
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
			label = this->subscript->getLeftJoinLabels().front(); // TODO: choose optimal label
			if (label != last_label) {
				label_poss_in_ops = this->subscript->getLabelPossInOperands(label);
				is_result_label = this->subscript->isResultLabel(label);
				non_optional_poss = this->subscript->getNonOptionalOperands(label);
				if (is_result_label)
					label_pos_in_result = this->subscript->getLabelPosInResult(label);
			}
			// initialize entry_generator
			auto entry_gen_sc = std::make_shared<Subscript>(std::vector<std::vector<Label>>(),
															this->subscript->getRawSubscript().result);
            entry_generator = std::make_unique<EntryGeneratorOperator_t>(entry_gen_sc, this->context);
			// prepare next_subscript by removing the active label from the current subscript
			next_subscript = this->subscript->removeLabel(label);
            // prepare left join
			left_join = LeftJoin_t{operands, label_poss_in_ops, non_optional_poss};
			left_join_iterator = left_join.begin();
			// check if left join has entries
			if (left_join_iterator) {
				// generate new entry for the current result
				entry_generator->load({}, *this->entry);
				std::vector<std::optional<const_Hypertrie<tr>>> right_operands;
                // the positions of the operands of the original subscript in the next_subscript
                // it is used in order to handle dependencies
                std::vector<hypertrie::pos_type> pos_in_out;
				std::tie(right_operands, current_key_part, pos_in_out) = *left_join_iterator;
				// initialize and load sub_operator
				init_load_sub_operator(&right_operands, pos_in_out);
                if (is_result_label)
                    this->entry->key[label_pos_in_result] = current_key_part;
			} else {
				this->ended_ = true;
			}
		}

        /*
        * yx,xa,a,[ab,be,ed,[ac]]->abc [level 1, bound_vars: ]
        *
        *
        * a,a,[ab,be,ed,[ac]]->abc [level 1, bound_vars: ]
        * join (a,a) -> ab,be,ed,[ac] [level 2, bound_vars: [a]]
        * match (a) -> b,be,ed,[ac] [level 2, bound_vars: [a]]
        * what match does: go on only if a is equal to what was selected in level < current level
        *
        * result_keys:
        * level 1: [a]
        * level 2: [a,b,e,d]
        * level 3: [a,c]
        */

        void init_load_sub_operator(std::vector<std::optional<const_Hypertrie<tr>>>* join_returned_operands,
									const std::vector<hypertrie::pos_type>& pos_in_out) {
			// the labels to be removed
			std::set<Label> labels_to_remove{};
			// the labels of the operands to be considered
			auto original_operands_labels = next_subscript->getRawSubscript().operands;
			// the next operands to be passed to the sub_operator
			std::vector<const_Hypertrie<tr>> next_operands{};
			// the position of the operands in the sub_operator
			// used to find dependencies between sub_operators
            std::vector<hypertrie::pos_type> pos_in_sub_op(pos_in_out.size(),
														   std::numeric_limits<hypertrie::pos_type>::max());
            // the positions of the operands, which will be passed to the next operator, in the current operator
            std::vector<std::size_t> next_operands_poss{};
			// if the left operand appears in the result save the operand and the label
			if(pos_in_out[0] < std::numeric_limits<hypertrie::pos_type>::max())
				next_operands_poss.push_back(0);
			// iterate over the right operands
			for(auto right_operand : this->subscript->getDependentOperands(0)) {
				// if a right operand does not yield a result we do not have to consider it nor its dependent operands
				if(pos_in_out[right_operand] >= std::numeric_limits<hypertrie::pos_type>::max()
                    or !(*join_returned_operands)[pos_in_out[right_operand]].has_value())
                    continue;
                next_operands_poss.push_back(right_operand);
				// iterate over all (transitively) dependent operands of the current right operand
				// check if they participate in the result and if they have returned an operand
				std::set<std::size_t> visited{0, right_operand};
				std::deque<std::size_t> check{};
				for(auto right_operand_dependent : this->subscript->getDependentOperands(right_operand))
					check.push_back(right_operand_dependent);
                while(!check.empty()) {
					auto right_operand_dependent = check.front();
					check.pop_front();
                    if(visited.find(right_operand_dependent) != visited.end())
                        continue;
                    visited.insert(right_operand_dependent);
					if(pos_in_out[right_operand_dependent] >= std::numeric_limits<hypertrie::pos_type>::max()
						or !(*join_returned_operands)[pos_in_out[right_operand_dependent]].has_value()) {
						// remove join dependent operands that were previously not removed
						for(auto nested_dependent : this->subscript->getDependentOperands(right_operand_dependent)) {
							if(auto nested_dep_pos = std::find(next_operands_poss.begin(), next_operands_poss.end(), nested_dependent);
                                nested_dep_pos != next_operands_poss.end())
                                next_operands_poss.erase(nested_dep_pos);
						}
                        continue;
					}
                    next_operands_poss.push_back(right_operand_dependent);
					// check the operands that depend on the current operand as well
                    for(auto nested_dependent : this->subscript->getDependentOperands(right_operand_dependent))
						check.push_back(nested_dependent);
				}
			}
			// populate next_operands and find the labels to be removed
			uint8_t pos = 0;
			for(auto i : iter::range(pos_in_out.size())) {
				if(pos_in_out[i] < std::numeric_limits<hypertrie::pos_type>::max()) {
                    if(std::find(next_operands_poss.begin(), next_operands_poss.end(), i) == next_operands_poss.end())
					    labels_to_remove.insert(original_operands_labels[pos_in_out[i]].begin(),
                                                original_operands_labels[pos_in_out[i]].end());
					else {
						next_operands.push_back(join_returned_operands->at(pos_in_out[i]).value());
						pos_in_sub_op[i] = pos++;
					}
                }
			}
            auto sub_op_subscript = next_subscript;
			for (auto label_to_remove : labels_to_remove) {
                sub_op_subscript = sub_op_subscript->removeLabel(label_to_remove);
            }
            sub_operator = Operator_t::construct(sub_op_subscript, this->context);
            sub_operator->load(std::move(next_operands), *this->entry);
		}

	};
}

#endif//HYPERTRIE_LEFTJOINOPERATOR_HPP
