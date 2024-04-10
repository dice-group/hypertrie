#ifndef HYPERTRIE_OPERATOR_HPP
#define HYPERTRIE_OPERATOR_HPP

#include "dice/einsum/internal/operators/CartesianOperator.hpp"
#include "dice/einsum/internal/operators/CountOperator.hpp"
#include "dice/einsum/internal/operators/EntryGeneratorOperator.hpp"
#include "dice/einsum/internal/operators/JoinOperator.hpp"
#include "dice/einsum/internal/operators/ResolveOperator.hpp"

#include "dice/einsum/internal/operators/Operator_predeclare.hpp"


#include <memory>
#include <utility>

namespace dice::einsum::internal::operators {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_result_done>
	inline auto get_sub_operator(
			const std::shared_ptr<Subscript> &subscript,
			std::shared_ptr<Context> &context,
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
			Entry<value_type, htt_t> &entry) -> std::conditional_t<all_result_done, Entry<value_type, htt_t> const &, std::generator<Entry<value_type, htt_t> const &>> {
		if constexpr (all_result_done) {
			switch (subscript->type) {
				case Subscript::Type::Join: {
					return JoinOperator<value_type, htt_t, allocator_type>::single_result(subscript, context, operands, entry);
				}
				case Subscript::Type::Resolve: {
					return ResolveOperator<value_type, htt_t, allocator_type>::single_result(subscript, context, operands, entry);
				}
				case Subscript::Type::Count: {
					return CountOperator<value_type, htt_t, allocator_type>::single_result(subscript, context, operands, entry);
				}
				case Subscript::Type::Cartesian: {
					return CartesianOperator<value_type, htt_t, allocator_type>::single_result(subscript, context, operands, entry);
				}
				case Subscript::Type::EntryGenerator: {
					return EntryGeneratorOperator<value_type, htt_t, allocator_type>::single_result(subscript, context, operands, entry);
				}
				default:
					throw std::invalid_argument{"subscript is of an undefined type."};
			}
		} else {
			switch (subscript->type) {
				case Subscript::Type::Join: {
					return JoinOperator<value_type, htt_t, allocator_type>::generator(subscript, context, operands, entry);
				}
				case Subscript::Type::Resolve: {
					return ResolveOperator<value_type, htt_t, allocator_type>::generator(subscript, context, operands, entry);
				}
				case Subscript::Type::Count: {
					return CountOperator<value_type, htt_t, allocator_type>::generator(subscript, context, operands, entry);
				}
				case Subscript::Type::Cartesian: {
					return CartesianOperator<value_type, htt_t, allocator_type>::generator(subscript, context, operands, entry);
				}
				case Subscript::Type::EntryGenerator: {
					return EntryGeneratorOperator<value_type, htt_t, allocator_type>::generator(subscript, context, operands, entry);
				}
				default:
					throw std::invalid_argument{"subscript is of an undefined type."};
			}
		}
	};

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t>
	inline void clear_used_entry_poss(Entry<value_type, htt_t> &entry, std::shared_ptr<Subscript> const &subscript) noexcept {
		for (auto const &result_pos : subscript->getUsedResultPoss()) {
			entry[result_pos] = {};
		}
	}


}// namespace dice::einsum::internal::operators

#endif//HYPERTRIE_OPERATOR_HPP
