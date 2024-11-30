#ifndef HYPERTRIE_OPERATOR_PREDECLARE_HPP
#define HYPERTRIE_OPERATOR_PREDECLARE_HPP

#include "dice/einsum/Commons.hpp"
#include "dice/einsum/Subscript.hpp"
#include "dice/einsum/internal/Context.hpp"

#include <dice/hypertrie.hpp>
#include <dice/hypertrie/internal/commons/generator.hpp>

#include <memory>
#include <utility>

namespace dice::einsum::internal::operators {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_result_done>
	inline auto get_sub_operator(
			const std::shared_ptr<Subscript> &subscript,
			std::shared_ptr<Context> &context,
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
			Entry<value_type, htt_t> &entry) -> std::conditional_t<all_result_done, Entry<value_type, htt_t> const &, std::generator<Entry<value_type, htt_t> const &>>;

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t>
	inline void clear_used_entry_poss(Entry<value_type, htt_t> &entry, std::shared_ptr<Subscript> const &subscript) noexcept;


}// namespace dice::einsum::internal::operators

#endif//HYPERTRIE_OPERATOR_PREDECLARE_HPP
