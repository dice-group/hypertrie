#ifndef HYPERTRIE_SINGLEENTRYNODE_HPP
#define HYPERTRIE_SINGLEENTRYNODE_HPP

#include <cstddef>

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/ReferenceCounted.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"
#include "dice/hypertrie/internal/raw/node/Valued.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t>
	class SingleEntryNode : public ReferenceCounted, public SingleEntry<depth, htt_t> {
	public:
		using RawKey_t = RawKey<depth, htt_t>;
		using value_type = typename htt_t::value_type;

		SingleEntryNode() noexcept = default;

		explicit SingleEntryNode(SingleEntry<depth, htt_t> const &entry, size_t ref_count = 0) noexcept
			: ReferenceCounted(ref_count), SingleEntry<depth, htt_t>(entry) {}

		/**
		 * Constructor for bool-valued SingleEntryNode
		 * @param key
		 * @param value
		 * @param ref_count
		 */
		SingleEntryNode(const RawKey_t &key, [[maybe_unused]] value_type value, size_t ref_count = 0) noexcept
				requires HypertrieTrait_bool_valued<htt_t>
			: ReferenceCounted(ref_count), SingleEntry<depth, htt_t>(key /* here is the difference to the constructor below */) {}

		/**
		 * Constructor for non-bool-valued SingleEntryNode
		 * @param key
		 * @param value
		 * @param ref_count
		 */
		SingleEntryNode(const RawKey_t &key, [[maybe_unused]] value_type value, size_t ref_count = 0) noexcept
				requires(not HypertrieTrait_bool_valued<htt_t>)
			: ReferenceCounted(ref_count), SingleEntry<depth, htt_t>(key, value) {}

		// TODO: should be inherited from SingleEntry
		/*
		auto operator<=>(const SingleEntryNode &other) const noexcept {
			return this->SingleEntry<depth,htt_t>::operator<=>(other);
		}

		bool operator==(const SingleEntryNode &other) const noexcept {
			return this->SingleEntry<depth,htt_t>::operator==(other);
		}
		 */
	};

}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_SINGLEENTRYNODE_HPP
