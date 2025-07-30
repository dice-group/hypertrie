#ifndef HYPERTRIE_BULKUPDATER_CALLBACK_HPP
#define HYPERTRIE_BULKUPDATER_CALLBACK_HPP

#include <cstddef>
#include <functional>

namespace dice::hypertrie::internal::raw {

	/**
	 * A Callback that is invoked after a bulk was processed.
	 * It writes to the passed arguments the number of entries processed for this bulk, how many unique new entries were
	 * inserted/removed and how many entries the hypertrie contains now after insertion/removal.
	 * @param processed_entries
	 * @param committed_entries
	 * @param hypertrie_size_after
	 */
	using BulkUpdater_bulk_processed_callback = std::function<void(size_t processed_entries, size_t committed_entries, size_t hypertrie_size_after)>;

}// namespace dice::hypertrie::internal::raw
#endif//HYPERTRIE_BULKUPDATER_CALLBACK_HPP
