#ifndef HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP
#define HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP

#include <Dice/hypertrie/internal/raw/node/NodeContainer.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <boost/lockfree/policies.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <tsl/sparse_set.h>

#include <functional>
#include <thread>

namespace Dice::hypertrie::internal::raw {

	/**
	 * A Callback that is invoked after a bulk was loaded.
	 * It writes to the passed arguments the number of entries processed for this bulk, how many unique new entries were inserted and how many entries the hypertrie contains now after insertion.
	 * @param processed_entries
	 * @param inserted_entries
	 * @param hypertrie_size_after
	 */
	using BulkInserter_bulk_loaded_callback = std::function<void(size_t processed_entries, size_t inserted_entries, size_t hypertrie_size_after)>;

	template<size_t depth, HypertrieCoreTrait_bool_valued tri_t, size_t context_max_depth>
	class RawHypertrieBulkInserter {
		// TODO: extend to non-Boolean valued hypertries
		// TODO: extend to removing entries
		// TODO: extend to change values (only non-Boolean valued hypertries)
	public:
		using tri = tri_t;
		using tr = typename tri::tr;
		using Entry = SingleEntry<depth, tri_with_stl_alloc<tri>>;

	private:
		// TODO: there are faster queues than this one
		// TODO: compare with previous approach
		// TODO: move to own submodule (because it has dependencies)
		boost::lockfree::spsc_queue<Entry> entry_queue_;
		NodeContainer<depth, tri> *nodec_;
		RawHypertrieContext<context_max_depth, tri> *context_;
		std::jthread check_and_insertion_thread_;
		std::vector<Entry> new_entries_;// buffer_size
		size_t bulk_size_;
		BulkInserter_bulk_loaded_callback get_stats_;

	public:
		/**
		 *
		 * @param nodec
		 * @param context
		 * @param bulk_size
		 * @param get_stats see BulkInserter_bulk_loaded_callback
		 */
		RawHypertrieBulkInserter(
				NodeContainer<depth, tri> &nodec,
				RawHypertrieContext<context_max_depth, tri> &context,
				size_t bulk_size = 1'000'000,
				BulkInserter_bulk_loaded_callback get_stats = []([[maybe_unused]] size_t processed_entries,
																 [[maybe_unused]] size_t inserted_entries,
																 [[maybe_unused]] size_t hypertrie_size_after) {})
			: entry_queue_(bulk_size), nodec_(&nodec), context_(&context), bulk_size_(bulk_size), get_stats_(get_stats) {
			if (bulk_size_ == 0)
				bulk_size_ = 1;
			new_entries_.reserve(bulk_size_);
			check_and_insertion_thread_ = std::jthread([&](std::stop_token stoken) {
				do {
					tsl::sparse_set<RawIdentifier<depth, tri>> de_duplication;
					size_t no_seen_entries = 0;

					while (new_entries_.size() < bulk_size_ and
						   not(entry_queue_.read_available() == 0 and stoken.stop_requested())) {
						entry_queue_.consume_all([&](auto const &entry) {
							no_seen_entries++;
							RawIdentifier<depth, tri> id{entry};
							const auto &[_, is_new] = de_duplication.insert(id);
							if (is_new)
								if (not context_->template get<depth>(*nodec_, entry.key()))
									new_entries_.push_back(entry);
						});
					}
					context_->template insert(*nodec_, new_entries_);
					get_stats_(no_seen_entries, new_entries_.size(), context_->template size(nodec));
					new_entries_.clear();
				} while (not(entry_queue_.read_available() == 0 and stoken.stop_requested()));
			});
		}

		~RawHypertrieBulkInserter() {
			check_and_insertion_thread_.request_stop();
			if (check_and_insertion_thread_.joinable()) [[likely]]
				check_and_insertion_thread_.join();
		}

		void add(Entry const &entry) noexcept {
			while (true) {
				bool push_succeeded = entry_queue_.push(entry);
				if (push_succeeded) [[likely]]
					return;
			}
		}

		void add(NonZeroEntry<tr> const &entry) {
			if (entry.size() == depth) [[likely]] {
				Entry raw_entry;
				std::copy(entry.key().begin(), entry.key().end(), raw_entry.key().begin());
				add(raw_entry);
			} else [[unlikely]] {
				throw std::logic_error{"The provided NonZeroEntry has a wrong depth/size."};
			}
		}

		[[nodiscard]] size_t size() const noexcept {
			return new_entries_.size();
		}
	};
}// namespace Dice::hypertrie::internal::raw

#endif//HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP
