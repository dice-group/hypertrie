#ifndef HYPERTRIE_JOINOPERATOR_HPP
#define HYPERTRIE_JOINOPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"

namespace einsum::internal {

	template<typename value_type, typename key_part_type, template<typename, typename> class map_type,
			template<typename> class set_type, template<typename, template<typename, typename> class map_type_a,
            template<typename> class set_type_a> class const_BoolHypertrie,
                    typename Diagonal>
	class JoinOperator : public Operator<value_type, key_part_type, map_type, set_type, const_BoolHypertrie, Diagonal> {
		using JoinOperator_t = JoinOperator<value_type, key_part_type, map_type, set_type, const_BoolHypertrie, Diagonal>;
        using const_BoolHypertrie_t = const_BoolHypertrie<key_part_type, map_type, set_type>;
        using Join_t = Join<key_part_type, map_type, set_type, const_BoolHypertrie_t, Diagonal>;
        using Operator_t = Operator<value_type, key_part_type, map_type, set_type, const_BoolHypertrie, Diagonal>;
        using CardinalityEstimation_t = typename Operator_t::CardinalityEstimation_t;
        constexpr static const bool bool_value_type = Operator_t::bool_value_type;
		Join_t join;
		typename Join_t::iterator join_iter;
		bool is_result_label = false;
		LabelPossInOperands label_poss_in_ops;
		LabelPos label_pos_in_result;
		Label label = std::numeric_limits<Label>::max(); // TODO: type the unusable value (max) explicitly
		key_part_type current_key_part;

		std::shared_ptr<Operator_t> sub_operator;

		bool ended_ = true;

	public:
		JoinOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context> &context)
				: Operator_t(Subscript::Type::Join, subscript, context, this) {}


		static void next(void *self_raw) {
			JoinOperator &self = *static_cast<JoinOperator *>(self_raw);
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
					sub_operator->load(std::move(next_operands), *this->entry);
				} else {
					ended_ = true;
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
			label = CardinalityEstimation_t::getMinCardLabel(operands, this->subscript, this->context);
			if (label != last_label) {
				label_poss_in_ops = this->subscript->getLabelPossInOperands(label);
				is_result_label = this->subscript->isResultLabel(label);
				if (is_result_label)
					label_pos_in_result = this->subscript->getLabelPosInResult(label);
			}
			const std::shared_ptr<Subscript> &next_subscript = this->subscript->removeLabel(label);
			// check if sub_operator was not yet initialized or if the next subscript is different
			if (not sub_operator or sub_operator->hash() != next_subscript->hash()) {
				sub_operator = Operator_t::construct(next_subscript, this->context);
			}

			// initialize the join
			join = Join_t{operands, label_poss_in_ops};
			join_iter = join.begin();
			// check if join has entries
			if (join_iter) {
				std::vector<const_BoolHypertrie_t> next_operands;
				std::tie(next_operands, current_key_part) = *join_iter;
				// initialize the next sub_operator
				sub_operator->load(std::move(next_operands), *this->entry);
				find_next_valid();
			} else {
				this->ended_ = true;
			}
		}

	};
}
#endif //HYPERTRIE_JOINOPERATOR_HPP
