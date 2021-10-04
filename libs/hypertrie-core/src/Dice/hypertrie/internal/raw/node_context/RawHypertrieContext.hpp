#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include <Dice/hypertrie/internal/raw/node/NodeStorage.hpp>
#include <Dice/hypertrie/internal/raw/node_context/insert_impl_RawNodeContext.hpp>


namespace hypertrie::internal::raw {

	template<size_t max_depth, HypertrieCoreTrait_bool_valued tri_t>
	struct RawHypertrieContext {
		using tri = tri_t;

		NodeStorage<max_depth, tri_t> node_storage_;

		explicit RawHypertrieContext(const typename tri::allocator_type &alloc) : node_storage_(alloc) {}

		/**
		 * Entries must not yet be contained in nodec
		 * @tparam depth depth of the hypertrie
		 * @param nodec nodec
		 * @param entries
		 */
		template<size_t depth>
		void insert(NodeContainer<depth, tri> &nodec,
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries) {
			insert_impl_RawNodeContext<max_depth, tri>::exec(node_storage_, nodec, entries);
		}
	};
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_NODECONTEXT_HPP
