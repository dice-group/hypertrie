#ifndef QUERY_ENTRYGENERATOROPERATOR_HPP
#define QUERY_ENTRYGENERATOROPERATOR_HPP

#include "Operator_predeclare.hpp"

namespace dice::query::operators {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct EntryGeneratorOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;

		inline static std::generator<Entry<value_type, htt_t> const &> generator(
				[[maybe_unused]] OperandDependencyGraph const &odg,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				[[maybe_unused]] Query<htt_t, allocator_type> const &query,
				Entry<value_type, htt_t> &entry_arg) {
			assert(false);
			// must not be used
			co_yield entry_arg;
		}

		inline static Entry<value_type, htt_t> const &single_result(
				[[maybe_unused]] OperandDependencyGraph const &odg,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				[[maybe_unused]] Query<htt_t, allocator_type> const &query,
				Entry<value_type, htt_t> &entry_arg) {
			entry_arg.value(1);
			return entry_arg;
		}
	};
}// namespace dice::query::operators
#endif//QUERY_ENTRYGENERATOROPERATOR_HPP
