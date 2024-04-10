#ifndef HYPERTRIE_VALIDATIONRAWNODECONTEXT_SLICE_HPP
#define HYPERTRIE_VALIDATIONRAWNODECONTEXT_SLICE_HPP
#include "ValidationRawNodeContext.hpp"
#include <dice/hypertrie/Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/RawKey.hpp>
#include <dice/hypertrie/internal/raw/node/SingleEntry.hpp>

namespace dice::hypertrie::tests::core::node {
	using namespace fmt::literals;

	using namespace ::dice::hypertrie::internal::raw;
	using namespace ::dice::hypertrie::internal;

	template<size_t depth, size_t fixed_depth, HypertrieTrait htt_t>
	inline auto slice_entries(std::vector<SingleEntry<depth, htt_t>> const &entries, RawSliceKey<fixed_depth, htt_t> const &raw_slice_key) {
		std::vector<SingleEntry<depth - fixed_depth, htt_t>> result_entries;
		for (const auto &entry : entries)
			if (auto sliced = raw_slice_key.slice(entry.key()); sliced.has_value())
				result_entries.emplace_back(*sliced, entry.value());
		return result_entries;
	}
}// namespace dice::hypertrie::tests::core::node

#endif//HYPERTRIE_VALIDATIONRAWNODECONTEXT_SLICE_HPP
