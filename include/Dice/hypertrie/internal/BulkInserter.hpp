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
			if constexpr (not tr::is_bool_valued){
				throw std::logic_error("Bulk loading is only supported for bool-valued Hypertries yet.");
			}
		}

		void add(Entry&& entry) {
			assert(EntryFunctions::key(entry).size() == hypertrie->depth());
			if ((*hypertrie)[EntryFunctions::key(entry)] == value_type{})
				new_entries.insert(std::forward<Entry>(entry));
		}

		void flush() {

			std::vector<Entry> keys(new_entries.size());
			for (auto [i, entry] : iter::enumerate(new_entries)){
				keys[i] = std::move(entry);
			}

			new_entries.clear();

			internal::compiled_switch<hypertrie_depth_limit, 1>::switch_void(
					this->depth_,
					[&](auto depth_arg){
					  auto &typed_nodec = *reinterpret_cast<internal::raw::NodeContainer<depth_arg, tri> *>(hypertrie->rawNodeContainer());
					  hypertrie->context()->rawContext().bulk_insert(typed_nodec, std::move(keys));
					}
			);

		}
	};
}


#endif//HYPERTRIE_BULKINSERTER_HPP
