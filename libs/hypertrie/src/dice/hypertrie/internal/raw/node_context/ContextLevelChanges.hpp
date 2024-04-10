#ifndef HYPERTRIE_CONTEXTLEVELCHANGES_HPP
#define HYPERTRIE_CONTEXTLEVELCHANGES_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/NodeContainer.hpp"

#include "dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp"

#include <robin_hood.h>

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	class ContextLevelChanges {
	public:
		using Entry = SingleEntry<depth, htt_t>;
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;

		struct SEN_Change {
			ssize_t ref_count_delta = 0;
			std::optional<Entry> entry;
		};

		struct FN_New {
			FNContainer<depth, htt_t, allocator_type> after;
			RawIdentifier_t sen_node_before;
			std::vector<Entry> entries;
		};

		/**
		 * For single entry nodes:
		 * id_after -> (ref_count_delta, entry)
		 */
		::robin_hood::unordered_map<RawIdentifier_t, SEN_Change> SEN_new_ones{};
		/**
		 * inserting into full nodes:
		 * id_before -> (id_after -> entries)
		 * id_before and id_after identify both full nodes.
		 */
		::robin_hood::unordered_map<RawIdentifier_t, ::robin_hood::unordered_map<RawIdentifier_t, std::vector<Entry>>> FN_changes{};
		/**
		 * creating new full nodes
		 *
		 */
		::robin_hood::unordered_map<RawIdentifier_t, FN_New> FN_new_ones{};

		/**
		 * id -> delta
		 */
		::robin_hood::unordered_map<RawIdentifier_t, ssize_t> fn_deltas{};

		/**
		 * Full nodes which are simply incremented.
		 */
		::robin_hood::unordered_set<RawIdentifier_t> fn_incs{};

		/**
		 * done full nodes
		 */
		::robin_hood::unordered_set<RawIdentifier_t> done_fns{};

		void inc_ref(RawIdentifier_t id, ssize_t n = 1) noexcept {
			if (id.is_fn()) {
				fn_deltas[id] += n;
				fn_incs.insert(id);
			} else {
				SEN_new_ones[id].ref_count_delta += n;
			}
		}

		RawIdentifier_t add_node(std::vector<Entry> entries, ssize_t n = 1) noexcept {
			if (entries.size() == 1) {
				RawIdentifier_t id_after{entries[0]};
				auto &change = SEN_new_ones[id_after];
				change.ref_count_delta += n;
				if constexpr (not(depth == 1 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>))
					change.entry = entries[0];
				return id_after;
			}
			assert(entries.size() > 1);
			RawIdentifier_t id_after{entries};
			if (auto found = fn_deltas.find(id_after); found != fn_deltas.end()) {
				found->second += n;
			} else {
				fn_deltas.insert({id_after, n});

				FN_New new_fn{};
				new_fn.entries = std::move(entries);
				FN_new_ones.insert({id_after, new_fn});
			}
			return id_after;
		}

		RawIdentifier_t insert_into_node(RawIdentifier_t id_before, std::vector<Entry> const &entries, bool decrement_before = false) noexcept {
			auto id_after = RawIdentifier_t{entries}.combine(id_before);
			fn_deltas[id_after] += 1;
			if (id_before.is_sen()) {
				if constexpr (depth == 1 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					auto &change = FN_new_ones[id_after];
					change.entries = entries;
					change.entries.push_back(id_before.get_entry());
				} else {
					// decrement refcount delta of node before
					if (decrement_before)
						SEN_new_ones[id_before].ref_count_delta -= 1;

					// new node must be created
					auto &change = FN_new_ones[id_after];
					change.entries = entries;
					change.after.raw_identifier() = id_after;
					change.sen_node_before = id_before;
				}
			} else {
				if (decrement_before)
					fn_deltas[id_before] -= 1;
				auto &changes = FN_changes[id_before];
				if (not changes.contains(id_after)) {
					changes[id_after] = entries;
				}
			}

			return id_after;
		}

		/**
		 * for FN:
		 * id_before -> {id_after}
		 */
		::robin_hood::unordered_map<RawIdentifier_t, ::robin_hood::unordered_set<RawIdentifier_t>> moveables_fns;

		void calc_moveables(SpecificNodeStorage<depth, htt_t, FullNode, allocator_type> &specific_full_node_storage) noexcept {
			auto &nodes_ = specific_full_node_storage.nodes();
			for (auto const &[id_before, changes] : FN_changes) {
				if (ssize_t(nodes_[id_before]->ref_count()) + fn_deltas[id_before] <= 0) {
					for (auto const &[id_after, _] : changes) {
						//changed for boost map test
						//if (not nodes_.contains(id_after)) {
						if (nodes_.find(id_after) == nodes_.end()) {
							moveables_fns[id_before].insert(id_after);
						}
					}
				}
			}
		}
	};

	/**
	 * Is instantiated for simplicity but must never be used (therefore, has no members or methods).
	 */
	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	class ContextLevelChanges<0, htt_t, allocator_type> {
	};
}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_CONTEXTLEVELCHANGES_HPP
