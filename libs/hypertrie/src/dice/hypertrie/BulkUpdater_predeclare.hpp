#ifndef HYPERTRIE_BULKUPDATER_PREDECLARE_HPP
#define HYPERTRIE_BULKUPDATER_PREDECLARE_HPP

#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node_context/BulkUpdaterSettings.hpp"

namespace dice::hypertrie {

	using internal::raw::BulkUpdaterMode;
	using internal::raw::BulkUpdaterSyncness;

	/**
	 * Bulk updater for a hypertrie.
	 * @tparam mode if inserting or deleting entries
	 * @tparam tr_t Hypertrie trait that must be boolean valued.
	 * @tparam allocator_type The allocator used by the hypertrie.
	 * @tparam syncness If queueing entries for insertion and executing the insertion happen on the same thread.
	 */
	template<BulkUpdaterMode mode, HypertrieTrait_bool_valued tr_t, ByteAllocator allocator_type, BulkUpdaterSyncness syncness>
	class BulkUpdater;

	template<HypertrieTrait_bool_valued tr_t, ByteAllocator allocator_type, BulkUpdaterSyncness syncness>
	using BulkInserter = BulkUpdater<BulkUpdaterMode::Insert, tr_t, allocator_type, syncness>;

	template<HypertrieTrait_bool_valued tr_t, ByteAllocator allocator_type, BulkUpdaterSyncness syncness>
	using BulkRemover = BulkUpdater<BulkUpdaterMode::Remove, tr_t, allocator_type, syncness>;

	template<HypertrieTrait_bool_valued tr_t, ByteAllocator allocator_type>
	using SyncBulkInserter = BulkInserter<tr_t, allocator_type, BulkUpdaterSyncness::Sync>;

	template<HypertrieTrait_bool_valued tr_t, ByteAllocator allocator_type>
	using SyncBulkRemover = BulkRemover<tr_t, allocator_type, BulkUpdaterSyncness::Sync>;

	template<HypertrieTrait_bool_valued tr_t, ByteAllocator allocator_type>
	using AsyncBulkInserter = BulkInserter<tr_t, allocator_type, BulkUpdaterSyncness::Async>;

	template<HypertrieTrait_bool_valued tr_t, ByteAllocator allocator_type>
	using AsyncBulkRemover = BulkRemover<tr_t, allocator_type, BulkUpdaterSyncness::Async>;

}// namespace dice::hypertrie


#endif//HYPERTRIE_BULKUPDATER_PREDECLARE_HPP
