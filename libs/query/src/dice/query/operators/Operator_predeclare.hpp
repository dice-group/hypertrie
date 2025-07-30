#ifndef QUERY_OPERATOR_PREDECLARE_HPP
#define QUERY_OPERATOR_PREDECLARE_HPP

#include <dice/hypertrie/internal/commons/generator.hpp>

#include "dice/query/Commons.hpp"
#include "dice/query/OperandDependencyGraph.hpp"
#include "dice/query/Query.hpp"


namespace dice::query::operators {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_result_done>
	inline std::conditional_t<all_result_done, Entry<value_type, htt_t> const &, std::generator<Entry<value_type, htt_t> const &>>
	get_sub_operator(OperandDependencyGraph &odg,
					 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					 Query<htt_t, allocator_type> const &query,
					 Entry<value_type, htt_t> &entry);

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline void clear_used_entry_poss(Entry<value_type, htt_t> &entry,
									  OperandDependencyGraph &graph,
									  Query<htt_t, allocator_type> const &query) noexcept;

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline static std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>
	extract_operands(OperandDependencyGraph &odg,
					 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands);

}// namespace dice::query::operators

#endif//QUERY_OPERATOR_PREDECLARE_HPP
