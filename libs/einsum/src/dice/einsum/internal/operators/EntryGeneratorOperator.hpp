#ifndef HYPERTRIE_ENTRYGENERATOROPERATOR_HPP
#define HYPERTRIE_ENTRYGENERATOROPERATOR_HPP

#include "dice/einsum/internal/operators/Operator_predeclare.hpp"

namespace dice::einsum::internal::operators {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct EntryGeneratorOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;

		inline static std::generator<Entry<value_type, htt_t> const &> generator(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			assert(false);
			// must not be used
			entry_arg.value(1);
			co_yield entry_arg;
		}

		inline static Entry<value_type, htt_t> const &single_result(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			entry_arg.value(1);
			return entry_arg;
		}
	};
}// namespace dice::einsum::internal::operators
#endif//HYPERTRIE_ENTRYGENERATOROPERATOR_HPP
