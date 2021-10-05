#ifndef HYPERTRIE_CONTEXTLEVELCHANGES_HPP
#define HYPERTRIE_CONTEXTLEVELCHANGES_HPP

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/NodeContainer.hpp>

#include <Dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <tsl/sparse_map.h>
#include <tsl/sparse_set.h>

namespace hypertrie::internal::raw {

	template<size_t depth, HypertrieCoreTrait tri_t>
	class ContextLevelChanges {
	public:
		using tri = tri_t;
		using Entry = SingleEntry<depth, tri_with_stl_alloc<tri>>;
		using Identifier_t = Identifier<depth, tri>;

		struct SEN_Change {
			ssize_t ref_count_delta = 0;
			Entry entry;
		};

		struct FN_New {
			FNContainer<depth, tri> after;
			Identifier_t sen_node_before;
			std::vector<Entry> entries;
		};

		/**
		 * For single entry nodes:
		 * id_after -> (ref_count_delta, entry)
		 */
		tsl::sparse_map<Identifier_t, SEN_Change> SEN_new_ones;
		/**
		 * inserting into full nodes:
		 * id_before -> (id_after -> entries)
		 * id_before and id_after identify both full nodes.
		 */
		tsl::sparse_map<Identifier_t, tsl::sparse_map<Identifier_t, std::vector<Entry>>> FN_changes;
		/**
		 * creating new full nodes
		 *
		 */
		tsl::sparse_map<Identifier_t, FN_New> FN_new_ones;

		/**
		 * id -> delta
		 */
		tsl::sparse_map<Identifier_t, ssize_t> fn_deltas;

		/**
		 * done full nodes
		 */
		tsl::sparse_set<Identifier_t> done_fns;

		Identifier_t add_node(std::vector<Entry> entries, ssize_t n = 1) noexcept {
			if (entries.size() == 1) {
				Identifier_t id_after{entries[0]};
				auto &change = SEN_new_ones[id_after];
				change.ref_count_delta += n;
				if constexpr (not(depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>))
					change.entry = entries[0];
				return id_after;
			} else {
				Identifier_t id_after{entries};
				if (auto found = fn_deltas.find(id_after); found != fn_deltas.end())
					found.value() += n;
				else {
					fn_deltas.insert(found, {id_after, n});

					FN_New new_fn{};
					new_fn.entries = std::move(entries);
					FN_new_ones.insert({id_after, new_fn});
				}
				return id_after;
			}
		}

		Identifier_t insert_into_node(Identifier_t id_before, std::vector<Entry> const &entries, ssize_t n = 1) noexcept {
			auto id_after = Identifier_t{entries}.combine(id_before);
			fn_deltas[id_after] += n;
			if (id_before.is_sen()) {
				if constexpr (depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) {
					auto &change = FN_new_ones[id_after];
					change.entries = entries;
					change.entries.push_back(id_before.get_entry());
				} else {
					// decrement refcount delta of node before
					SEN_new_ones[id_before].ref_count_delta -= n;

					// new node must be created
					auto &change = FN_new_ones[id_after];
					change.entries = entries;
					change.after.identifier() = id_after;
					change.sen_node_before = id_before;
				}
			} else {
				fn_deltas[id_before] -= n;
				auto &changes = FN_changes[id_before];
				if (auto found = changes.find(id_after); found == changes.end())
					changes[id_after] = entries;
			}

			return id_after;
		}

		/**
		 * for FN:
		 * id_before -> {id_after}
		 */
		boost::container::flat_map<Identifier_t, boost::container::flat_set<Identifier_t>> moveables_fns;

		void calc_moveables(SpecificNodeStorage<depth, tri, FullNode> &specific_full_node_storage) noexcept {
			auto &nodes_ = specific_full_node_storage.nodes();
			for (const auto &[id_before, changes] : FN_changes) {
				if (ssize_t(nodes_[id_before]->ref_count()) + fn_deltas[id_before] == 0) {
					for (const auto &[id_after, _] : changes) {
						if (not nodes_.contains(id_after)) {
							moveables_fns[id_before].insert(id_after);
						}
					}
				}
			}
		}
	};

	template<HypertrieCoreTrait tri>
	class ContextLevelChanges<0, tri> {
		// is instantiated for simplicity but must never be used (therefore, has no members or methods).
	};
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_CONTEXTLEVELCHANGES_HPP
