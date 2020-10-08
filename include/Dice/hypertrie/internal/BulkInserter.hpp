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
				tsl::sparse_set<Key, absl::Hash<Key>>,
				tsl::sparse_map<Key, value_type, absl::Hash<Key>>>;

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
			flush();
			if (insertion_thread.joinable())
				insertion_thread.join();
		}

		template<typename T>
		void add(T &&entry) {
			assert(EntryFunctions::key(entry).size() == hypertrie->depth());
			if ((*hypertrie)[EntryFunctions::key(entry)] == value_type{}) {
				new_entries.insert(std::forward<T>(entry));
				if (threshold != 0 and new_entries.size() > threshold)
					flush();
			}
		}

		void flush() {
			if (insertion_thread.joinable())
				insertion_thread.join();
			internal::compiled_switch<hypertrie_depth_limit, 1>::switch_void(
					hypertrie->depth(),
					[&](auto depth_arg) {
					  this->load_entries = std::move(this->new_entries);
					  this->new_entries = {};
					  if (load_entries.size() > 0)
						  insertion_thread = std::thread([&]() {
							using RawKey = typename tri::template RawKey<depth_arg>;
							std::vector<RawKey> keys(load_entries.size());
							for (auto &&[raw_key, entry] : iter::zip(keys, load_entries))
								for (auto j : iter::range(size_t(depth_arg)))
									raw_key[j] = entry[j];// todo: add support for non-boolean

							auto &typed_nodec = *reinterpret_cast<internal::raw::NodeContainer<depth_arg, tri> *>(const_cast<hypertrie::internal::raw::RawNodeContainer *>(hypertrie->rawNodeContainer()));
							hypertrie->context()->rawContext().template bulk_insert<depth_arg>(typed_nodec, std::move(keys));
						  });
					});
		}

		[[nodiscard]] size_t size() const {
			return new_entries.size();
		}
	};
}// namespace hypertrie


#endif//HYPERTRIE_BULKINSERTER_HPP
