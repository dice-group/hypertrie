#ifndef HYPERTRIE_RESOLVEOPERATOR_HPP
#define HYPERTRIE_RESOLVEOPERATOR_HPP

#include "Dice/einsum/internal/Context.hpp"

namespace einsum::internal {

	template<typename value_type, typename key_part_type, template<typename, typename> class map_type,
			template<typename> class set_type, template<typename, template<typename, typename> class map_type_a,
            template<typename> class set_type_a> class const_BoolHypertrie, typename Diagonal>
	class ResolveOperator : public Operator<value_type, key_part_type, map_type, set_type, const_BoolHypertrie, Diagonal> {
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"
		using ResolveOperator_t = ResolveOperator<value_type, key_part_type, map_type, set_type, const_BoolHypertrie, Diagonal>;
        using Operator_t = Operator<value_type, key_part_type, map_type, set_type, const_BoolHypertrie, Diagonal>;
        constexpr static const key_part_type default_key_part = Operator_t::default_key_part;
        constexpr static const bool bool_value_type = Operator_t::bool_value_type;
        using const_BoolHypertrie_t = const_BoolHypertrie<key_part_type, map_type, set_type>;
		LabelPossInOperand label_pos_in_result;
		bool ended_;

		typename const_BoolHypertrie_t::const_iterator operand_iter;

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

		static void
		load(void *self_raw, std::vector<const_BoolHypertrie_t> operands, Entry <key_part_type, value_type> &entry) {
			auto &self = *static_cast<ResolveOperator *>(self_raw);
			self.load_impl(std::move(operands), entry);
		}

		static std::size_t hash(const void *self_raw) {
			auto &self = *static_cast<const ResolveOperator *>(self_raw);
			return self.subscript->hash();
		}

	private:
		inline void load_impl(std::vector<const_BoolHypertrie_t> operands, Entry <key_part_type, value_type> &entry) {
			if constexpr(_debugeinsum_) fmt::print("Resolve {}\n", this->subscript);
			this->entry = &entry;
			assert(operands.size() == 1); // only one operand must be left to be resolved
			operand_iter = std::move(operands[0].cbegin());
			assert(operand_iter);
			ended_ = not operand_iter;
			if (not ended_) {
				for (auto &key_part : entry.key)
					key_part = default_key_part;
				this->entry->value = value_type(1);
				const auto operand_key = *this->operand_iter;
				for (auto i : iter::range(operand_key.size()))
					this->entry->key[this->label_pos_in_result[i]] = operand_key[i];
			}
		}

	};
}
#endif //HYPERTRIE_RESOLVEOPERATOR_HPP
