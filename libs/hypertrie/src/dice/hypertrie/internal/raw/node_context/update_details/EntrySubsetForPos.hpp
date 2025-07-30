#ifndef ENTRYSUBSETFORPOS_HPP
#define ENTRYSUBSETFORPOS_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"

#include <robin_hood.h>

#include <cstddef>
#include <vector>

namespace dice::hypertrie::internal::raw::node_context::update_details {


	template<size_t depth, HypertrieTrait htt_t>
	auto entry_subset_for_pos(std::vector<SingleEntry<depth, htt_t>> const &entries, size_t pos) noexcept
			-> ::robin_hood::unordered_map<typename htt_t::key_part_type, std::vector<SingleEntry<(depth - 1), htt_t>>> {
		::robin_hood::unordered_map<typename htt_t::key_part_type, std::vector<SingleEntry<(depth - 1), htt_t>>> ret;

		for (auto const &e : entries) {
			ret[e.key()[pos]].emplace_back(e.key().subkey(pos), e.value());
		}

		return ret;
	}
}// namespace dice::hypertrie::internal::raw::node_context::update_details
#endif//ENTRYSUBSETFORPOS_HPP
