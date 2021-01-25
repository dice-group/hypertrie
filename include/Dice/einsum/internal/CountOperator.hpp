#ifndef HYPERTRIE_COUNTOPERATOR_HPP
#define HYPERTRIE_COUNTOPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"

namespace einsum::internal {

	template<typename value_type, HypertrieTrait tr_t>
	class CountOperator : public Operator<value_type, tr_t> {
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"

		using CountOperator_t = CountOperator<value_type, tr>;
		bool _ended;
	public:
		CountOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context<key_part_type>> &context)
				: Operator_t(Subscript::Type::Count, subscript, context, this) {}

		static void next(void *self_raw) {
			auto &self = *static_cast<CountOperator *>(self_raw);
			self._ended = true;
			if constexpr (_debugeinsum_)
				fmt::print("[{}]->{} {}\n", fmt::join(self.entry->key, ","), self.entry->value, self.subscript);
		}

		static bool ended(const void *self_raw) {
			auto &self = *static_cast<const CountOperator *>(self_raw);
			return self._ended;
		}

		static void clear([[maybe_unused]]void *self_raw) {
			//
		}

		static void
		load(void *self_raw, std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			static_cast<CountOperator *>(self_raw)->load_impl(operands, entry);
		}

	private:
		inline void load_impl(std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			this->entry = &entry;
			assert(operands.size() == 1);// only one operand must be left to be resolved
			_ended = operands[0].empty();
			if (not ended(this)) {
				entry.clear(default_key_part);
				this->entry->value = operands[0].size();
			}
		}
	};
}
#endif //HYPERTRIE_COUNTOPERATOR_HPP
