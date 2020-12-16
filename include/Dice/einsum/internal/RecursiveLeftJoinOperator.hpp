#ifndef HYPERTRIE_RECURSIVELEFTJOINOPERATOR_HPP
#define HYPERTRIE_RECURSIVELEFTJOINOPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"

namespace einsum::internal {

	template<typename value_type, HypertrieTrait tr_t>
	class RecursiveLeftJoinOperator : public Operator<value_type, tr_t> {
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"
		using RecursiveLeftJoinOperator_t = RecursiveLeftJoinOperator<value_type, tr>;

		bool ended_;

        std::shared_ptr<Operator_t> sub_operator; // the operator of the optional part
        std::shared_ptr<Operator_t> node_operator; // the operator of the non-optional part
		std::shared_ptr<Subscript> next_subscript; // the subscript of the optional part
        std::shared_ptr<Subscript> node_subscript; // the subscript of the non-optional part
		std::vector<const_Hypertrie<tr>> next_operands{}; // the operands of the the optional part before slicing
		std::vector<OperandPos> non_opt_ops_poss; // the positions of non-optional operands
        std::map<Label, LabelPossInOperands> label_poss_in_operands{};
		std::unique_ptr<Entry_t> sub_entry;

	public:
        RecursiveLeftJoinOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context> &context)
			: Operator_t(Subscript::Type::LeftJoin, subscript, context, this),
			  non_opt_ops_poss(subscript->getNonOptionalOperands()){
			ended_ = true;
			// prepare node and next subscripts
			extractSubscripts(subscript);
			// create new entry for the sub_operator
			sub_entry = std::make_unique<Entry_t>(next_subscript->resultLabelCount(), Operator_t::default_key_part);
			// construct operators
            node_operator = Operator_t::construct(node_subscript, this->context);
            sub_operator = Operator_t::construct(next_subscript, this->context);
		}

        static bool ended(const void *self_raw) {
            auto &self = *static_cast<const RecursiveLeftJoinOperator_t *>(self_raw);
            return self.ended_ or self.context->hasTimedOut();
        }

        static void load(void *self_raw, std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
            static_cast<RecursiveLeftJoinOperator *>(self_raw)->load_impl(std::move(operands), entry);
        }

        static void clear(void *self_raw) {
            return static_cast<RecursiveLeftJoinOperator_t *>(self_raw)->clear_impl();
        }

        static void next(void *self_raw) {
            RecursiveLeftJoinOperator_t &self = *static_cast<RecursiveLeftJoinOperator_t *>(self_raw);
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
                self.node_operator->next();
				if (self.node_operator->ended()) {
					self.ended_ = true;
					return;
				}
				else {
                    self.sub_entry->clear(default_key_part);
					self.sub_operator->load(std::move(self.slice_operands()), *self.sub_entry);
				}
			}
            self.updateNodeEntry();
        }

	private:
        inline void clear_impl(){
            if(this->node_operator)
                this->node_operator->clear();
			if(this->sub_operator)
				this->sub_operator->clear();
        }

        inline void load_impl(std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			if constexpr (_debugeinsum_) fmt::print("LeftJoin {}\n", this->subscript);

			this->entry = &entry;
			ended_ = false;
			// collect the operands of the node_operator
			std::vector<const_Hypertrie<tr>> node_operands{};
            for(auto& non_opt_pos : non_opt_ops_poss) {
                node_operands.emplace_back(std::move(operands[non_opt_pos]));
            }
			// load node_operator
			node_operator->load(std::move(node_operands), *this->entry);
			// if node_operator does not yield any results terminate
			// TODO: special case wwd patterns
			if(node_operator->ended()) {
				ended_ = true;
				return;
			}
			next_operands.clear(); // clear results of previous iteration
			// prepare the operands for the sub_operator
			for(const auto& [pos, operand] : iter::enumerate(operands)) {
				if(std::find(non_opt_ops_poss.begin(), non_opt_ops_poss.end(), pos) == non_opt_ops_poss.end())
					next_operands.push_back(operand);
			}
            sub_operator->load(std::move(slice_operands()), *sub_entry);
            updateNodeEntry();
		}

		// use the mapped labels to slice the next operands
		std::vector<const_Hypertrie<tr>> slice_operands() {
            std::vector<const_Hypertrie<tr>> sliced_operands = next_operands;
            key_part_type* mapped_value;
			// iterate over the result labels
			for(auto& node_label : node_subscript->getOperandsLabelSet()) {
				if(!label_poss_in_operands.contains(node_label))
					continue;
                if(this->entry->active_mapping.contains(node_label))
                    mapped_value = &this->entry->active_mapping[node_label];
				else
                    mapped_value = &this->entry->key[this->subscript->getLabelPosInResult(node_label)];
                const auto &poss_in_ops = label_poss_in_operands[node_label];
                for(const auto &[pos, next_operand] : iter::enumerate(next_operands)) {
                    if(poss_in_ops[pos].empty())
                        continue;
                    else {
                        // prepare the slice key for the operand
                        hypertrie::SliceKey<key_part_type> skey(next_operand.depth(), std::nullopt);
                        for (const auto &pos_in_op : poss_in_ops[pos])
                            skey[pos_in_op] = *mapped_value;
                        // slice the operand
                        auto sliced_operand = std::get<0>(next_operand[skey]);
                        sliced_operands[pos] = std::move(sliced_operand);
                    }
                }
			}
			return sliced_operands;
        }

		// called in constructor
		// it prepares the node and next subscripts
		void extractSubscripts(std::shared_ptr<Subscript> subscript) {
            const auto& operands_labels = subscript->getRawSubscript().operands;
            // prepare the subscript of the node operator
            std::vector<std::vector<Label>> node_operands_labels{};
			std::set<Label> non_opt_labels{};
            for(auto& non_opt_pos : subscript->getNonOptionalOperands()) {
                node_operands_labels.emplace_back(std::move(operands_labels[non_opt_pos]));
				non_opt_labels.insert(node_operands_labels.rbegin()->begin(), node_operands_labels.rbegin()->end());
            }
			node_subscript = std::make_shared<Subscript>(node_operands_labels, subscript->getRawSubscript().result);
            // prepare next_subscript
            next_subscript = subscript->removeOperands(non_opt_ops_poss);
			// store the slicing positions
            for(auto &non_opt_label : non_opt_labels)
				if(next_subscript->getOperandsLabelSet().find(non_opt_label) != next_subscript->getOperandsLabelSet().end())
					label_poss_in_operands[non_opt_label] = next_subscript->getLabelPossInOperands(non_opt_label);
            next_subscript = next_subscript->removeLabels(non_opt_labels);
		}

		// updates the entry of the node operator with the results of the sub_entry
		void updateNodeEntry() {
			for(auto &sub_res_label : next_subscript->getResultLabelSet())
				this->entry->key[node_subscript->getLabelPosInResult(sub_res_label)] =
										 sub_entry->key[next_subscript->getLabelPosInResult(sub_res_label)];
		}

	};
}

#endif//HYPERTRIE_RECURSIVELEFTJOINOPERATOR_HPP
