#ifndef HYPERTRIE_JOINOPERATOR_HPP
#define HYPERTRIE_JOINOPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"

namespace einsum::internal {

	template<typename value_type, typename key_part_type, template<typename, typename> class map_type,
			template<typename> class set_type>
	class JoinOperator : public Operator<value_type, key_part_type, map_type, set_type> {
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"

		using Join_t = Join<key_part_type, map_type, set_type>;
		using JoinOperator_t = JoinOperator<value_type, key_part_type, map_type, set_type>;

		Join_t join;
		typename Join_t::iterator join_iter;
		bool is_result_label = false;
		LabelPossInOperands label_poss_in_ops;
		LabelPos label_pos_in_result;
		Label label = std::numeric_limits<Label>::max(); // TODO: type the unusable value (max) explicitly
		key_part_type current_key_part;

		std::shared_ptr<Operator_t> sub_operator;

		bool ended_ = true;

		bool label_is_fixed;

	public:
		JoinOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context < key_part_type>>

		&context)
		:
		Operator_t(Subscript::Type::Join, subscript, context,
		this) {}


		static void next(void *self_raw) {
			JoinOperator &self = *static_cast<JoinOperator *>(self_raw);
			if (self.label_is_fixed){
				self.sub_operator->next();
				if(self.sub_operator->ended())
					self.ended_ = true;
				return;
			}
			if constexpr (bool_value_type) {
				if (self.subscript->all_result_done) {
					assert(self.sub_operator);
					self.ended_ = true;
					if (not self.label_is_fixed) self.context->fixed_labels.erase(self.label);
					return;
				}
			}
			self.sub_operator->next();
			self.find_next_valid();
		}

	private:
		/**
		 * requires bool(join_iter) == true (not ended).
		 * Finds the next valid entry of a sub_operator.
		 * - If sub_operator has already a valid entry, it is not incremented.
		 * - If sub_operator has ended, join_iter is increased until either a valid sub_operator entry is found or join_iter is ended
		 * Finally, results are written to entry.
		 */
		void find_next_valid() {
			assert(join_iter);
			while (sub_operator->ended()) {
				++join_iter;
				if (join_iter and not this->context->hasTimedOut()) {
					std::vector<const_BoolHypertrie_t> next_operands;
					std::tie(next_operands, current_key_part) = *join_iter;
					this->context->fixed_labels[label] = current_key_part;
					sub_operator->load(std::move(next_operands), *this->entry);
				} else {
					ended_ = true;
					if (not label_is_fixed) this->context->fixed_labels.erase(label);
					break;
				}
			}
			if (is_result_label)
				this->entry->key[label_pos_in_result] = current_key_part;

			if constexpr (_debugeinsum_)
				fmt::print("[{}]->{} {}\n", fmt::join(this->entry->key, ","), this->entry->value, this->subscript);
		}

	public:
		static bool ended(const void *self_raw) {
			auto &self = *static_cast<const JoinOperator *>(self_raw);
			return self.ended_ or self.context->hasTimedOut();
		}

		static void
		load(void *self_raw, std::vector<const_BoolHypertrie_t> operands, Entry <key_part_type, value_type> &entry) {
			static_cast<JoinOperator *>(self_raw)->load_impl(std::move(operands), entry);
		}

		static std::size_t hash(const void *self_raw) {
			return static_cast<const JoinOperator *>(self_raw)->subscript->hash();
		}

	private:
		inline void load_impl(std::vector<const_BoolHypertrie_t> operands, Entry <key_part_type, value_type> &entry) {
			if constexpr (_debugeinsum_) fmt::print("Join {}\n", this->subscript);

			this->entry = &entry;
			ended_ = false;
			Label last_label = label;
			// TODO: return the position of the selected label in the operands
			LabelCardInfo label_card_info = CardinalityEstimation_t::getMinCardLabel(operands, this->subscript,
																					 this->context);
			label = label_card_info.label;
			label_poss_in_ops = label_card_info.label_poss_in_operands;
			label_is_fixed = bool(this->context->getFixedLabel(label));
			is_result_label = this->subscript->isResultLabel(label);
			if (is_result_label)
				label_pos_in_result = this->subscript->getLabelPosInResult(label);
			// TODO: remove label by (op_pos + label_pos) list
			const std::shared_ptr<Subscript> &next_subscript = this->subscript->removeLabel(label_poss_in_ops);
			// check if sub_operator was not yet initialized or if the next subscript is different
			if (not sub_operator or sub_operator->hash() != next_subscript->hash()) {
				sub_operator = Operator_t::construct(next_subscript, this->context);
			}

			if (label_is_fixed) {
				using SliceKey = typename const_BoolHypertrie_t::SliceKey;
				OperandPos op_pos = [&]() {
					for (const auto[pos, op] : iter::enumerate(label_poss_in_ops))
						if (op.size() > 0) return pos;
					throw std::logic_error("");
				}();
				LabelPos label_pos = label_poss_in_ops[op_pos][0];
				const const_BoolHypertrie_t &operand = operands[op_pos];
				SliceKey slice_key(operand.depth());
				slice_key[label_pos] = this->context->getFixedLabel(label);
				std::vector<const_BoolHypertrie_t> next_operands{operands};
				std::variant<std::optional<const_BoolHypertrie_t>, bool> slice = operand[slice_key];
				if (std::holds_alternative<std::optional<const_BoolHypertrie_t>>(slice)) {
					std::optional<const_BoolHypertrie_t> &next_operand_opt =
							std::get<std::optional<const_BoolHypertrie_t>>(slice);
					if (next_operand_opt.has_value()) {
						next_operands[op_pos] = next_operand_opt.value();
						sub_operator->load(std::move(next_operands), *this->entry);
						if (sub_operator->ended())
							this->ended_ = true;
						return;
					}
				} else {
					if (std::get<bool>(slice)) {
						next_operands.erase(next_operands.begin() + op_pos);
						sub_operator->load(std::move(next_operands), *this->entry);
						if (sub_operator->ended())
							this->ended_ = true;
						return;
					}
				}
			} else {

				// initialize the join
				join = Join_t{operands, label_poss_in_ops};
				join_iter = join.begin();
				// check if join has entries
				if (join_iter) {
					std::vector<const_BoolHypertrie_t> next_operands;
					std::tie(next_operands, current_key_part) = *join_iter;
					this->context->fixed_labels[label] = current_key_part;
					// initialize the next sub_operator
					sub_operator->load(std::move(next_operands), *this->entry);
					find_next_valid();
					return;
				}
			}
			this->ended_ = true;
		}

	};
}
#endif //HYPERTRIE_JOINOPERATOR_HPP
