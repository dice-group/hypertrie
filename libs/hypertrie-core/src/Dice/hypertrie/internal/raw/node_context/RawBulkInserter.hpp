#ifndef HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP
#define HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP

#include <Dice/hypertrie/internal/raw/node/NodeContainer.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <boost/lockfree/policies.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <tsl/sparse_set.h>

#include <thread>

namespace hypertrie::internal::raw {


	template<size_t depth, HypertrieCoreTrait_bool_valued tri_t, size_t context_max_depth>
	class RawHypertrieBulkInserter {
		// TODO: extend to non-Boolean valued hypertries
		// TODO: extend to removing entries
		// TODO: extend to change values (only non-Boolean valued hypertries)
	public:
		using tri = tri_t;
		using tr = typename tri::tr;
		using Entry = SingleEntry<depth, tri_with_stl_alloc<tri>>;
		static constexpr size_t buffer_size = 1'000'000;

	private:
		// TODO: there are faster queues than this one
		// TODO: compare with previous approach
		boost::lockfree::spsc_queue<Entry> entry_queue_;
		NodeContainer<depth, tri> *nodec_;
		RawHypertrieContext<context_max_depth, tri> *context_;
		std::jthread check_and_insertion_thread_;
		std::vector<Entry> new_entries_;// buffer_size

		void start_check_thread() {
			new_entries_.reserve(buffer_size);
			check_and_insertion_thread_ = std::jthread([&](std::stop_token stoken) {
				do {
					tsl::sparse_set<RawIdentifier<depth, tri>> de_duplication;

					while (new_entries_.size() < buffer_size and
						   not(entry_queue_.read_available() == 0 and stoken.stop_requested())) {
						entry_queue_.consume_all([&](auto const &entry) {
							RawIdentifier<depth, tri> id{entry};
							const auto &[_, is_new] = de_duplication.insert(id);
							if (is_new)
								if (not context_->template get<depth>(*nodec_, entry.key()))
									new_entries_.push_back(entry);
						});
					}
					std::cout << "insertion starts ..." << std::flush;
					context_->template insert(*nodec_, new_entries_);
					std::cout << " and is done. " << std::endl;
					new_entries_.clear();
				} while (not(entry_queue_.read_available() == 0 and stoken.stop_requested()));
			});
		}

	public:
		~RawHypertrieBulkInserter() {
			check_and_insertion_thread_.request_stop();
			if (check_and_insertion_thread_.joinable())
				check_and_insertion_thread_.join();
		}

		RawHypertrieBulkInserter(NodeContainer<depth, tri> &nodec,
								 RawHypertrieContext<context_max_depth, tri> &context)
			: entry_queue_(buffer_size), nodec_(&nodec), context_(&context) {
			start_check_thread();
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
				Entry raw_entry(RawKey<depth, tri>{entry.key().begin(), entry.key().end()});
				add(raw_entry);
			} else [[unlikely]] {
				throw std::logic_error{"The provided NonZeroEntry has a wrong depth/size."};
			}
		}

		[[nodiscard]] size_t size() const noexcept {
			return new_entries_.size();
		}
	};
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP
