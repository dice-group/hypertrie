#ifndef HYPERTRIE_SYNCHRONOUSRAWHYPERTRIEBULKINSERTER_HPP
#define HYPERTRIE_SYNCHRONOUSRAWHYPERTRIEBULKINSERTER_HPP


#include "dice/hypertrie/internal/raw/node/NodeContainer.hpp"
#include "dice/hypertrie/internal/raw/node_context/BulkUpdaterSettings.hpp"
#include "dice/hypertrie/internal/raw/node_context/BulkUpdater_callback.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"

#include <robin_hood.h>

#include <utility>

namespace dice::hypertrie::internal::raw {

	template<BulkUpdaterMode mode, size_t depth, HypertrieTrait_bool_valued htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	class SynchronousRawHypertrieBulkUpdater {
		// TODO: extend to non-Boolean valued hypertries
		// TODO: extend to change values (only non-Boolean valued hypertries)
	public:
		using Entry = SingleEntry<depth, htt_t>;
		using key_part_type = typename htt_t::key_part_type;

	private:
		uint32_t bulk_size_;
		size_t const deduplication_max_size_;
		RawNodeContainer<htt_t, allocator_type> *nodec_;
		RawHypertrieContext<context_max_depth, htt_t, allocator_type> *context_;
		std::vector<Entry> new_entries_;// buffer_size
		BulkUpdater_bulk_processed_callback get_stats_;
		::robin_hood::unordered_set<RawIdentifier<depth, htt_t>> de_duplication_;
		size_t no_seen_entries = 0;

	public:
		/**
		 * @param nodec
		 * @param context
		 * @param bulk_size
		 * @param get_stats see BulkUpdater_bulk_processed_callback
		 */
		SynchronousRawHypertrieBulkUpdater(
				RawNodeContainer<htt_t, allocator_type> &nodec,
				RawHypertrieContext<context_max_depth, htt_t, allocator_type> &context,
				uint32_t bulk_size = 1'000'000U,
				BulkUpdater_bulk_processed_callback get_stats = [](auto...) {}) noexcept
			: bulk_size_(bulk_size), deduplication_max_size_(4UL * bulk_size_), nodec_(&nodec), context_(&context), get_stats_(std::move(get_stats)), de_duplication_(bulk_size_ + 1) {

			if (bulk_size_ == 0)
				bulk_size_ = 1;
			new_entries_.reserve(bulk_size_);
		}

		~SynchronousRawHypertrieBulkUpdater() {
			flush();
		}

		void add(Entry const &entry) noexcept {
			RawIdentifier<depth, htt_t> entry_hash{entry};
			const auto &[_, unseen] = de_duplication_.insert(entry_hash);
			if (de_duplication_.size() == deduplication_max_size_)
				de_duplication_.clear();

			if (not unseen)
				return;

			bool const already_stored = context_->template get<depth>(NodeContainer<depth, htt_t, allocator_type>{*nodec_}, entry.key());

			if constexpr (mode == BulkUpdaterMode::Insert) {
				if (already_stored) {
					return;
				}
			} else if constexpr (mode == BulkUpdaterMode::Remove) {
				if (not already_stored) {
					return;
				}
			}

			new_entries_.push_back(entry);

			if (new_entries_.size() >= bulk_size_)
				flush();
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
			if (not new_entries_.empty()) {
				NodeContainer<depth, htt_t, allocator_type> nodec{*nodec_};
				auto const new_entries_size = new_entries_.size();
				context_->insert(nodec, std::move(new_entries_));
				*nodec_ = nodec;
				get_stats_(no_seen_entries, new_entries_size, context_->size(nodec));
				new_entries_.clear();
			}
		}
	};

	template<size_t depth, HypertrieTrait_bool_valued htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	using SynchronousRawHypertrieBulkInserter = SynchronousRawHypertrieBulkUpdater<BulkUpdaterMode::Insert, depth, htt_t, allocator_type, context_max_depth>;

	template<size_t depth, HypertrieTrait_bool_valued htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	using SynchronousRawHypertrieBulkRemover = SynchronousRawHypertrieBulkUpdater<BulkUpdaterMode::Remove, depth, htt_t, allocator_type, context_max_depth>;

}// namespace dice::hypertrie::internal::raw

#endif// HYPERTRIE_SYNCHRONOUSRAWHYPERTRIEBULKINSERTER_HPP
