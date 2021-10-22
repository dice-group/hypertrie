#ifndef HYPERTRIE_ENTRYGENERATOROPERATOR_HPP
#define HYPERTRIE_ENTRYGENERATOROPERATOR_HPP

#include "Dice/einsum/internal/Operator.hpp"

namespace Dice::einsum::internal {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued tr_t>
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

		static void clear([[maybe_unused]] void *self_raw) {
			//
		}

		static void load(void *self_raw, std::vector<hypertrie::const_Hypertrie<tr>> operands, Entry_t &entry) {
			static_cast<EntryGeneratorOperator *>(self_raw)->load_impl(std::move(operands), entry);
		}

	private:
		inline void load_impl([[maybe_unused]] std::vector<hypertrie::const_Hypertrie<tr>> operands,
							  Entry_t &entry) {
			assert(operands.size() == 0);// no operand must be left
			if constexpr (_debugeinsum_) fmt::print("EntryGen {}\n", this->subscript);
			this->entry = &entry;
			_ended = false;
			this->entry->clear(default_key_part);
			this->entry->value = value_type(1);
		}
	};
}// namespace Dice::einsum::internal
#endif//HYPERTRIE_ENTRYGENERATOROPERATOR_HPP
