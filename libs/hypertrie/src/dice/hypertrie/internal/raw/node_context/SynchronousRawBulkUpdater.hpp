#ifndef HYPERTRIE_SYNCHRONOUSRAWHYPERTRIEBULKINSERTER_HPP
#define HYPERTRIE_SYNCHRONOUSRAWHYPERTRIEBULKINSERTER_HPP


#include "dice/hypertrie/internal/raw/node/NodePtr.hpp"
#include "dice/hypertrie/internal/raw/node_context/BulkUpdaterSettings.hpp"
#include "dice/hypertrie/internal/raw/node_context/BulkUpdater_callback.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"

namespace dice::hypertrie::internal::raw {

	/**
	 * @note This does not support changing values yet.
	 *      On insert:
	 *          - will ignore entries if their key already exists in the hypertrie
	 *          - for each given key will insert the first value it encounters
	 */
	template<BulkUpdaterMode mode, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	class SynchronousRawHypertrieBulkUpdater {
		// TODO: extend to change values (only non-Boolean valued hypertries)
	public:
		using RawEntry = std::conditional_t<mode == BulkUpdaterMode::Insert, SingleEntry<depth, htt_t>, RawKey<depth, htt_t>>;
		using Entry = std::conditional_t<mode == BulkUpdaterMode::Insert, NonZeroEntry<htt_t>, Key<htt_t>>;
		using AltEntry = std::conditional_t<mode == BulkUpdaterMode::Insert, ::dice::hypertrie::Entry<htt_t>, Key<htt_t>>;
		using key_part_type = typename htt_t::key_part_type;

	private:
		uint32_t bulk_size_;
		size_t const deduplication_max_size_;
		NodePtr<depth, htt_t, allocator_type> *node_ptr_;
		RawHypertrieContext<context_max_depth, htt_t, allocator_type> *context_;
		std::vector<SingleEntry<depth, htt_t>> new_entries_;// buffer_size
		BulkUpdater_bulk_processed_callback get_stats_;
		node_context::common_detail::Set<size_t> de_duplication_;
		size_t no_seen_entries = 0;

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
		SynchronousRawHypertrieBulkUpdater(NodePtr<depth, htt_t, allocator_type> &node_ptr,
										   RawHypertrieContext<context_max_depth, htt_t, allocator_type> &context,
										   uint32_t bulk_size = 1'000'000U,
										   BulkUpdater_bulk_processed_callback get_stats = [](auto...) {}) noexcept : bulk_size_{bulk_size == 0 ? 1 : bulk_size},
																						   							  deduplication_max_size_{4UL * bulk_size_},
																						   							  node_ptr_{&node_ptr},
																						   							  context_{&context},
																						   							  get_stats_{std::move(get_stats)},
																						   							  de_duplication_{bulk_size_ + 1} {
			new_entries_.reserve(bulk_size_);
		}

		SynchronousRawHypertrieBulkUpdater(SynchronousRawHypertrieBulkUpdater &&other) noexcept = default;

		SynchronousRawHypertrieBulkUpdater(SynchronousRawHypertrieBulkUpdater const &other) = delete;
		SynchronousRawHypertrieBulkUpdater &operator=(SynchronousRawHypertrieBulkUpdater const &other) = delete;
		SynchronousRawHypertrieBulkUpdater &operator=(SynchronousRawHypertrieBulkUpdater &&other) = delete;

		~SynchronousRawHypertrieBulkUpdater() {
			flush();
		}

		void add(RawEntry const &entry) noexcept(mode == BulkUpdaterMode::Remove || HypertrieTrait_bool_valued<htt_t>) {
			if constexpr (mode == BulkUpdaterMode::Insert && !HypertrieTrait_bool_valued<htt_t>) {
				if (entry.value() == typename htt_t::value_type{}) [[unlikely]] {
					throw std::logic_error{"SynchronousRawHypertrieBulkUpdater::add: Cannot insert or remove a zero-valued entry"};
				}
			}

			if (auto const &[_, is_new] = de_duplication_.insert(hash(entry)); !is_new) {
				return;
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

			if (de_duplication_.size() == deduplication_max_size_) {
				de_duplication_.clear();

				// reinsert all entries already chosen for the current bulk
				// to prevent duplicates in current bulk
				for (auto const &e : new_entries_) {
					de_duplication_.insert(hash(e));
				}
			}

			if (new_entries_.size() >= bulk_size_) {
				flush();
			}
		}

		void add(Entry const &entry) {
			if (entry.size() != depth) [[unlikely]] {
				throw std::logic_error{"SynchronousRawHypertrieBulkUpdater::add: The provided NonZeroEntry has a wrong depth/size."};
			}

			add(to_raw(entry));
		}

		void add(AltEntry const &entry) requires (mode == BulkUpdaterMode::Insert) {
			if (entry.size() != depth) [[unlikely]] {
				throw std::logic_error{"SynchronousRawHypertrieBulkUpdater::add: The provided Entry has a wrong depth/size."};
			}

			if (entry.value() == typename htt_t::value_type{}) [[unlikely]] {
				return;
			}

			add(to_raw(entry));
		}

		[[nodiscard]] size_t size() const noexcept { return new_entries_.size(); }

		void flush() {
			if (new_entries_.empty()) {
				return;
			}

			auto const change_size = new_entries_.size();

			if constexpr (mode == BulkUpdaterMode::Insert) {
				context_->insert(*node_ptr_, std::move(new_entries_));
			} else /* mode == BulkUpdaterMode::Remove */ {
				context_->remove(*node_ptr_, std::move(new_entries_));
			}

			new_entries_ = std::vector<SingleEntry<depth, htt_t>>{};
			new_entries_.reserve(bulk_size_);

			get_stats_(no_seen_entries, change_size, context_->size(*node_ptr_));
		}
	};

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	using SynchronousRawHypertrieBulkInserter = SynchronousRawHypertrieBulkUpdater<BulkUpdaterMode::Insert, depth, htt_t, allocator_type, context_max_depth>;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	using SynchronousRawHypertrieBulkRemover = SynchronousRawHypertrieBulkUpdater<BulkUpdaterMode::Remove, depth, htt_t, allocator_type, context_max_depth>;

}// namespace dice::hypertrie::internal::raw

#endif// HYPERTRIE_SYNCHRONOUSRAWHYPERTRIEBULKINSERTER_HPP
