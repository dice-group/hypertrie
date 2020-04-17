#ifndef HYPERTRIE_COUNTOPERATOR_HPP
#define HYPERTRIE_COUNTOPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"

namespace einsum::internal {

	template<typename value_type, typename key_part_type, template<typename, typename> class map_type,
			template<typename> class set_type, template<typename, template<typename, typename> class map_type_a,
            template<typename> class set_type_a> class const_BoolHypertrie, typename Diagonal>
	class CountOperator : public Operator<value_type, key_part_type, map_type, set_type,const_BoolHypertrie, Diagonal> {
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"

		using CountOperator_t = CountOperator<value_type, key_part_type, map_type, set_type, const_BoolHypertrie, Diagonal>;
		using Operator_t = Operator<value_type, key_part_type, map_type, set_type, const_BoolHypertrie, Diagonal>;
		using const_BoolHypertrie_t = const_BoolHypertrie<key_part_type, map_type, set_type>;
		bool _ended;
	public:
		CountOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context> &context)
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

		static void
		load(void *self_raw, std::vector<const_BoolHypertrie_t> operands, Entry <key_part_type, value_type> &entry) {
			static_cast<CountOperator *>(self_raw)->load_impl(operands, entry);
		}

		static std::size_t hash(const void *self_raw) {
			return static_cast<const CountOperator *>(self_raw)->subscript->hash();
		}

	private:
		inline void load_impl(std::vector<const_BoolHypertrie_t> operands, Entry <key_part_type, value_type> &entry) {
			this->entry = &entry;
			assert(operands.size() == 1); // only one operand must be left to be resolved
			this->entry->value = operands[0].size();
			_ended = not this->entry->value;
			if (not ended(this))
				for (auto &key_part : entry.key)
					key_part = std::numeric_limits<key_part_type>::max();
		}

	};
}
#endif //HYPERTRIE_COUNTOPERATOR_HPP
