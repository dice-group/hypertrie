#ifndef HYPERTRIE_ENTRYGENERATOROPERATOR_HPP
#define HYPERTRIE_ENTRYGENERATOROPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"

namespace einsum::internal {

	template<typename value_type, HypertrieTrait tr_t>
	class EntryGeneratorOperator : public Operator<value_type, tr_t> {
#include "Dice/einsum/internal/OperatorMemberTypealiases.hpp"
		using EntryGeneratorOperator_t = EntryGeneratorOperator<value_type, tr>;

		bool _ended = true;

	public:
		EntryGeneratorOperator(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context> &context)
				: Operator_t(Subscript::Type::EntryGenerator, subscript, context, this) {}

		static void next(void *self_raw) {
			auto &self = *static_cast<EntryGeneratorOperator *>(self_raw);
			self._ended = true;

			if constexpr (_debugeinsum_) fmt::print("[{}]->{} {}\n", fmt::join(self.entry->key, ","), self.entry->value, self.subscript);
		}

		static bool ended(const void *self_raw) {
			return static_cast<const EntryGeneratorOperator *>(self_raw)->_ended;
		}

		static void load(void *self_raw, std::vector<const_Hypertrie<tr>> operands, Entry_t &entry) {
			static_cast<EntryGeneratorOperator *>(self_raw)->load_impl(std::move(operands), entry);
		}

		static std::size_t hash(const void *self_raw) {
			return static_cast<const EntryGeneratorOperator *>(self_raw)->subscript->hash();
		}

	private:
		inline void load_impl([[maybe_unused]]std::vector<const_Hypertrie<tr>> operands,
							  Entry_t &entry) {
			if constexpr(_debugeinsum_) fmt::print("EntryGen {}\n", this->subscript);
			this->entry = &entry;
			this->entry->value = value_type(1);
			assert(operands.size() == 0); // no operand must be left
			_ended = false;
			if (not _ended)
				for (auto &key_part : entry.key)
					key_part = default_key_part;
		}
	};
}
#endif //HYPERTRIE_ENTRYGENERATOROPERATOR_HPP
