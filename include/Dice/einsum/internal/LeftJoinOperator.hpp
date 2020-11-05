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
			label = this->subscript->getLeftJoinLabel();
			if (label != last_label) {
				label_poss_in_ops = this->subscript->getLabelPossInOperands(label);
				is_result_label = this->subscript->isResultLabel(label);
				if (is_result_label)
					label_pos_in_result = this->subscript->getLabelPosInResult(label);
			}
			// prepare next_subscript by removing the active label from the current subscript
			next_subscript = this->subscript->removeLabel(label, true);
            // prepare left join
			left_join = LeftJoin_t{operands, label_poss_in_ops};
			left_join_iterator = left_join.begin();
			// check if left join has entries
			if (left_join_iterator) {
				std::vector<std::optional<const_Hypertrie<tr>>> right_operands;
                // the positions of the operands of the original subscript in the next_subscript
                // it is used in order to handle dependencies
                std::vector<hypertrie::pos_type> pos_in_out;
				std::tie(right_operands, current_key_part, pos_in_out) = *left_join_iterator;
				// initialize and load sub_operator
				init_load_sub_operator(&right_operands, pos_in_out);
				find_next_valid();
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
			// the labels of the operands to be considered
			auto original_operands_labels = next_subscript->getRawSubscript().original_operands;
			// the result labels
			auto original_result_labels = next_subscript->getRawSubscript().result;
			// the operands labels to be passed to the subscript of the sub_operator
			decltype(original_operands_labels) next_operands_labels{};
			// the next operands to be passed to the sub_operator
			std::vector<const_Hypertrie<tr>> next_operands{};

			// if the left operand appears in the result save the operand and the label
			if(pos_in_out[0] < std::numeric_limits<hypertrie::pos_type>::max()) {
                next_operands_labels.push_back(original_operands_labels[0]);
                next_operands.push_back(join_returned_operands->at(pos_in_out[0]).value());
			}

			// iterate over the right operands
			for(auto right_operand : this->subscript->getDependentOperands(0)) {
				// if a right operand does not yield a result we do not have to consider it nor its dependents operands
				if(pos_in_out[right_operand] >= std::numeric_limits<hypertrie::pos_type>::max()
                    or !(*join_returned_operands)[pos_in_out[right_operand]].has_value())
					continue;
				// check if the current right operand participates in a join
				// the join operands
				auto scc_neighbors = this->subscript->getJoinDependentOperands(right_operand);
				// if the operand participates in a join find out if it returns a result
				if(scc_neighbors.size()) {
					bool break_join = false;
					decltype(original_operands_labels) join_operands_labels{original_operands_labels[pos_in_out[right_operand]]};
					decltype(next_operands) join_next_operands{join_returned_operands->at(pos_in_out[right_operand]).value()};
					for(auto neighbor : scc_neighbors) {
						// one of the join operands does not have value -> do not consider current operand
						if(!join_returned_operands->at(pos_in_out[neighbor]).has_value()) {
							break_join = true;
							break;
						}
						join_operands_labels.push_back(original_operands_labels[pos_in_out[neighbor]]);
                        join_next_operands.push_back(join_returned_operands->at(pos_in_out[neighbor]).value());
					}
					if(break_join)
						continue;
					auto join_subscript = std::make_shared<Subscript>(join_operands_labels, original_result_labels);
					auto join_operator = Operator_t::construct(join_subscript, this->context);
					join_operator->load(std::move(join_next_operands), *this->entry);
					if(join_operator->ended())
						continue;
 				}
                next_operands.push_back(join_returned_operands->at(pos_in_out[right_operand]).value());
                next_operands_labels.push_back(original_operands_labels[pos_in_out[right_operand]]);
				// iterate over all (transitively) dependent operands of the current right operand
				// check if they participate in the result and if the have returned an operand
				std::set<std::size_t> visited{right_operand};
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
						or !(*join_returned_operands)[pos_in_out[right_operand_dependent]].has_value())
						continue;
                    next_operands.push_back(join_returned_operands->at(pos_in_out[right_operand_dependent]).value());
                    next_operands_labels.push_back(original_operands_labels[pos_in_out[right_operand_dependent]]);
					// check the operands that depend on the current operand as well
                    for(auto nested_dependent : this->subscript->getDependentOperands(right_operand_dependent))
						check.push_back(nested_dependent);
				}
			}
            auto sub_op_subscript = std::make_shared<Subscript>(next_operands_labels, original_result_labels);
            sub_operator = Operator_t::construct(sub_op_subscript, this->context);
            sub_operator->load(std::move(next_operands), *this->entry);
		}

	};
}

#endif//HYPERTRIE_LEFTJOINOPERATOR_HPP
