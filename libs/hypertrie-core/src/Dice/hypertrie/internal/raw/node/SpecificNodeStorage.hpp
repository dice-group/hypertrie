#ifndef HYPERTRIE_SPECIFICNODESTORAGE_HPP
#define HYPERTRIE_SPECIFICNODESTORAGE_HPP

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/AllocateNode.hpp>
#include <Dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp>

namespace hypertrie::internal::raw {

	template<size_t depth, HypertrieCoreTrait tri_t, template<size_t, typename> typename node_type_t>
	class SpecificNodeStorage {
	public:
		using tri = tri_t;
		using allocator_type = typename tri::allocator_type;
		using AllocateNode_t = AllocateNode<depth, tri, node_type_t>;

		using key_type = Identifier<depth, tri>;
		using node_type = node_type_t<depth, tri>;
		using node_pointer_type = typename tri::template allocator_pointer<node_type>;
		using Map_t = typename tri::template map_type<key_type, node_pointer_type>;

	private:
		AllocateNode_t allocate_node_;
		Map_t nodes_;

	public:
		SpecificNodeStorage(const typename tri::allocator_type &alloc)
			: allocate_node_(alloc),
			  nodes_(alloc) {}

		virtual ~SpecificNodeStorage() {
			for (auto &[hash, node] : this->nodes())
				node_lifecycle().delete_(node);
		}

		const AllocateNode_t &node_lifecycle() const noexcept {
			return allocate_node_;
		}

		AllocateNode_t &node_lifecycle() noexcept {
			return allocate_node_;
		}

		Map_t &nodes() noexcept {
			return nodes_;
		}

		const Map_t &nodes() const noexcept {
			return nodes_;
		}
	};
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_SPECIFICNODESTORAGE_HPP
