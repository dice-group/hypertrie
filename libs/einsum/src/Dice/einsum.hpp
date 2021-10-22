#ifndef EINSUM_H_HPP
#define EINSUM_H_HPP

//#include "Dice/hypertrie/hypertrie_version.hpp"
// TODO: add version again

#include "Dice/einsum/internal/Einsum.hpp"
#include "Dice/hypertrie.hpp"

namespace Dice::einsum {
	using Subscript = einsum::internal::Subscript;
	using RawSubscript = einsum::internal::RawSubscript;

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued tr = hypertrie::default_bool_Hypertrie_trait>
	using Einsum = typename ::Dice::einsum::internal::Einsum<value_type, tr>;

	using TimePoint = ::Dice::einsum::internal::TimePoint;

	template<typename value_type = size_t, hypertrie::HypertrieTrait_bool_valued tr = hypertrie::default_bool_Hypertrie_trait>
	using EinsumEntry = ::Dice::einsum::internal::Entry<value_type, tr>;

	// TODO: that is a bit general
	template<typename value_type = std::size_t, hypertrie::HypertrieTrait_bool_valued tr = hypertrie::default_bool_Hypertrie_trait>
	static auto
	einsum2map(const std::shared_ptr<Subscript> &subscript,
			   const std::vector<hypertrie::const_Hypertrie<tr>> &operands,
			   const TimePoint &time_point = TimePoint::max()) {
		using Key_t = internal::Key<value_type, tr>;

		tsl::sparse_map<Key_t, value_type, hash::DiceHash<Key_t>> results{};
		for (const auto &operand : operands)
			if (operand.size() == 0)
				return results;

		Einsum<value_type, tr> einsum{subscript, operands, time_point};
		for (auto &&entry : einsum) {
			results[entry.key()] += entry.value();
		}
		return results;
	}
}// namespace Dice::einsum


#endif//EINSUM_H_HPP
