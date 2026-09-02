#ifndef HYPERTRIE_BULKUPDATER_SETTINGS_HPP
#define HYPERTRIE_BULKUPDATER_SETTINGS_HPP

namespace dice::hypertrie::internal::raw {

	enum struct BulkUpdaterMode : bool {
		Insert,
		Remove
	};

	enum struct BulkUpdaterSyncness : bool {
		Sync,
		Async,
	};

} // namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_BULKUPDATER_SETTINGS_HPP
