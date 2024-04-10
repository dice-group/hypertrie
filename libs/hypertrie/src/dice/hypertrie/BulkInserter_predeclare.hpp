#ifndef HYPERTRIE_BULKINSERTER_PREDECLARE_HPP
#define HYPERTRIE_BULKINSERTER_PREDECLARE_HPP

#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"

namespace dice::hypertrie {

	/**
	 * Bulkinserter for a hypertrie.
	 * @tparam tr_t Hypertrie trait that must be boolean valued.
	 * @tparam allocator_type The allocator used by the hypertrie.
	 * @tparam threaded If queueing entries for insertion and executing the insertion happen on the same thread.
	 */
	template<HypertrieTrait_bool_valued tr_t, ByteAllocator allocator_type, bool threaded = true>
	class BulkInserter;
}// namespace dice::hypertrie


#endif//HYPERTRIE_BULKINSERTER_PREDECLARE_HPP
