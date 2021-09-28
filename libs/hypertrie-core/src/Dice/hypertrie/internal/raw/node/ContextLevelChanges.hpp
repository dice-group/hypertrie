#ifndef HYPERTRIE_CONTEXTLEVELCHANGES_HPP
#define HYPERTRIE_CONTEXTLEVELCHANGES_HPP

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/NodeContainer.hpp>

#include <tsl/sparse_map.h>

namespace hypertrie::internal::raw {

	template<size_t depth, HypertrieCoreTrait tri_t>
	class ContextLevelChanges {
	public:
		using tri = tri_t;
		using Entry = SingleEntry<depth, typename tri_t::with_std_allocator>;
		using NodeContainer_t = NodeContainer<depth, tri>;
		using Identifier = typename NodeContainer_t::Identifier;

		struct SEN_Change {
			ssize_t ref_count_delta = 0;
			Entry entry;
		};

		struct FN_New {
			FNContainer<depth, tri> after;
			std::vector<Entry> entries;
			ssize_t ref_count_delta;
		};

		struct FN_Insert : public FN_New {
			FNContainer<depth, tri> before;
		};

		struct FN_Change {
			std::optional<FN_New> new_fn;
			tsl::sparse_map<Identifier, std::vector<Entry>> entries;
		};


	private:
		/**
		 * For single entry nodes:
		 * id_after -> (ref_count_delta, entry)
		 */
		tsl::sparse_map<Identifier, SEN_Change> SEN_changes;
		/**
		 * For full nodes:
		 * id_after ->
		 */
		tsl::sparse_map<Identifier, FN_Change> FN_changes;

	public:
		Identifier add_node(std::vector<Entry> entries) {
			if (entries.size() == 1) {
				auto id_after = Identifier{}.addFirstEntry(entries[0].key(), entries[0].value());
				auto &change = SEN_changes[id_after];
				change.ref_count_delta = 1;
				if constexpr (not(tri::is_bool_valued and tri::taggable_key_part))
					change.entry = entries[0];
				// TODO: go on here
			} else {
				Identifier id_after{};
				id_after.addFirstEntry(entries[0].key(), entries[0].value());
				for (size_t i : iter::range(1, entries.size()))
					id_after.addEntry(entries[i].key(), entries[i].value());
				auto &change = FN_changes[id_after];
				if (change.new_fn.has_value()) {
					change.new_fn.value().ref_count_delta++;
				} else {
					change.new_fn = FN_New{.after = {id_after, {}}, .entries = entries};
				}
			}
		}

		// TODO: go on here
	};
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_CONTEXTLEVELCHANGES_HPP
