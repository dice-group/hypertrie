#ifndef UPDATEREQUESTS_HPP
#define UPDATEREQUESTS_HPP

#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/container/AllContainer.hpp"
#include "dice/hypertrie/internal/raw/node/Identifier.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"
#include "dice/hypertrie/internal/raw/node_context/update_details/EntrySubsetForPos.hpp"

#include <robin_hood.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <vector>


namespace dice::hypertrie::internal::raw::node_context::update_details {

	template<size_t depth, typename htt_t>
	concept HTHSIDepth1 = (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>);
	template<size_t depth, typename htt_t>
	concept HTHSIDepth2 = (depth == 2 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>);

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t max_depth>
	struct UpdatePlan;

	enum struct EntriesUpdateMode : uint8_t {
		INSERT,
		ERASE
	};

	/**
	 * Register update requests to hypertries of depth here.
	 * @tparam depth hypertrie depth
	 * @tparam htt_t hypertrie trait
	 * @tparam allocator_type
	 * @tparam max_depth
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t max_depth>
	struct UpdateRequests {
		static constexpr bool ht_hsi_depth1 = HTHSIDepth1<depth, htt_t>;
		friend UpdatePlan<depth, htt_t, allocator_type, max_depth>;

		/// public type defs

		using SingleEntry_t = SingleEntry<depth, htt_t>;
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;
		using NodeStorage_t = NodeStorage<max_depth, htt_t, allocator_type>;

		/**
		 * Changes to a single SEN.
		 */
		struct SENChange {
			ssize_t ref_count_delta = 0;       ///< delta to the SEN ref_count
			std::optional<SingleEntry_t> entry;///< affected single entry
		};

		struct FNEntriesUpdateData {
			std::vector<SingleEntry_t> entries;///< entries to be inserted
			EntriesUpdateMode mode;            ///< if the entries are inserted or erased
		};

	private:
		/// private type defs
		using key_part_type = typename htt_t::key_part_type;
		template<size_t depth2>
		using FNPtr_t = typename NodeStorage_t::template SpecificNodePtr<depth2, FullNode>;

		/**
		 * see sen_changes_
		 */
		using SENChanges = robin_hood::unordered_map<RawIdentifier_t, std::conditional_t<!ht_hsi_depth1, SENChange, SingleEntry_t>>;

		/// fields
		/**
		 * NodeStorage for retrieving nodes and looking up information about nodes relevant for planning.
		 */
		NodeStorage_t const &node_storage_;
		/**
		* Changes to SENs. This include creations, ref_count deltas and erasures (because of resulting ref_counts of 0).
		 * id -> (ref_count_delta, entry)<br/>
		 * special case hypertrie-hsi of depth1: Tracks the entries for ids.<br/>
		 * id ->  entry
		 * this is used as cache for entries (note: the id is calculated as if in-place storage was not supported)
		 */
		SENChanges sen_changes_{};
		/**
		 * source_id -> (target_id -> entries)<br/>
		 * Inserting or erasing entries in the node for source_id will result in the node for target_id.
		 */
		robin_hood::unordered_map<RawIdentifier_t, robin_hood::unordered_map<RawIdentifier_t, FNEntriesUpdateData>> fn_update_entries_{};
		/**
		 * Creating a FN from the entries will result in the node for id.<br/>
		 * id -> entries
		 */
		robin_hood::unordered_map<RawIdentifier_t, std::vector<SingleEntry_t>> fn_creations_{};
		/**
		 * delta to be applied to ref_count of fn for id <br/>
		 * id -> delta
		 */
		robin_hood::unordered_map<RawIdentifier_t, ssize_t> fn_deltas_{};
		/**
		 * FNs that are potentially deleted. All nodes for which the ref_count is being decreased at least once are deletion candidates.
		 */
		robin_hood::unordered_set<RawIdentifier_t> fn_deletion_candidates_{};
		/**
		 * FNs that are targets of entry insertions or erasures can potentially be created with move semantics, i.e., the source could be altered and reused.
		 */
		robin_hood::unordered_set<RawIdentifier_t> fn_move_target_candidates_{};
		/**
		 * Nodes that are potentially created. For these identifiers the ref_count is increased at least once.
		 */
		robin_hood::unordered_set<RawIdentifier_t> fn_potentially_new_{};


