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
        std::shared_ptr<Subscript> wwd_subscript; // the subscript of the optional part in case of wwd pattern
        std::shared_ptr<Subscript> next_subscript; // the subscript that will be passed to the sub_operator
        std::shared_ptr<Subscript> node_subscript; // the subscript of the non-optional part
        std::vector<const_Hypertrie<tr>> node_operands{}; // the operands of the non-optional part
        std::vector<const_Hypertrie<tr>> opt_operands{}; // the operands of the optional part before slicing
		std::unique_ptr<Entry_t> sub_entry;
		std::map<Label, LabelPossInOperands> slicing_positions{};
		boost::container::flat_set<Label> wwd_labels{};

	public:

        RecursiveLeftJoinOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context<key_part_type>> &context)
			: Operator_t(Subscript::Type::RecursiveLeftJoin, subscript, context, this) {
			ended_ = true;
			// split the subscript into non-optional and optional subscripts
			extractSubscripts(subscript);
			// reserve the size of the operands' vectors
            node_operands.resize(subscript->getNonOptionalOperands().size());
            opt_operands.resize(subscript->getRawSubscript().operands.size() - subscript->getNonOptionalOperands().size());
			// for count operator to carry out join -- count does not keep track of the values of the labels
			if (node_subscript->type == Subscript::Type::Count)
				node_subscript->type = Subscript::Type::Join;
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
				if(not node_operator->ended()) {
					next_subscript = opt_subscript;
					next_operands = slice_operands();
				}
				else {
					this->entry->value = 1; // node operator has no result but we are going to return an entry
					next_subscript = wwd_subscript;
					next_operands = slice_operands(true);
				}
                if (not sub_operator or sub_operator->hash() != next_subscript->hash()) {
                    sub_operator = Operator_t::construct(next_subscript, this->context);
                    sub_entry = std::make_unique<Entry_t>(next_subscript->resultLabelCount(), Operator_t::default_key_part);
                }
                else
                    sub_entry->clear(default_key_part);
                sub_operator->load(std::move(next_operands), *sub_entry);
				updateNodeEntry();
			}
		}

		// use the mapped labels to slice the next operands
		std::vector<const_Hypertrie<tr>> slice_operands(bool wwd = false) {
            std::vector<const_Hypertrie<tr>> sliced_operands{};
			std::vector<hypertrie::SliceKey<key_part_type>> slice_keys{};
			// prepare slice keys
			for (auto opt_operand : opt_operands)
				slice_keys.emplace_back(hypertrie::SliceKey<key_part_type>(opt_operand.depth(), std::nullopt));
			// iterate over the mapping
			for (const auto &[label, slice_poss] : slicing_positions) {
				if (wwd and wwd_labels.contains(label))
					continue;
				for (auto op_pos : iter::range(slice_poss.size()))
					for (auto slice_pos : slice_poss[op_pos])
						slice_keys[op_pos][slice_pos] = this->context->mapping[label];
			}
			// iterate over slice keys and slice operands
			for (const auto &[pos, s_key] : iter::enumerate(slice_keys)) {
				bool slice = false;
				for (auto &s_key_part : s_key) {
					if (s_key_part != std::nullopt) {
						slice = true;
						break;
					}
				}
				if (slice) {
                    auto sliced_operand = opt_operands[pos][s_key];
                    if (std::holds_alternative<const_Hypertrie<tr>>(sliced_operand))
                        sliced_operands.push_back(std::move(std::get<0>(sliced_operand)));
                }
				else
					sliced_operands.push_back(opt_operands[pos]);
			}
			return sliced_operands;
        }

		// it prepares the subscripts for the node_operator and sub_operator -- called by constructor
		// it also stores the slicing positions
		void extractSubscripts(const std::shared_ptr<Subscript>& subscript) {
			// create node_subscript
			OperandsSc node_operands_labels{};
			for (auto non_opt_pos : subscript->getNonOptionalOperands())
                node_operands_labels.push_back(subscript->getRawSubscript().operands[non_opt_pos]);
			node_subscript = std::make_shared<Subscript>(node_operands_labels, subscript->getRawSubscript().result);
			// prepare the mapping by adding the labels of the node operator
			for (auto node_label : node_subscript->getOperandsLabelSet())
				this->context->mapping[node_label];
			// create a temp subscript that does not have the node operands
            auto pre_slicing_subscript = subscript->removeOperands(subscript->getNonOptionalOperands());
			// store the labels that appear in the mapping
            tsl::hopscotch_set<Label> mapping_labels{};
            for (auto &mapped_label : this->context->mapping)
				mapping_labels.insert(mapped_label.first);
            auto slice_poss = pre_slicing_subscript->getNonOptionalOperands();
            // store slicing positions
            for (auto mapped_label : mapping_labels) {
                if (not pre_slicing_subscript->getOperandsLabelSet().contains(mapped_label))
                    continue;
                LabelPossInOperands label_slice_poss{};
                for (auto slice_pos : slice_poss) {
                    LabelPossInOperand op_slice_poss{};
                    for (const auto &[l_pos, label] : iter::enumerate(pre_slicing_subscript->getRawSubscript().operands[slice_pos]))
                        if (label == mapped_label)
                            op_slice_poss.push_back(l_pos);
                    label_slice_poss.push_back(std::move(op_slice_poss));
                }
                slicing_positions[mapped_label] = label_slice_poss;
            }
			// create opt_subscript by removing all non-opt labels of the original subscript
			opt_subscript = pre_slicing_subscript->slice(mapping_labels);
            // create wwd subscript in case there are wwd labels
            if (not subscript->getWWDLabels().empty()) {
                for (auto wwd_label : subscript->getWWDLabels()) {
                    wwd_labels.insert(wwd_label);
                    mapping_labels.erase(wwd_label);
                }
                wwd_subscript = pre_slicing_subscript->slice(mapping_labels);
            }
		}

		// splits the input operands to non-optional and optional
		void extractOperands(std::vector<const_Hypertrie<tr>>& operands) {
			std::uint8_t non_opt_pos = 0;
            std::uint8_t opt_pos = 0;
            for (const auto &[op_pos, operand] : iter::enumerate(operands)) {
                if (std::find(this->subscript->getNonOptionalOperands().begin(),
							  this->subscript->getNonOptionalOperands().end(),
							  op_pos)  != this->subscript->getNonOptionalOperands().end()) {
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
			if(sub_entry->value > 0)
				this->entry->value *= sub_entry->value;
            for(auto &sub_res_label : next_subscript->getResultLabelSet())
                this->entry->key[node_subscript->getLabelPosInResult(sub_res_label)] =
                        sub_entry->key[next_subscript->getLabelPosInResult(sub_res_label)];
		}

	};
}

#endif//HYPERTRIE_RECURSIVELEFTJOINOPERATOR_HPP
