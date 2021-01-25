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
		std::shared_ptr<Subscript> opt_subscript; // the subscript of the optional part
        std::shared_ptr<Subscript> sliced_subscript; // the subscript of the optional part after slicing
        std::shared_ptr<Subscript> wwd_subscript; // the subscript of the optional part in case of wwd pattern
        std::shared_ptr<Subscript> next_subscript; // the subscript that will be passed to the sub_operator
        std::shared_ptr<Subscript> node_subscript; // the subscript of the non-optional part
        std::vector<const_Hypertrie<tr>> node_operands{}; // the operands of the non-optional part
        std::vector<const_Hypertrie<tr>> opt_operands{}; // the operands of the optional part before slicing
 		std::vector<OperandPos> non_opt_ops_poss; // the positions of non-optional operands
		std::unique_ptr<Entry_t> sub_entry;
		std::map<Label, LabelPossInOperands> slicing_positions{};
		boost::container::flat_set<Label> wwd_labels{};

	public:
        RecursiveLeftJoinOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context<key_part_type>> &context)
			: Operator_t(Subscript::Type::LeftJoin, subscript, context, this),
			  non_opt_ops_poss(subscript->getNonOptionalOperands()){
			ended_ = true;
			// split the subscript into non-optional and optional subscripts
			extractSubscripts(subscript);
			// reserve the size of the operands' vectors
            node_operands.resize(non_opt_ops_poss.size());
            opt_operands.resize(subscript->getRawSubscript().operands.size() - non_opt_ops_poss.size());
			// construct node operator
            node_operator = Operator_t::construct(node_subscript, this->context);
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
            self.find_next_valid();
            self.updateNodeEntry();
        }

	private:
        inline void clear_impl(){
            if(this->node_operator)
                this->node_operator->clear();
			if(this->sub_operator)
				this->sub_operator->clear();
        }

		inline void find_next_valid() {
            if(sub_operator->ended()) {
                if(!node_operator->ended()) {
					node_operator->next();
					if(!node_operator->ended()) {
						sub_entry->clear(default_key_part);
						sub_operator->load(std::move(slice_operands()), *sub_entry);
					}
                    else {
                        ended_ = true;
                        return;
                    }
				}
                else {
                    ended_ = true;
                    return;
                }
            }
		}

        inline void load_impl(std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			if constexpr (_debugeinsum_) fmt::print("RecursiveLeftJoin {}\n", this->subscript);
			this->entry = &entry;
            // split non-optional and optional operands
            extractOperands(operands);
			// load node_operator
			node_operator->load(node_operands, *this->entry);
			if(node_operator->ended() and wwd_labels.empty())
				return;
			else {
				ended_ = false;
				std::vector<const_Hypertrie<tr>> next_operands;
                if(not sliced_subscript)
                    sliceSubscript();
				if(not node_operator->ended()) {
					next_subscript = sliced_subscript;
					next_operands = slice_operands();
				}
				else {
					next_subscript = wwd_subscript;
					next_operands = slice_operands(true);
				}
                if (not sub_operator or sub_operator->hash() != next_subscript->hash()) {
                    sub_operator = Operator_t::construct(next_subscript, this->context);
                    sub_entry = std::make_unique<Entry_t>(next_subscript->resultLabelCount(), Operator_t::default_key_part);
                }
                sub_operator->load(std::move(next_operands), *sub_entry);
				updateNodeEntry();
			}
		}

		// use the mapped labels to slice the next operands
		std::vector<const_Hypertrie<tr>> slice_operands(bool wwd = false) {
            std::vector<const_Hypertrie<tr>> sliced_operands{};
			auto opt_non_opt_poss = opt_subscript->getNonOptionalOperands();
			for(const auto &[pos, operand] : iter::enumerate(opt_operands)) {
				if(std::find(opt_non_opt_poss.begin(), opt_non_opt_poss.end(), pos) == opt_non_opt_poss.end()) {
					sliced_operands.emplace_back(operand);
					continue;
				}
				hypertrie::SliceKey<key_part_type> s_key(operand.depth(), std::nullopt);
				for(const auto &[label, slice_poss] : slicing_positions) {
					if(wwd and wwd_labels.find(label) != wwd_labels.end())
						continue;
					if(slice_poss[pos].empty())
						continue;
					for(auto &slice_pos : slice_poss[pos])
						s_key[slice_pos] = this->context->mapping[label];
				}
				auto sliced_operand = operand[s_key];
				if(std::holds_alternative<const_Hypertrie<tr>>(sliced_operand))
					sliced_operands.push_back(std::move(std::get<0>(sliced_operand)));
			}
			return sliced_operands;
        }

		void sliceSubscript() {
            auto opt_non_opt_poss = opt_subscript->getNonOptionalOperands();
            // find the slicing positions of the labels that have already been resolved
			tsl::hopscotch_set<Label> sliced_labels{};
            for(const auto &[label, _] : this->context->mapping) {
                sliced_labels.insert(label);
				if(opt_subscript->getOperandsLabelSet().find(label) == opt_subscript->getOperandsLabelSet().end())
					continue;
                slicing_positions[label] = opt_subscript->getLabelPossInOperands(label);
            }
            // ensure that only non-optional operands will be sliced
            for(auto &[_, slicing_poss] : slicing_positions) {
                for(auto op_pos : iter::range(slicing_poss.size()))
                    if(std::find(opt_non_opt_poss.begin(), opt_non_opt_poss.end(), op_pos) == opt_non_opt_poss.end())
                        slicing_poss.erase(slicing_poss.begin()+op_pos);
            }
            // create sliced_subscript by removing the sliced labels from the non-opt positions of opt_subscript
            sliced_subscript = opt_subscript->removeLabels(sliced_labels);
			// create wwd subscript by removing the labels that do not participate in wwd pattern
			for(auto label : wwd_labels)
				sliced_labels.erase(label);
			wwd_subscript = opt_subscript->removeLabels(sliced_labels);
		}

		// it prepares the subscripts of the node_operator and sub_operator -- called by constructor
		void extractSubscripts(const std::shared_ptr<Subscript>& subscript) {
            const auto& operands_labels = subscript->getRawSubscript().operands;
            // prepare the subscript of the node operator
            std::vector<std::vector<Label>> node_operands_labels{};
            tsl::hopscotch_set<Label> non_opt_labels{};
			// the labels of the node_subscript are found in non_optional_positions of the current subscript
            for(auto& non_opt_pos : non_opt_ops_poss) {
                node_operands_labels.emplace_back(operands_labels[non_opt_pos]);
				non_opt_labels.insert(node_operands_labels.rbegin()->begin(), node_operands_labels.rbegin()->end());
            }
			// add each non-optional label as a key to the active mapping - value is not set yet
			for(auto non_opt_label : non_opt_labels)
				this->context->mapping[non_opt_label];
			node_subscript = std::make_shared<Subscript>(node_operands_labels, subscript->getRawSubscript().result);
            // prepare the subscripts of the sub_operator
			opt_subscript = subscript->removeOperands(non_opt_ops_poss);
			for(auto label : non_opt_labels)
				if(subscript->getWWDLabels().find(label) != subscript->getWWDLabels().end())
					wwd_labels.insert(label);
		}

		// splits the input operands to non-optional and optional
		void extractOperands(std::vector<const_Hypertrie<tr>>& operands) {
			std::uint8_t non_opt_pos = 0;
            std::uint8_t opt_pos = 0;
            for(const auto &[op_pos, operand] : iter::enumerate(operands)) {
                if(std::find(non_opt_ops_poss.begin(), non_opt_ops_poss.end(), op_pos) != non_opt_ops_poss.end()) {
                    node_operands[non_opt_pos] = std::move(operand);
                    non_opt_pos++;
                }
                else {
                    opt_operands[opt_pos] = std::move(operand);
                    opt_pos++;
                }
            }
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