	public:
		UpdateRequests(NodeStorage_t const &node_storage)
			: node_storage_{node_storage} {}

		RawIdentifier_t add_node(std::vector<SingleEntry_t> &&entries, ssize_t n = 1) noexcept {
			assert(!entries.empty());
			assert(std::ranges::all_of(entries, [](auto &entry) {
				return std::ranges::all_of(entry.key(), [](auto &key_part) { return key_part != typename htt_t::key_part_type{}; });
			}));
			assert(std::ranges::all_of(entries, [](auto &entry) {
				return entry.value() != typename htt_t::value_type{};
			}));
			RawIdentifier_t id_after{entries};
			if (entries.size() == 1) {
				if constexpr (ht_hsi_depth1) {
					return id_after;
				} else {
					auto &change = sen_changes_[id_after];
					change.ref_count_delta += n;
					change.entry = entries[0];
				}
			} else {
				fn_creations_.emplace(id_after, std::move(entries));
				apply_ref_count_delta(id_after, 1);
			}
			return id_after;
		}

		/**
		 * Request the insertion of entries into the node with id source_id.
		 * @param source_id id of the source node where entries are inserted
		 * @param entries the entries to be inserted
		 * @param node_before_needs_decrement if the ref_count of the source node needs to be decremented
		 * @return the (possibly future) id of the target node
		 */
		RawIdentifier_t insert_into_node(RawIdentifier_t source_id, std::vector<SingleEntry_t> &&entries, bool node_before_needs_decrement = false) noexcept {
			assert(!entries.empty());
			assert(!source_id.empty());
			assert(std::ranges::all_of(entries, [](auto &entry) {
				return std::ranges::all_of(entry.key(), [](auto &key_part) { return key_part != typename htt_t::key_part_type{}; });
			}));
			assert(std::ranges::all_of(entries, [](auto &entry) {
				return entry.value() != typename htt_t::value_type{};
			}));
			auto const target_id = RawIdentifier_t{entries}.combine(source_id);
			apply_ref_count_delta(target_id, 1);// the target node is always a FN
			if (source_id.is_sen()) {
				// if the source is a SEN its entry is retrieved and added to entries
				// from entries a new node is created
				if constexpr (ht_hsi_depth1) {
					// for in-place stored nodes the entry is stored directly in the source_id
					entries.push_back(source_id.get_entry());
				} else {
					auto &sen_change = [&]() -> SENChange & {// retrieve or create the SEN change
						if (auto found = sen_changes_.find(source_id); found != sen_changes_.end()) {
							auto &ret = found->second;
							return ret;
						}
						assert(sens().contains(source_id));
						auto &ret = sen_changes_[source_id];
						ret.entry = *sens().at(source_id);
						return ret;
					}();
					// decrement refcount delta of node before
					if (node_before_needs_decrement) {
						sen_change.ref_count_delta -= 1;
					}
					// add the entry of the source SEN to entries
					if (!sen_change.entry.has_value()) {
						sen_change.entry = *sens().at(source_id);
					}
					entries.push_back(sen_change.entry.value());
				}
				// register a new node to be created
				// note: emplace inserts only if it doesn't exist yet
				fn_creations_.emplace(target_id, std::move(entries));
			} else {// source_id.is_fn()
				if (node_before_needs_decrement) {
					fn_move_target_candidates_.insert(target_id);
					apply_ref_count_delta(source_id, -1);
				}
				auto &changes = fn_update_entries_[source_id];
				if (not changes.contains(target_id)) {
					changes[target_id] = FNEntriesUpdateData{.entries = std::move(entries), .mode = EntriesUpdateMode::INSERT};
				}
			}

			return target_id;
		}


