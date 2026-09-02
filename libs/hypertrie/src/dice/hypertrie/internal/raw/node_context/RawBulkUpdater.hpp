#ifndef HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP
#define HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP

#include "dice/hypertrie/internal/raw/node/NodePtr.hpp"
#include "dice/hypertrie/internal/raw/node_context/BulkUpdaterSettings.hpp"
#include "dice/hypertrie/internal/raw/node_context/BulkUpdater_callback.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"
#include "dice/hypertrie/internal/util/folly_ProducerConsumerQueue.hpp"

#include <atomic>
#include <thread>

namespace dice::hypertrie::internal::raw {


	/**
	 * @note This does not support changing values yet.
	 *      On insert:
	 *          - will ignore entries if their key already exists in the hypertrie
	 *          - for each given key will insert the first value it encounters
	 */
	template<BulkUpdaterMode mode, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	class RawHypertrieBulkUpdater {
		// TODO: extend to change values (only non-Boolean valued hypertries)
	public:
		using RawEntry = std::conditional_t<mode == BulkUpdaterMode::Insert, SingleEntry<depth, htt_t>, RawKey<depth, htt_t>>;
		using Entry = std::conditional_t<mode == BulkUpdaterMode::Insert, NonZeroEntry<htt_t>, Key<htt_t>>;
		using AltEntry = std::conditional_t<mode == BulkUpdaterMode::Insert, ::dice::hypertrie::Entry<htt_t>, Key<htt_t>>;
		using key_part_type = typename htt_t::key_part_type;

	private:
		folly_standalone::ProducerConsumerQueue<RawEntry> entry_queue_;
		uint32_t const bulk_size_; // this is must be exacty here for memory alignment
		NodePtr<depth, htt_t, allocator_type> *node_ptr_;
		RawHypertrieContext<context_max_depth, htt_t, allocator_type> *context_;
		std::unique_ptr<std::jthread> check_and_insertion_thread_;
		std::vector<SingleEntry<depth, htt_t>> new_entries_;// buffer_size
		BulkUpdater_bulk_processed_callback get_stats_;
		std::atomic_flag please_flush_ = false;

		[[nodiscard]] inline static size_t hash(SingleEntry<depth, htt_t> const &e) noexcept {
			return hash::dice_hash_templates<hash::Policies::wyhash>::dice_hash(e.key());
		}

		[[nodiscard]] inline static size_t hash(RawKey<depth, htt_t> const &k) noexcept {
			return hash::dice_hash_templates<hash::Policies::wyhash>::dice_hash(k);
		}

		template<typename E>
		[[nodiscard]] inline static RawEntry to_raw(E const &entry) noexcept {
			RawEntry raw_entry;

			if constexpr (mode == BulkUpdaterMode::Insert) {
				std::copy(entry.key().begin(), entry.key().end(), raw_entry.key().begin());

				if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
					raw_entry.set_value(entry.value());
				}
			} else /* mode == BulkUpdaterMode::Remove */ {
				std::copy(entry.begin(), entry.end(), raw_entry.begin());
			}

			return raw_entry;
		}

