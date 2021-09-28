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
	};

	template<size_t max_depth, HypertrieCoreTrait_bool_valued tri_t>
	struct update_node_in_context {
		using tri = tri_t;


		template<size_t depth>
		static void exec(NodeStorage<max_depth, tri> &node_storage,
						 NodeContainer<depth, tri> &nodec,
						 std::vector<SingleEntry<depth, tri>> entries) {
			ContextLevelChanges<depth, tri> changes;
			using Entry = SingleEntry<depth, tri>;

			if (entries.empty())
				return;


			if (nodec.empty()) {
				if (entries.size() == 1)
					nodec.identifier() = changes.add_single_entry_node(entries);
				else
					nodec.identifier() = changes.add_full_node(entries);
			} else if (nodec.is_fn()) {
				nodec.identifier() = changes.update_full_node(nodec, entries);
			} else {// nodec.is_sen()
				changes.remove_single_entry_node(nodec);
				nodec.identifier() = changes.add_full_node(nodec, entries);
			}

			apply<depth>(node_storage, nodec, changes);

			if (not(depth == 1 and tri::taggable_key_part))
				nodec = node_storage.template lookup<depth>(nodec.identifier());
			else
				nodec.void_node_ptr() = {};
		}

		template<size_t depth>
		static void apply(NodeStorage<max_depth, tri> &node_storage,
						  NodeContainer<depth, tri> &nodec,
						  ContextLevelChanges<depth, tri> &changes) {
		}
	};


}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_NODECONTEXT_HPP