		/**
		 * Remove entries from a node and return the resulting identifier.
		 *
		 * @param source_id the id of the node from which the entries are removed
		 * @param entries the entries to be removed
		 * @return The ID that the node will have after insertion. If all entries are removed, an empty node is returned.
		 */
		RawIdentifier_t remove_from_node(RawIdentifier_t const source_id, std::vector<SingleEntry_t> &&entries, bool node_before_needs_decrement = false) noexcept {
			assert(!entries.empty());
			assert(std::ranges::all_of(entries, [](auto &entry) {
				return std::ranges::all_of(entry.key(), [](auto &key_part) { return key_part != typename htt_t::key_part_type{}; });
			}));
			assert(std::ranges::all_of(entries, [](auto &entry) {
				return entry.value() != typename htt_t::value_type{};
			}));

			// remove a single entry
			if (source_id.is_sen()) {
				assert(entries.size() == 1);

				if constexpr (ht_hsi_depth1) {
					assert(source_id.get_entry() == entries[0]);
				} else {
					assert(static_cast<SingleEntry_t>(*sens().at(source_id)) == entries[0]);
					// decrement refcount delta of source node
					if (node_before_needs_decrement) {
						sen_changes_[source_id].ref_count_delta -= 1;
					}

					sen_changes_[source_id].entry = entries[0];
				}
				return RawIdentifier_t{};
			}

			// target_id value is provisional. Might be retagged to a single entry node or replaced by a in-place stored node (dpeth 1, hsi)
			auto target_id = RawIdentifier_t{entries}.combine(source_id);

			if (node_before_needs_decrement) {
				apply_ref_count_delta(source_id, -1);
			}

			if (target_id.empty())// node is being deleted
				return target_id;

			assert(fns().contains(source_id));
			auto const fn_ptr = fns().at(source_id);
			size_t const target_size = fn_ptr->size() - entries.size();

			//  if (target_size == 0) nothing to be done with the target node
			if (target_size == 1) {
				target_id.retag_as_sen();

				if constexpr (ht_hsi_depth1) {
					if (auto found = sen_changes_.find(target_id); found != sen_changes_.end()) {
						return RawIdentifier_t{found->second};
					}
					auto const entry = retrieve_remaining_single_entry(fn_ptr, std::move(entries));
					sen_changes_[target_id] = entry;
					return RawIdentifier_t{entry};
				} else {


					// first, check if there is already a SEN change for the target_id
					if (auto found = sen_changes_.find(target_id); found != sen_changes_.end()) {
						auto &sen_change = found->second;
						sen_change.ref_count_delta += 1;
						return target_id;
					}

					// second, check if the node exists already
					if (auto it = sens().find(target_id); it != sens().end()) {
						sen_changes_[target_id] = {.ref_count_delta = 1, .entry = *container::deref(it)};
						return target_id;
					}

					// third, calculate the remaining entry and add it to sen_changes_
					sen_changes_[target_id] = {
							.ref_count_delta = 1,
							.entry = retrieve_remaining_single_entry(fn_ptr, std::move(entries))};
					assert(RawIdentifier_t{sen_changes_[target_id].entry.value()} == target_id);
					return target_id;
				}
			}

			if (target_size > 1) {
				apply_ref_count_delta(target_id, 1);
				fn_move_target_candidates_.insert(target_id);
				auto &changes = fn_update_entries_[source_id];
				// note: only inserted if no FNUpdate for source_id -> target_id exists already
				changes.emplace(target_id, FNEntriesUpdateData{.entries = std::move(entries), .mode = EntriesUpdateMode::ERASE});
				return target_id;
			}

			assert(false);
			return {};
		}

		/**
		 * Add a delta to the ref_count of the node identified by id. The user must make sure that the ref_count will not become negative.
		 * @param id id of node where ref_count should be altered
		 * @param ref_count_delta the value that will be added to the ref_count
		 */
		void apply_ref_count_delta(RawIdentifier_t id, ssize_t ref_count_delta = 1) noexcept {
			if (ref_count_delta == 0)
				return;
			if (id.is_fn()) {
				fn_deltas_[id] += ref_count_delta;
				if (ref_count_delta < 0)// necessary to support removing nodes via decrementing ref_count (e.g., when a hypertrie constuctor is called)
					fn_deletion_candidates_.insert(id);
				else// ref_count_delta > 0
					fn_potentially_new_.emplace(id);
			} else {
				// note: depth=1, hsi SEN are never stored. But it is easier to guard the ref_count_delta assignment here with a constexpr if then make hard to read changes in the actual insertion code.
				if constexpr (!ht_hsi_depth1) {
					sen_changes_[id].ref_count_delta += ref_count_delta;
				}
			}
		}

