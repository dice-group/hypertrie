#ifndef HYPERTRIE_UNIONOPERATOR_HPP
#define HYPERTRIE_UNIONOPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"

namespace einsum::internal {

	template<typename value_type, HypertrieTrait tr_t>
	class UnionOperator : public Operator<value_type, tr_t> {
    #include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"

		// the sub_operators of the union operation
        std::vector<std::shared_ptr<Operator_t>> sub_operators; // set in construct
		// the entries of each sub_operator
        std::vector<Entry_t> sub_entries;
		// sub_operators vector iterators
        typename std::vector<std::shared_ptr<Operator_t>>::const_iterator sub_op_iter;
		// the provided operands
        std::vector<const_Hypertrie<tr>> operands;
        bool ended_ = true;

	public:

        UnionOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context<key_part_type>> &context)
                : Operator_t(Subscript::Type::Union, subscript, context, this) {
            // generate sub-operators
			// same to cartesian operator -- currently supports union between operators that do not share any labels
            const std::vector<std::shared_ptr<Subscript>> &sub_subscripts = this->subscript->getCartesianSubscript().getSubSubscripts();
            sub_operators.reserve(sub_subscripts.size());
            sub_entries.reserve(sub_subscripts.size());
            for (const auto &sub_subscript : sub_subscripts) {
                sub_operators.push_back(Operator_t::construct(sub_subscript, context));
                sub_entries.push_back(Entry_t(sub_subscript->resultLabelCount(), default_key_part));
            }
		}

        static void next(void *self_raw) {
			auto &self = *static_cast<UnionOperator *>(self_raw);
			self.sub_op_iter->get()->next();
			if(self.sub_op_iter->get()->ended()) {
                auto sub_op_pos = std::distance(self.sub_operators.cbegin(), self.sub_op_iter);
				// clear the last sub_entry of the sub_operator from the entry
				self.clearEntryKey(self.subscript->getCartesianSubscript().getOriginalResultPoss()[sub_op_pos],
                                   *self.entry);
                self.sub_op_iter++;
                if(self.sub_op_iter == self.sub_operators.cend()) {
                    self.ended_ = true;
                    return;
                }
				sub_op_pos++;
				self.sub_op_iter->get()->load(std::move(self.extractOperands(sub_op_pos, self.operands)),
											  self.sub_entries[sub_op_pos]);
				while(self.sub_op_iter != self.sub_operators.cend() and self.sub_op_iter->get()->ended()) {
					self.sub_op_iter++;
                    if(self.sub_op_iter == self.sub_operators.cend()) {
                        self.ended_ = true;
                        return;
                    }
					sub_op_pos++;
                    self.sub_op_iter->get()->load(std::move(self.extractOperands(sub_op_pos, self.operands)),
                                                  self.sub_entries[sub_op_pos]);
				}
			}
            // updated entry with the sub_entry of the active sub_operator
            auto sub_op_pos = std::distance(self.sub_operators.cbegin(), self.sub_op_iter);
            updateEntryKey(self.subscript->getCartesianSubscript().getOriginalResultPoss()[sub_op_pos],
                           *self.entry,
                           self.sub_entries[sub_op_pos].key);
		}

        static bool ended(const void *self_raw) {
            auto &self = *static_cast<const UnionOperator *>(self_raw);
            return self.ended_ or self.context->hasTimedOut();
        }

        static void
        load(void *self_raw, std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
            static_cast<UnionOperator *>(self_raw)->load_impl(std::move(operands), entry);
        }

        static void clear(void *self_raw) {
            return static_cast<UnionOperator *>(self_raw)->clear_impl();
        }

	private:

        inline void clear_impl(){
            for (auto &sub_operator : sub_operators)
                sub_operator->clear();
            for (auto &sub_entry : sub_entries)
                sub_entry.clear(default_key_part);
        }

        inline void load_impl(std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			if constexpr (_debugeinsum_) fmt::print("Union {}\n", this->subscript);

			this->operands = std::move(operands);
            this->entry = &entry;

            sub_op_iter = sub_operators.cbegin();
			std::size_t sub_op_pos = 0;

			// load first operand
			sub_op_iter->get()->load(std::move(extractOperands(0, this->operands)), sub_entries[0]);
			while(sub_op_iter != sub_operators.cend() and sub_op_iter->get()->ended()) {
				sub_op_iter++;
				sub_op_pos++;
                sub_op_iter->get()->load(std::move(extractOperands(sub_op_pos, this->operands)),
										 sub_entries[sub_op_pos]);
			}
			ended_ = (sub_op_iter == sub_operators.cend());
			if(ended_)
				return;

			// updated entry with the sub_entry of the active sub_operator
			updateEntryKey(this->subscript->getCartesianSubscript().getOriginalResultPoss()[sub_op_pos],
						   *this->entry,
						   sub_entries[sub_op_pos].key);
		}

		// taken from cartesian
        static void updateEntryKey(const OriginalResultPoss &original_result_poss,
								   Entry_t &sink,
								   const typename Entry_t::Key &source_key) {
            for (auto i : iter::range(original_result_poss.size()))
                sink.key[original_result_poss[i]] = source_key[i];
        }

        static void clearEntryKey(const OriginalResultPoss &original_result_poss,
                                   Entry_t &sink) {
            for (auto i : iter::range(original_result_poss.size()))
                sink.key[original_result_poss[i]] = default_key_part;
        }

		// taken from cartesian
        std::vector<const_Hypertrie<tr>>
        extractOperands(OperandPos cart_op_pos, const std::vector<const_Hypertrie<tr>> &operands) {
            std::vector<const_Hypertrie<tr>> sub_operands;
            for (const auto &original_op_pos : this->subscript->getCartesianSubscript().getOriginalOperandPoss(
                    cart_op_pos))
                sub_operands.emplace_back(operands[original_op_pos]);
            return sub_operands;
        }

	};

}

#endif//HYPERTRIE_UNIONOPERATOR_HPP
