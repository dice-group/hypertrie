#ifndef HYPERTRIE_RESOLVEOPERATOR_HPP
#define HYPERTRIE_RESOLVEOPERATOR_HPP

#include "Dice/einsum/internal/Context.hpp"

namespace einsum::internal {

	template<typename value_type, HypertrieTrait tr_t>
	class ResolveOperator : public Operator<value_type, tr_t> {
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"
		using ResolveOperator_t = ResolveOperator<value_type, tr>;

		LabelPossInOperand label_pos_in_result;
		bool ended_;

		typename const_Hypertrie<tr>::const_iterator operand_iter;

	public:
		ResolveOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context> &context)
				: Operator_t(Subscript::Type::Resolve, subscript, context, this) {
			label_pos_in_result = this->subscript->operand2resultMapping_ResolveType();
			ended_ = true;
		}


		static void next(void *self_raw) {
			auto &self = *static_cast<ResolveOperator *>(self_raw);
			++self.operand_iter;
			self.ended_ = not self.operand_iter;
			if (self.ended_)
				return;
			self.entry->value = value_type(1);
			const auto &operand_key = *self.operand_iter;
			for (auto i : iter::range(operand_key.size()))
				self.entry->key[self.label_pos_in_result[i]] = operand_key[i];
			if constexpr (bool_value_type) {
				if (self.subscript->all_result_done) {
					self.ended_ = true;
					return;
				}
			}

			if constexpr (_debugeinsum_)
				fmt::print("[{}]->{} {}\n", fmt::join(self.entry->key, ","), self.entry->value, self.subscript);
		}

		static bool ended(const void *self_raw) {
			auto &self = *static_cast<const ResolveOperator *>(self_raw);
			return self.ended_ or self.context->hasTimedOut();
		}

		static void clear([[maybe_unused]]void *self_raw) {
			//
		}

		static void
		load(void *self_raw, std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			auto &self = *static_cast<ResolveOperator *>(self_raw);
			self.load_impl(std::move(operands), entry);
		}

	private:
		inline void load_impl(std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			if constexpr(_debugeinsum_) fmt::print("Resolve {}\n", this->subscript);
			this->entry = &entry;
			assert(operands.size() == 1); // only one operand must be left to be resolved
			operand_iter = std::move(operands[0].cbegin());
			assert(operand_iter);
			ended_ = not operand_iter;
			if (not ended_) {
				this->entry->clear(default_key_part);
				this->entry->value = value_type(1);
				const auto &operand_key = *this->operand_iter;
				for (auto i : iter::range(operand_key.size()))
					this->entry->key[this->label_pos_in_result[i]] = operand_key[i];
			}
		}

	};
}
#endif //HYPERTRIE_RESOLVEOPERATOR_HPP