	private:
		auto const &fns() const {
			return node_storage_.template nodes<depth, FullNode>().nodes();
		}

		auto const &sens() const
			requires(!ht_hsi_depth1)
		{
			return node_storage_.template nodes<depth, SingleEntryNode>().nodes();
		}

		template<size_t depth2>
		SingleEntry<depth2, htt_t> retrieve_remaining_single_entry(FNPtr_t<depth2> fn_ptr,
																   std::vector<SingleEntry<depth2, htt_t>> &&entries) {
			auto entry = retrieve_remaining_single_entry_impl(fn_ptr, std::move(entries)).value();
			assert(
					std::ranges::all_of(entry.key(), [](auto &key_part) { return key_part != typename htt_t::key_part_type{}; }));
			assert(entry.value() != typename htt_t::value_type{});
			return entry;
		}
		template<size_t depth2>
		std::optional<SingleEntry<depth2, htt_t>> retrieve_remaining_single_entry_impl(FNPtr_t<depth2> fn_ptr,
																					   std::vector<SingleEntry<depth2, htt_t>> &&entries) noexcept {
			static_assert(depth2 > 0);
			auto const cards = fn_ptr->getCards();

			// take the position with the highest cardinality because it has the highest probability to reference a SEN or have one stored in-place
			size_t const pos = std::distance(cards.begin(), std::max_element(cards.begin(), cards.end()));

			if constexpr (depth2 <= 1) {
				::robin_hood::unordered_set<key_part_type> removed_parts;
				for (auto const &entry : entries)
					removed_parts.insert(entry.key()[0]);

				for (auto const entry : fn_ptr->edges()) {
					auto const key_part = [&]() {if constexpr (HypertrieTrait_bool_valued<htt_t>) return entry; else return entry.first; }();
					if (!removed_parts.contains(key_part)) {
						auto const value = [&]() {if constexpr (HypertrieTrait_bool_valued<htt_t>) return true; else return entry.second; }();
						return SingleEntry<1, htt_t>{{key_part}, value};
					}
				}
			} else {
				auto combine_res = [](key_part_type key_part, size_t pos, SingleEntry<depth2 - 1, htt_t> const &sub_res) {
					SingleEntry<depth2, htt_t> res{};
					for (size_t i = 0, j = 0; i < depth2; ++i) {
						if (i == pos)
							res.key()[i] = key_part;
						else
							res.key()[i] = sub_res.key()[j++];
					}
					if constexpr (!HypertrieTrait_bool_valued<htt_t>)
						res.value() = sub_res.value();
					return res;
				};

				auto removed_parts = entry_subset_for_pos(entries, pos);


				for (auto const &[key_part, child_id] : fn_ptr->edges(pos)) {
					auto iter = removed_parts.find(key_part);
					if (iter == removed_parts.end()) {
						if constexpr (depth2 == 2 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
							return combine_res(key_part, pos, child_id.get_entry());
						} else {
							return combine_res(key_part, pos, static_cast<SingleEntry<depth2 - 1, htt_t>>(*node_storage_.template lookup<(depth2 - 1), SingleEntryNode>(child_id)));
						}
					}
					auto &&next_entries = std::move(iter->second);
					if (child_id.is_sen()) {
						assert((RawIdentifier<depth2 - 1, htt_t>{next_entries} == child_id));
						continue;
					}
					assert(child_id.is_fn() && !child_id.empty());

					auto child_fn_ptr = node_storage_.template lookup<(depth2 - 1), FullNode>(child_id);
					auto const sub_entry = retrieve_remaining_single_entry_impl<depth2 - 1>(child_fn_ptr, std::move(next_entries));
					if (sub_entry.has_value())
						return combine_res(key_part, pos, sub_entry.value());
				}
			}

			return std::nullopt;
		}
	};
}// namespace dice::hypertrie::internal::raw::node_context::update_details
#endif//UPDATEREQUESTS_HPP
