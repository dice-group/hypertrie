#ifndef HYPERTRIE_BULKINSERTER_CALLBACK_HPP
#define HYPERTRIE_BULKINSERTER_CALLBACK_HPP

#include <cstddef>
#include <functional>

namespace dice::hypertrie::internal::raw {

	/**
	 * A Callback that is invoked after a bulk was loaded.
	 * It writes to the passed arguments the number of entries processed for this bulk, how many unique new entries were inserted and how many entries the hypertrie contains now after insertion.
	 * @param processed_entries
	 * @param inserted_entries
	 * @param hypertrie_size_after
	 */
	using BulkInserter_bulk_loaded_callback = std::function<void(size_t processed_entries, size_t inserted_entries, size_t hypertrie_size_after)>;

}// namespace dice::hypertrie::internal::raw
#endif//HYPERTRIE_BULKINSERTER_CALLBACK_HPP
