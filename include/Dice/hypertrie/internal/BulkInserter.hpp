#ifndef HYPERTRIE_BULKINSERTER_HPP
#define HYPERTRIE_BULKINSERTER_HPP

#include "Dice/hypertrie/internal/Hypertrie.hpp"

namespace hypertrie {

	template<HypertrieTrait tr_t>
	class BulkInserter {
	public:
		using tr = tr_t;
		using tri = internal::raw::Hypertrie_internal_t<tr>;
		using Entry = typename tr::IteratorEntry;
		using Key = typename tr::Key;
		using value_type = typename tr::value_type;
		using EntryFunctions = typename tr::iterator_entry;

	private:
		using collection_type = std::conditional_t<(tr::is_bool_valued),
												   tsl::sparse_set<Key, absl::Hash<Key>>,
												   tsl::sparse_map<Key, value_type, absl::Hash<Key>>>;

		Hypertrie<tr> *hypertrie;

		collection_type new_entries;
		size_t threshold = 1'000'000;

	public:
		BulkInserter(Hypertrie<tr> &hypertrie, size_t threshold = 1'000'000) : hypertrie(&hypertrie), threshold(threshold) {
			if constexpr (not tr::is_bool_valued) {
				throw std::logic_error("Bulk loading is only supported for bool-valued Hypertries yet.");
			}
		}

		~BulkInserter() {
			flush();
		}

		void add(Entry &&entry) {
			assert(EntryFunctions::key(entry).size() == hypertrie->depth());
			if ((*hypertrie)[EntryFunctions::key(entry)] == value_type{}) {
				new_entries.insert(std::forward<Entry>(entry));
				if (new_entries.size() > threshold)
					flush();
			}
		}

		void flush() {

			internal::compiled_switch<hypertrie_depth_limit, 1>::switch_void(
					hypertrie->depth(),
					[&](auto depth_arg) {
						using RawKey = typename tri::template RawKey<depth_arg>;
						std::vector<RawKey> keys(new_entries.size());
						for (auto [i, entry] : iter::enumerate(new_entries)) {
							RawKey &raw_key = keys[i];
							for (auto i : iter::range(size_t(depth_arg)))
								raw_key[i] = entry[i];// todo: add support for non-boolean
						}

						new_entries.clear();
						auto &typed_nodec = *reinterpret_cast<internal::raw::NodeContainer<depth_arg, tri> *>(const_cast<hypertrie::internal::raw::RawNodeContainer *>(hypertrie->rawNodeContainer()));
						hypertrie->context()->rawContext().template bulk_insert<depth_arg>(typed_nodec, std::move(keys));
					});
		}
	};
}// namespace hypertrie


#endif//HYPERTRIE_BULKINSERTER_HPP
