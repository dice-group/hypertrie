#ifndef HYPERTRIE_SINGLEENTRYNODE_HPP
#define HYPERTRIE_SINGLEENTRYNODE_HPP

#include <cstddef>

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/ReferenceCounted.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"
#include "dice/hypertrie/internal/raw/node/Valued.hpp"
#include "dice/hypertrie/internal/raw/node/Hashed.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t>
	struct SingleEntryNode : ReferenceCounted, SingleEntry<depth, htt_t> {
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;

		SingleEntryNode() noexcept = default;

		explicit SingleEntryNode(size_t ref_count) noexcept
			: ReferenceCounted{ref_count} {
		}

		SingleEntryNode(SingleEntry<depth, htt_t> const &entry, size_t ref_count) noexcept
			: ReferenceCounted{ref_count}, SingleEntry<depth, htt_t>{entry} {
		}

		[[nodiscard]] RawIdentifier<depth, htt_t> identifier() const noexcept {
			return RawIdentifier<depth, htt_t>{*this};
		}

		[[nodiscard]] typename Hashed<depth, htt_t>::hash_type hash() const noexcept {
			return RawIdentifier<depth, htt_t>::hash_single_entry(*this);
		}
	};

}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_SINGLEENTRYNODE_HPP
