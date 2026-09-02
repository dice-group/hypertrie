#ifndef HYPERTRIE_NODETYPES_PREDECLARE_HPP
#define HYPERTRIE_NODETYPES_PREDECLARE_HPP

#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct FullNode;

	template<size_t depth, HypertrieTrait htt_t>
	struct SingleEntryNode;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct CartesianNode;

} // namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_NODETYPES_PREDECLARE_HPP
