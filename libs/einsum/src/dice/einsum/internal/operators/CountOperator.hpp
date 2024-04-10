#ifndef HYPERTRIE_COUNTOPERATOR_HPP
#define HYPERTRIE_COUNTOPERATOR_HPP

#include "dice/einsum/internal/operators/Operator_predeclare.hpp"

namespace dice::einsum::internal::operators {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct CountOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;

		inline static std::generator<Entry<value_type, htt_t> const &> generator(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				[[maybe_unused]] Entry<value_type, htt_t> &entry_arg) {
			assert(false);// only one operand must be left to be resolved
			co_yield entry_arg;
		}

		inline static Entry<value_type, htt_t> const &single_result(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			assert(operands.size() == 1);// only one operand must be left to be resolved
			assert(not operands[0].empty());

			if constexpr (bool_valued) {
				entry_arg.value(true);// for bool it is true anyways
			} else {
				entry_arg.value(operands[0].size());
			}
			return entry_arg;
		}
	};

}// namespace dice::einsum::internal::operators
#endif//HYPERTRIE_COUNTOPERATOR_HPP
