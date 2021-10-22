#ifndef HYPERTRIE_JOINOPERATOR_HPP
#define HYPERTRIE_JOINOPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"
#include <Dice/hypertrie/HashJoin.hpp>

namespace Dice::einsum::internal {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued tr_t>
	class JoinOperator : public Operator<value_type, tr_t> {
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"

		using Join_t = hypertrie::HashJoin<tr>;
		using JoinOperator_t = JoinOperator<value_type, tr>;

		Join_t join;
		typename Join_t::iterator join_iter;
		bool is_result_label = false;
		LabelPossInOperands label_poss_in_ops;
		LabelPos label_pos_in_result;
		Label label = std::numeric_limits<Label>::max();// TODO: type the unusable value (max) explicitly
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
					std::vector<hypertrie::const_Hypertrie<tr>> next_operands;
					std::tie(next_operands, current_key_part) = *join_iter;
					sub_operator->load(std::move(next_operands), *this->entry);
				} else {
					ended_ = true;
					return;
				}
			}
			if (is_result_label)
				this->entry->key()[label_pos_in_result] = current_key_part;

			if constexpr (_debugeinsum_)
				fmt::print("[{}]->{} {}\n", fmt::join(this->entry->key, ","), this->entry->value, this->subscript);
		}

	public:
		static bool ended(const void *self_raw) {
			auto &self = *static_cast<const JoinOperator *>(self_raw);
			return self.ended_ or self.context->hasTimedOut();
		}

		static void clear(void *self_raw) {
			return static_cast<JoinOperator_t *>(self_raw)->clear_impl();
		}

		static void
		load(void *self_raw, std::vector<hypertrie::const_Hypertrie<tr>> operands, Entry_t &entry) {
			static_cast<JoinOperator *>(self_raw)->load_impl(std::move(operands), entry);
		}

	private:
		inline void clear_impl() {
			if (this->sub_operator)
				this->sub_operator->clear();
			this->join = {};
			this->join_iter = {};
		}

		inline void load_impl(std::vector<hypertrie::const_Hypertrie<tr>> operands, Entry_t &entry) {
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
				std::vector<hypertrie::const_Hypertrie<tr>> next_operands;
				std::tie(next_operands, current_key_part) = *join_iter;
				// initialize the next sub_operator
				sub_operator->load(std::move(next_operands), *this->entry);
				find_next_valid();
			} else {
				this->ended_ = true;
			}
		}
	};
}// namespace Dice::einsum::internal
#endif//HYPERTRIE_JOINOPERATOR_HPP
