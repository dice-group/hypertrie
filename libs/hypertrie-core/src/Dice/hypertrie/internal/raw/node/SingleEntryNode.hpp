#ifndef HYPERTRIE_SINGLEENTRYNODE_HPP
#define HYPERTRIE_SINGLEENTRYNODE_HPP


#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/ReferenceCounted.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleEntry.hpp>
#include <cstddef>


namespace hypertrie::internal::raw {
	template<size_t depth, HypertrieCoreTrait tri_t>
	class SingleEntryNode : public ReferenceCounted, public SingleEntry<depth, tri_t> {
	public:
		using tri = tri_t;
		using RawKey = RawKey<depth, tri_t>;

		SingleEntryNode() noexcept = default;

		explicit SingleEntryNode(const RawKey &key, size_t ref_count = 0) noexcept
			: ReferenceCounted(ref_count), SingleEntry<depth, tri_t>(key) {}

		[[nodiscard]] constexpr bool value() const noexcept { return true; }
	};

}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_SINGLEENTRYNODE_HPP
