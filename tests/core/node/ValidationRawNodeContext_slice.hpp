#ifndef HYPERTRIE_VALIDATIONRAWNODECONTEXT_SLICE_HPP
#define HYPERTRIE_VALIDATIONRAWNODECONTEXT_SLICE_HPP
#include "ValidationRawNodeContext.hpp"
#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/RawKey.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleEntry.hpp>

namespace Dice::hypertrie::tests::core::node {
	using namespace fmt::literals;

	using namespace ::Dice::hypertrie::internal::raw;
	using namespace ::Dice::hypertrie::internal;

	template<size_t depth, size_t fixed_depth, HypertrieCoreTrait tri>
	inline auto slice_entries(std::vector<SingleEntry<depth, tri>> const &entries, RawSliceKey<fixed_depth, tri> const &raw_slice_key) {
		std::vector<SingleEntry<depth - fixed_depth, tri>> result_entries;
		for (const auto &entry : entries)
			if (auto sliced = raw_slice_key.slice(entry.key()); sliced.has_value())
				result_entries.emplace_back(*sliced, entry.value());
		return result_entries;
	}
}// namespace Dice::hypertrie::tests::core::node

#endif//HYPERTRIE_VALIDATIONRAWNODECONTEXT_SLICE_HPP