	public:
		/**
		 * @param nodec
		 * @param context
		 * @param bulk_size
		 * @param get_stats see BulkUpdater_bulk_processed_callback
		 */
		RawHypertrieBulkUpdater(NodePtr<depth, htt_t, allocator_type> &node_ptr,
								RawHypertrieContext<context_max_depth, htt_t, allocator_type> &context,
								uint32_t bulk_size = 1'000'000U,
								BulkUpdater_bulk_processed_callback get_stats = [](auto...) {}) noexcept : entry_queue_{(bulk_size > 2) ? bulk_size : uint32_t(2)},
																						   				   bulk_size_{(bulk_size > 2) ? bulk_size : uint32_t(2)},
																						   				   node_ptr_{&node_ptr},
																						   				   context_{&context},
																						   				   get_stats_{std::move(get_stats)} {

			new_entries_.reserve(bulk_size_ + 1);
			check_and_insertion_thread_ = std::make_unique<std::jthread>([&](std::stop_token const &stoken) {
				size_t const deduplication_max_size = 4UL * bulk_size_;
				bool done = false;
				RawEntry entry;

				while (not done) {
					new_entries_.reserve(bulk_size_ + 1);

					node_context::common_detail::Set<size_t> de_duplication(bulk_size_ + 1);
					size_t no_seen_entries = 0;

					while (new_entries_.size() < bulk_size_ and not please_flush_.test(std::memory_order_acquire)) {
						if (entry_queue_.read(entry)) {
							no_seen_entries += 1;

							if (auto const &[_, is_new] = de_duplication.insert(hash(entry)); !is_new) {
								continue;
							}

							if constexpr (mode == BulkUpdaterMode::Insert) {
								auto const old_value = context_->template get<depth>(*node_ptr_, entry.key());
								if (old_value == typename htt_t::value_type{}) {
									new_entries_.push_back(entry);
								}
							} else /* mode == BulkUpdaterMode::Remove */ {
								auto const old_value = context_->template get<depth>(*node_ptr_, entry);
								if (old_value != typename htt_t::value_type{}) {
									new_entries_.push_back(SingleEntry<depth, htt_t>{entry, old_value});
								}
							}

							if (de_duplication.size() == deduplication_max_size) {
								de_duplication.clear();

								// reinsert all entries already chosen for the current bulk
								// to prevent duplicates in current bulk
								for (auto const &e : new_entries_) {
									de_duplication.insert(hash(e));
								}
							}
						} else if (stoken.stop_requested()) {
							done = true;
							break;
						}
					}

					auto const change_size = new_entries_.size();

					if constexpr (mode == BulkUpdaterMode::Insert) {
						context_->insert(*node_ptr_, std::move(new_entries_));
					} else /* mode == BulkUpdaterMode::Remove */ {
						context_->remove(*node_ptr_, std::move(new_entries_));
					}

					new_entries_ = std::vector<SingleEntry<depth, htt_t>>{};
					get_stats_(no_seen_entries, change_size, context_->size(*node_ptr_));
					please_flush_.clear(std::memory_order_release);
					please_flush_.notify_one();
				}
			});
		}

		RawHypertrieBulkUpdater(RawHypertrieBulkUpdater const &other) = delete;
		RawHypertrieBulkUpdater(RawHypertrieBulkUpdater &&other) = delete;
		RawHypertrieBulkUpdater &operator=(RawHypertrieBulkUpdater const &other) = delete;
		RawHypertrieBulkUpdater &operator=(RawHypertrieBulkUpdater &&other) = delete;

		~RawHypertrieBulkUpdater() noexcept {
			check_and_insertion_thread_->request_stop();
			if (check_and_insertion_thread_->joinable()) [[likely]] {
				check_and_insertion_thread_->join();
			}
		}

		void add(RawEntry const &entry) noexcept(mode == BulkUpdaterMode::Remove || HypertrieTrait_bool_valued<htt_t>) {
			if constexpr (mode == BulkUpdaterMode::Insert && !HypertrieTrait_bool_valued<htt_t>) {
				if (entry.value() == typename htt_t::value_type{}) [[unlikely]] {
					throw std::logic_error{"RawHypertrieBulkUpdater::add: Cannot insert a zero-valued entry"};
				}
			}

			while (true) {
				if (entry_queue_.write(entry)) [[likely]] {
					return;
				}
			}
		}

		void add(Entry const &entry) {
			if (entry.size() != depth) [[unlikely]] {
				throw std::logic_error{"RawHypertrieBulkUpdater::add: The provided NonZeroEntry has a wrong depth/size."};
			}

			add(to_raw(entry));
		}

		void add(AltEntry const &entry) requires (mode == BulkUpdaterMode::Insert) {
			if (entry.size() != depth) [[unlikely]] {
				throw std::logic_error{"RawHypertrieBulkUpdater::add: The provided Entry has a wrong depth/size."};
			}

			if (entry.value() == typename htt_t::value_type{}) [[unlikely]] {
				return;
			}

			add(to_raw(entry));
		}

		[[nodiscard]] size_t size() const noexcept {
			return new_entries_.size();
		}

		void flush() {
			please_flush_.test_and_set(std::memory_order_release);
			please_flush_.wait(true);
		}
	};

	template<size_t depth, HypertrieTrait_bool_valued htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	using RawHypertrieBulkInserter = RawHypertrieBulkUpdater<BulkUpdaterMode::Insert, depth, htt_t, allocator_type, context_max_depth>;

	template<size_t depth, HypertrieTrait_bool_valued htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	using RawHypertrieBulkRemover = RawHypertrieBulkUpdater<BulkUpdaterMode::Remove, depth, htt_t, allocator_type, context_max_depth>;

}// namespace dice::hypertrie::internal::raw

#endif// HYPERTRIE_RAWHYPERTRIEBULKINSERTER_HPP
