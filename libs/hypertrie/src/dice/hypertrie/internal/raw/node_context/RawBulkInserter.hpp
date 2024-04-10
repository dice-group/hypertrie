#ifndef HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP
#define HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP

#include "dice/hypertrie/internal/raw/node/NodeContainer.hpp"
#include "dice/hypertrie/internal/raw/node_context/BulkInserter_callback.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"
#include "dice/hypertrie/internal/util/folly_ProducerConsumerQueue.hpp"

#include <robin_hood.h>

#include <functional>
#include <thread>
#include <utility>

namespace dice::hypertrie::internal::raw {


	template<size_t depth, HypertrieTrait_bool_valued htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	class RawHypertrieBulkInserter {
		// TODO: extend to non-Boolean valued hypertries
		// TODO: extend to removing entries
		// TODO: extend to change values (only non-Boolean valued hypertries)
	public:
		using Entry = SingleEntry<depth, htt_t>;
		using key_part_type = typename htt_t::key_part_type;

	private:
		folly_standalone::ProducerConsumerQueue<Entry> entry_queue_;
		uint32_t const bulk_size_; // this is must be exacty here for memory alignment
		RawNodeContainer<htt_t, allocator_type> *nodec_;
		RawHypertrieContext<context_max_depth, htt_t, allocator_type> *context_;
		std::unique_ptr<std::jthread> check_and_insertion_thread_;
		std::vector<Entry> new_entries_;// buffer_size
		BulkInserter_bulk_loaded_callback get_stats_;
		std::atomic<bool> please_flush_ = false;

	public:
		/**
		 * @param nodec
		 * @param context
		 * @param bulk_size
		 * @param get_stats see BulkInserter_bulk_loaded_callback
		 */
		RawHypertrieBulkInserter(
				RawNodeContainer<htt_t, allocator_type> &nodec,
				RawHypertrieContext<context_max_depth, htt_t, allocator_type> &context,
				uint32_t bulk_size = 1'000'000U,
				BulkInserter_bulk_loaded_callback get_stats = [](auto...) {}) noexcept
			: entry_queue_{(bulk_size > 2) ? bulk_size : uint32_t(2)},
			  bulk_size_((bulk_size > 2) ? bulk_size : uint32_t(2)),
			  nodec_(&nodec),
			  context_(&context), get_stats_(std::move(get_stats)) {

			new_entries_.reserve(bulk_size_ + 1);
			check_and_insertion_thread_ =
					std::make_unique<std::jthread>([&](std::stop_token const &stoken) {
						size_t const deduplication_max_size = 4UL * bulk_size_;
						bool done = false;
						Entry entry;
						while (not done) {
							::robin_hood::unordered_set<RawIdentifier<depth, htt_t>> de_duplication(bulk_size_ + 1);
							size_t no_seen_entries = 0;

							while (new_entries_.size() < bulk_size_ and not please_flush_.load()) {
								if (entry_queue_.read(entry)) {
									no_seen_entries++;
									RawIdentifier<depth, htt_t> id{entry};
									const auto &[_, is_new] = de_duplication.insert(id);

									if (de_duplication.size() == deduplication_max_size) {
										de_duplication.clear();

										// reinsert all entries already chosen for the current bulk
										// to prevent duplicates in current bulk
										for (auto const &e : new_entries_) {
											de_duplication.insert(RawIdentifier<depth, htt_t>{e});
										}
									}

									if (is_new)
										if (not context_->template get<depth>(NodeContainer<depth, htt_t, allocator_type>{*nodec_}, entry.key()))
											new_entries_.push_back(entry);
								} else if (stoken.stop_requested()) {
									done = true;
									break;
								}
							}
							NodeContainer<depth, htt_t, allocator_type> nodec{*nodec_};
							context_->insert(nodec, new_entries_);
							*nodec_ = nodec;
							get_stats_(no_seen_entries, new_entries_.size(), context_->size(nodec));
							new_entries_.clear();
							please_flush_.store(false);
						}
					});
		}

		~RawHypertrieBulkInserter() {
			check_and_insertion_thread_->request_stop();
			if (check_and_insertion_thread_->joinable()) [[likely]]
				check_and_insertion_thread_->join();
		}

		void add(Entry const &entry) noexcept {
			while (true) {
				bool push_succeeded = entry_queue_.write(entry);
				if (push_succeeded) [[likely]]
					return;
			}
		}

		void add(NonZeroEntry<htt_t> const &entry) {
			if (entry.size() == depth) [[likely]] {
				Entry raw_entry;
				std::copy(entry.key().begin(), entry.key().end(),
						  raw_entry.key().begin());
				add(raw_entry);
			} else [[unlikely]] {
				throw std::logic_error{
						"The provided NonZeroEntry has a wrong depth/size."};
			}
		}

		[[nodiscard]] size_t size() const noexcept { return new_entries_.size(); }

		void flush() {
			please_flush_.store(true);
			while (please_flush_.load())
				;
		}
	};
}// namespace dice::hypertrie::internal::raw

#endif// HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP
