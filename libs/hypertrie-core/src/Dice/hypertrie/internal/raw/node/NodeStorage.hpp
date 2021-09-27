#ifndef HYPERTRIE_NODESTORAGE_HPP
#define HYPERTRIE_NODESTORAGE_HPP


#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp>

#include <Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp>


namespace hypertrie::internal::raw {

	template<size_t max_depth, HypertrieCoreTrait_bool_valued tri_t>
	class NodeStorage {
	public:
		using tri = tri_t;
		using allocator_type = typename tri::allocator_type;
		template<size_t depth>
		using SingleEntryNodeStorage_t = SpecificNodeStorage<depth, tri, SingleEntryNode>;
		template<size_t depth>
		using FullNodeStorage_t = SpecificNodeStorage<depth, tri, FullNode>;
		using SingleEntryNodes = util::IntegralTemplatedTuple<SingleEntryNodeStorage_t, (tri::is_bool_valued) ? 1 : 2, max_depth, allocator_type const &>;
		using FullNodes = util::IntegralTemplatedTuple<FullNodeStorage_t, 1, max_depth, allocator_type const &>;

	private:
		SingleEntryNodes single_entry_nodes;
		FullNodes full_nodes;

	public:
		explicit NodeStorage(const typename tri::allocator_type &alloc) : single_entry_nodes(alloc), full_nodes(alloc) {}
	};
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_NODESTORAGE_HPP
