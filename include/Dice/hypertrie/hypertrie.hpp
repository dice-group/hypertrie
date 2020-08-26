#ifndef HYPERTRIE_HPP
#define HYPERTRIE_HPP

#include "Dice/hypertrie/internal/Hypertrie.hpp"
#include "Dice/hypertrie/internal/HashJoin.hpp"
#include "Dice/einsum/internal/Einsum.hpp"

namespace hypertrie {
	using Subscript =  einsum::internal::Subscript;
	using RawSubscript = einsum::internal::RawSubscript;

	template<typename value_type, HypertrieTrait tr =default_bool_Hypertrie_t>
	using Einsum = typename ::einsum::internal::Einsum<value_type, tr>;

	template <typename key_part_type>
	using KeyHash = ::einsum::internal::KeyHash<key_part_type>;
	using TimePoint = ::einsum::internal::TimePoint;

	template<typename value_type = size_t, HypertrieTrait tr =default_bool_Hypertrie_t>
	using EinsumEntry = ::einsum::internal::Entry<value_type, tr>;

	// TODO: that is a bit general
	template<typename value_type = std::size_t, HypertrieTrait tr =default_bool_Hypertrie_t>
	static auto
	einsum2map(const std::shared_ptr<Subscript> &subscript,
			   const std::vector<const_Hypertrie<tr>> &operands,
			   const TimePoint &time_point = TimePoint::max()) {
		using Key = typename tr::Key;
		using key_part_type = typename tr::key_part_type;

		tsl::hopscotch_map<Key, value_type, KeyHash<key_part_type>> results{};
		for (const auto &operand : operands)
			if (operand.size() == 0)
				return results;

		Einsum<value_type, tr> einsum{subscript, operands, time_point};
		for (auto &&entry : einsum) {
			results[entry.key] += entry.value;
		}
		return results;
	}
}





#endif//HYPERTRIE_HPP
