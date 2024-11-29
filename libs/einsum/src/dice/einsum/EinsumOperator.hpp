#ifndef HYPERTRIE_EINSUMOPERATOR_HPP
#define HYPERTRIE_EINSUMOPERATOR_HPP

#include "dice/einsum/internal/operators/Operator.hpp"

#include <robin_hood.h>

namespace dice::einsum {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	std::generator<Entry<value_type, htt_t> const &> einsum(
			std::shared_ptr<Subscript> const &subscript,
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
			std::chrono::steady_clock::time_point end_time = internal::Context::time_point::max()) {
		using namespace internal::operators;
		constexpr bool bool_valued = std::is_same_v<value_type, bool>;

		auto context = std::make_shared<internal::Context>(end_time);
		context->check_time_out();
		auto entry_arg = Entry<value_type, htt_t>::make_filled(subscript->resultLabelCount(), {}, value_type(1));
		if (subscript->all_result_done) {
			auto const &entry = get_sub_operator<value_type, htt_t, allocator_type, true>(subscript, context, operands, entry_arg);
			if (entry.value()) {
				co_yield get_sub_operator<value_type, htt_t, allocator_type, true>(subscript, context, operands, entry_arg);
			}
		} else {
			if constexpr (bool_valued) {
				robin_hood::unordered_set<size_t, std::identity> found_entries{};
				for (auto const &entry : get_sub_operator<value_type, htt_t, allocator_type, false>(subscript, context, operands, entry_arg)) {
					size_t const hash = dice::hash::DiceHashwyhash<Entry<value_type, htt_t>>()(entry);
					auto [_, is_new_entry] = found_entries.emplace(hash);
					if (is_new_entry) {
						co_yield entry;
					}
				}
			} else {
				co_yield std::elements_of(get_sub_operator<value_type, htt_t, allocator_type, false>(subscript, context, operands, entry_arg));
			}
		}
	}

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	std::generator<Entry<value_type, htt_t> const &> einsum(
			std::shared_ptr<Subscript> const &subscript,
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
			std::chrono::steady_clock::duration time_out_duration) {
		return einsum(subscript, operands, internal::Context::clock ::now() + time_out_duration);
	}
}// namespace dice::einsum
#endif//HYPERTRIE_EINSUMOPERATOR_HPP
