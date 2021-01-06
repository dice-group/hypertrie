#ifndef HYPERTRIE_BULKINSERTER_HPP
#define HYPERTRIE_BULKINSERTER_HPP

#include <thread>

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

	protected:
		using collection_type = std::conditional_t<(tr::is_bool_valued),
				tsl::sparse_set<Key, Dice::hash::DiceHash<Key>>,
		tsl::sparse_map<Key, value_type, Dice::hash::DiceHash<Key>>>;

		Hypertrie<tr> *hypertrie;

		collection_type new_entries;
		collection_type load_entries;
		size_t threshold = 1'000'000;
		std::thread insertion_thread;

	public:
		BulkInserter(Hypertrie<tr> &hypertrie, size_t threshold = 1'000'000) : hypertrie(&hypertrie), threshold(threshold) {
			if constexpr (not tr::is_bool_valued) {
				throw std::logic_error("Bulk loading is only supported for bool-valued Hypertries yet.");
			}
		}

		~BulkInserter() {
			flush(true);
		}

		template<typename... Key_Parts>
		void add(Key_Parts &&...key_parts) {
			add<Entry>(
					Entry{std::forward<typename tr::key_part_type>(key_parts)...});
		}

		void add(Entry &&entry) {
			add<Entry>(std::forward<Entry>(entry));
		}

		template<typename T>
		void add(T &&entry) {
			assert(EntryFunctions::key(entry).size() == hypertrie->depth());
			hypertrie->set(tr::iterator_entry::key(entry), tr::iterator_entry::value(entry));
		}

		void flush([[maybe_unused]] const bool blocking = false) {

		}

		[[nodiscard]] size_t size() const {
			return new_entries.size();
		}
	};
}// namespace hypertrie


#endif//HYPERTRIE_BULKINSERTER_HPP
