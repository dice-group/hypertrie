#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include <Dice/hypertrie/internal/raw/node/ContextLevelChanges.hpp>
#include <Dice/hypertrie/internal/raw/node/NodeStorage.hpp>


namespace hypertrie::internal::raw {

	template<size_t max_depth, HypertrieCoreTrait_bool_valued tri_t>
	struct update_node_in_context;


	template<size_t max_depth, HypertrieCoreTrait_bool_valued tri_t>
	struct RawHypertrieContext {
		using tri = tri_t;

		NodeStorage<max_depth, tri_t> node_storage_;

		explicit RawHypertrieContext(const typename tri::allocator_type &alloc) : node_storage_(alloc) {}

		template<size_t depth>
		void insert(NodeContainer<depth, tri> &nodec,
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries) {
			update_node_in_context<max_depth, tri>::exec(node_storage_, nodec, entries);
		}
	};

	template<size_t max_depth, HypertrieCoreTrait_bool_valued tri_t>
	struct update_node_in_context {
		using tri = tri_t;


		template<size_t depth>
		static void exec(NodeStorage<max_depth, tri> &node_storage,
						 NodeContainer<depth, tri> &nodec,
						 std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries) {
			ContextLevelChanges<depth, tri> changes;
			//			using Entry = SingleEntry<depth, tri>;

			if (entries.empty())
				return;


			if (nodec.empty()) {
				nodec.identifier() = changes.add_node(entries);
			} else {
				nodec.identifier() = changes.insert_into_node(nodec.identifier(), entries);
			}

			apply<depth>(node_storage, changes);

			if constexpr (not(depth == 1 and tri::taggable_key_part))
				nodec = node_storage.template lookup<depth>(nodec.identifier());
			else
				nodec.void_node_ptr() = {};
		}

		template<size_t depth>
		static void apply(NodeStorage<max_depth, tri> &node_storage,
						  ContextLevelChanges<depth, tri> &changes) {
			//			ContextLevelChanges<depth -1, tri> next_level_changes;

			for (auto it = changes.SEN_new_ones.begin(); it != changes.SEN_new_ones.end(); ++it) {
				auto id_after = it->first;
				auto &change = it.template value();
				node_storage.template update_sen<depth>(id_after, change.entry, change.ref_count_delta);
			}
		}
	};


}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_NODECONTEXT_HPP
