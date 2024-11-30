#ifndef HYPERTRIE_SPECIFICNODESTORAGE_HPP
#define HYPERTRIE_SPECIFICNODESTORAGE_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/hypertrie_allocator_trait.hpp"
#include "dice/hypertrie/internal/raw/node/AllocateNode.hpp"
#include "dice/hypertrie/internal/raw/node/Identifier.hpp"
#include "dice/hypertrie/internal/raw/node/NodeTypes_reflection.hpp"
#include "dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t, template<size_t, typename, typename> typename node_type_t, ByteAllocator allocator_type>
	class SpecificNodeStorage {
	private:
		using ht_allocator_trait = hypertrie_allocator_trait<allocator_type>;

	public:
		using AllocateNode_t = AllocateNode<depth, htt_t, node_type_t, allocator_type>;

		using key_type = RawIdentifier<depth, htt_t>;
		using node_type = node_type_t<depth, htt_t, allocator_type>;
		using node_pointer_type = typename ht_allocator_trait::template pointer<node_type>;
		using Map_t = typename htt_t::template map_type<key_type, node_pointer_type, allocator_type>;

	private:
		AllocateNode_t allocate_node_;
		Map_t nodes_;

	public:
		SpecificNodeStorage(allocator_type const &alloc) noexcept
			: allocate_node_(alloc),
			  nodes_(alloc) {}

		SpecificNodeStorage(SpecificNodeStorage const &other) = delete;
		SpecificNodeStorage &operator=(SpecificNodeStorage const &other) = delete;

		SpecificNodeStorage(SpecificNodeStorage &&other) noexcept
			: allocate_node_(std::move(other.allocate_node_)),
			  nodes_(std::move(other.nodes_)) {}

		SpecificNodeStorage &operator=(SpecificNodeStorage &&other) = delete;

		~SpecificNodeStorage() noexcept {
			for (auto &[hash, node] : this->nodes()) {
				node_lifecycle().delete_(node);
			}
			this->nodes().clear();
		}

		[[nodiscard]] const AllocateNode_t &node_lifecycle() const noexcept {
			return allocate_node_;
		}

		AllocateNode_t &node_lifecycle() noexcept {
			return allocate_node_;
		}

		Map_t &nodes() noexcept {
			return nodes_;
		}

		[[nodiscard]] const Map_t &nodes() const noexcept {
			return nodes_;
		}
	};
}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_SPECIFICNODESTORAGE_HPP
