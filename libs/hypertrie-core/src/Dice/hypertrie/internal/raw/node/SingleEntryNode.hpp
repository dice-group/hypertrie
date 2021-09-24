#ifndef HYPERTRIE_SINGLEENTRYNODE_HPP
#define HYPERTRIE_SINGLEENTRYNODE_HPP


#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/ReferenceCounted.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleEntry.hpp>
#include <Dice/hypertrie/internal/raw/node/Valued.hpp>
#include <cstddef>


namespace hypertrie::internal::raw {

	template<size_t depth, HypertrieCoreTrait tri_t>
	class SingleEntryNode : public ReferenceCounted, public SingleEntry<depth, tri_t>, public Valued<tri_t> {
	public:
		using tri = tri_t;
		using RawKey = RawKey<depth, tri_t>;
		using value_type = typename tri::value_type;

		SingleEntryNode() = default;

		SingleEntryNode(const RawKey &key, value_type value, size_t ref_count = 0)
			: ReferenceCounted(ref_count), SingleEntry<depth, tri_t>(key), Valued<tri_t>(value) {}
	};

	template<size_t depth, HypertrieCoreTrait_bool_valued tri_t>
	class SingleEntryNode<depth, tri_t> : public ReferenceCounted, public SingleEntry<depth, tri_t> {
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
