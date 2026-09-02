#ifndef HYPERTRIE_ENTRYGENERATOROPERATOR_HPP
#define HYPERTRIE_ENTRYGENERATOROPERATOR_HPP

#include "dice/einsum/internal/operators/Operator_predeclare.hpp"

namespace dice::einsum::internal::operators {

	/**
	 * This generator produces entries.
	 */
	// TODO: integrate into Join and Cartesian Operator directly.
	template<typename value_type, hypertrie::HypertrieTrait htt_t, hypertrie::ByteAllocator allocator_type>
	struct EntryGeneratorOperator {
		inline static std::generator<Entry<value_type, htt_t> const &> generator(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			assert(false);
			// must not be used
			entry_arg.set_value(1);
			co_yield entry_arg;
		}

		inline static Entry<value_type, htt_t> const &single_result(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			entry_arg.set_value(1);
			return entry_arg;
		}
	};
}// namespace dice::einsum::internal::operators
#endif//HYPERTRIE_ENTRYGENERATOROPERATOR_HPP
