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
        using LeftJoinOperator_t = LeftJoinOperator<value_type, tr>;

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

		robin_hood::unordered_map<std::vector<bool>, std::shared_ptr<Operator_t>> sub_operator_cache{};

		std::shared_ptr<Operator_t> sub_operator;
        std::shared_ptr<Subscript> next_subscript;

		bool ended_ = true;

	public:
		LeftJoinOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context<key_part_type>> &context)
			: Operator_t(Subscript::Type::LeftJoin, subscript, context, this) {
		}

		static bool ended(const void *self_raw) {
			auto &self = *static_cast<const LeftJoinOperator *>(self_raw);
			return self.ended_ or self.context->hasTimedOut();
		}

		static void load(void *self_raw, std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			static_cast<LeftJoinOperator *>(self_raw)->load_impl(std::move(operands), entry);
		}

        static void clear(void *self_raw) {
            return static_cast<LeftJoinOperator_t *>(self_raw)->clear_impl();
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
			// the operand might be already ended, in case of optional value generation
			if(!self.sub_operator->ended())
			    self.sub_operator->next();
			self.find_next_valid();
		}

	private:

        inline void clear_impl(){
            if (this->sub_operator)
                this->sub_operator->clear();
            this->left_join = {};
            this->left_join_iterator = {};
        }

        void find_next_valid() {
			while(sub_operator->ended() and !generate_optional_value and left_join_iterator) {
                ++left_join_iterator;
                if (left_join_iterator and not this->context->hasTimedOut()) {
					// clear entry: since in each iteration we use different sub_operators, some keys of the entry will stay the same
					this->entry->clear(default_key_part);
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
			Label last_label = label;
			// check if a label for this operator has already been chosen by the JoinSelectionOperator
			if(this->context->sub_operator_label.contains(this->subscript->hash()))
				label = this->context->sub_operator_label[this->subscript->hash()];
			else
			    label = *(this->subscript->getLeftJoinLabels().begin()); // TODO: choose optimal label
			if (label != last_label) {
				label_poss_in_ops = this->subscript->getLabelPossInOperands(label);
				is_result_label = this->subscript->isResultLabel(label);
				non_optional_poss = this->subscript->getNonOptionalOperands(label);
				if (is_result_label)
					label_pos_in_result = this->subscript->getLabelPosInResult(label);
                // prepare next_subscript by removing the active label from the current subscript
                next_subscript = this->subscript->removeLabel(label);
				// TODO: do we need this here
                for(auto &non_opt_pos : non_optional_poss)
                    if(operands[non_opt_pos].empty())
                        return;
			}
            ended_ = false;
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
            const auto& [opt_begin, opt_end] = next_subscript->getRawSubscript().optional_brackets;
			// the labels of the operands to be considered
			const auto& original_operands_labels = next_subscript->getRawSubscript().original_operands;
			const auto& poss_in_ops = next_subscript->getRawSubscript().poss_in_operands;
			// the next operands to be passed to the sub_operator
			std::vector<const_Hypertrie<tr>> next_operands{};
			// the labels of the next_operands
			std::vector<std::vector<Label>> next_operands_labels{};
			// bitstring for operands, 0: to be removed, 1: will be passed to sub_operator
			std::vector<bool> bitstring(join_returned_operands->size(), true);
			// iterate over all operands that participated in the join
            for(auto op_pos : iter::range(pos_in_out.size())) {
				if(joined[op_pos] || not bitstring[pos_in_out[op_pos]])
                    continue;
				// operands that do not yield a result need to be removed along with their dependent operands
				// store the position of the operands to be removed in the next_subscript
				if(pos_in_out[op_pos] < std::numeric_limits<hypertrie::pos_type>::max())
					bitstring[pos_in_out[op_pos]] = false;
				for(auto op_dependent_pos : this->subscript->getDependentOperands(op_pos)) {
                    if(pos_in_out[op_dependent_pos] < std::numeric_limits<hypertrie::pos_type>::max())
						bitstring[pos_in_out[op_dependent_pos]] = false;
				}
			}
			// populate next operands
			for(const auto &[op_pos, op] : iter::enumerate(*join_returned_operands))
				if(bitstring[op_pos])
                    next_operands.push_back(op.value());
			// check if the subscript has already been created; otherwise construct a new sub_operator
			if(sub_operator_cache.contains(bitstring))
				sub_operator = sub_operator_cache[bitstring];
			else {
				// populate next operands labels
				for (const auto &[orig_op_pos, op_labels] : iter::enumerate(original_operands_labels)) {
					if (op_labels == opt_begin || op_labels == opt_end) {
						next_operands_labels.push_back(op_labels);
						continue;
					}
					// get the position of the operand in the operands vector
					auto op_pos = poss_in_ops.at(orig_op_pos);
					// save successful operands and their labels
					if (bitstring[op_pos])
						next_operands_labels.push_back(op_labels);
				}
                auto sub_op_subscript = std::make_shared<Subscript>(next_operands_labels,
																	next_subscript->getRawSubscript().result);
                sub_operator = Operator_t::construct(sub_op_subscript, this->context);
				sub_operator_cache[bitstring] = sub_operator;
			}
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
