#ifndef HYPERTRIE_JOINSELECTIONOPERATOR_HPP
#define HYPERTRIE_JOINSELECTIONOPERATOR_HPP

#include "Dice/einsum/internal/JoinOperator.hpp"
#include "Dice/einsum/internal/LeftJoinOperator.hpp"
#include "Dice/einsum/internal/Operator.hpp"

namespace einsum::internal {

    template<typename value_type, HypertrieTrait tr_t>
    class JoinSelectionOperator : public Operator<value_type, tr_t> {
	private:
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"

        using LeftJoinOperator_t = LeftJoinOperator<value_type, tr>;
		using JoinOperator_t = JoinOperator<value_type, tr>;
        // the label to pass to the next operator
        Label label = std::numeric_limits<Label>::max();
		// the sub_operator will be either a LeftJoinOperator or a JoinOperator
        std::shared_ptr<Operator_t> sub_operator;
		// the next_subscript will be equal to the current subscript
        std::shared_ptr<Subscript> next_subscript;
        bool ended_ = true;

    public:

        JoinSelectionOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context> &context)
                : Operator_t(subscript->type, subscript, context, this) {
			ended_ = true;
		}

        static bool ended(const void *self_raw) {
            auto &self = *static_cast<const JoinSelectionOperator *>(self_raw);
            return self.ended_ or self.context->hasTimedOut();
        }

        static void load(void *self_raw, std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
            static_cast<JoinSelectionOperator *>(self_raw)->load_impl(std::move(operands), entry);
        }

        static std::size_t hash(const void *self_raw) {
            return static_cast<const JoinSelectionOperator *>(self_raw)->subscript->hash();
        }

        static void next(void *self_raw) {
			JoinSelectionOperator &self = *static_cast<JoinSelectionOperator *>(self_raw);
            self.sub_operator->next();
			if(self.sub_operator->ended()) {
				self.ended_ = true;
				return;
			}
		}

	private:

		// decides whether to load the JoinOperator or the LeftJoinOperator
        inline void load_impl(std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
            this->entry = &entry;
			// the subscript does not change
			next_subscript = this->subscript;
			// use CardinalityEstimation to select the best label
            label = CardinalityEstimation_t::getMinCardLabel(operands, this->subscript, this->context);
			// store the label in the context
			if(this->subscript->getLeftJoinLabels().find(label) == this->subscript->getLeftJoinLabels().end())
				next_subscript->type = Subscript::Type::Join;
			else
                next_subscript->type = Subscript::Type::LeftJoin;
			this->sub_operator = Operator_t::construct(next_subscript, this->context);
            this->context->sub_operator_label[sub_operator->hash()] = label;
			sub_operator->load(operands, *this->entry);
			ended_ = sub_operator->ended();
		}

	};

} // einsum::internal

#endif //HYPERTRIE_JOINSELECTIONOPERATOR_HPP
