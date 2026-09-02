#ifndef HYPERTRIE_FULLNODE_HPP
#define HYPERTRIE_FULLNODE_HPP

#include <cstddef>

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/Hashed.hpp"
#include "dice/hypertrie/internal/raw/node/NodePtr.hpp"
#include "dice/hypertrie/internal/raw/node/ReferenceCounted.hpp"
#include "dice/hypertrie/internal/raw/node/Sized.hpp"
#include "dice/hypertrie/internal/raw/node/WithEdges.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct FullNode : Hashed<depth, htt_t>, ReferenceCounted, Sized, WithEdges<depth, htt_t, allocator_type> {
	private:
		using Base = WithEdges<depth, htt_t, allocator_type>;
	public:
		using typename Base::key_part_type;
		using typename Base::value_type;
		using typename Base::child_type;
		using typename Base::edge_type;
		using typename Base::single_dim_edges_allocator;
		using typename Base::single_dim_edges_type;
		using typename Base::edges_type;

		FullNode() noexcept = default;

		FullNode(RawIdentifier<depth, htt_t> const &id, size_t ref_count, allocator_type const &alloc) noexcept : Hashed<depth, htt_t>{id.hash()},
																												  ReferenceCounted{ref_count},
																												  Sized{id.size()},
																												  Base{alloc} {
		}

		[[nodiscard]] RawIdentifier<depth, htt_t> identifier() const noexcept {
			return RawIdentifier<depth, htt_t>{this->hash(), this->size(), IdentifierTag::FN};
		}
	};

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct FullNode<1, htt_t, allocator_type> : Hashed<1, htt_t>, ReferenceCounted, WithEdges<1, htt_t, allocator_type> {
	private:
		using Base = WithEdges<1, htt_t, allocator_type>;
	public:
		using typename Base::key_part_type;
		using typename Base::value_type;
		using typename Base::child_type;
		using typename Base::edge_type;
		using typename Base::single_dim_edges_allocator;
		using typename Base::single_dim_edges_type;
		using typename Base::edges_type;

		FullNode() noexcept = default;

		FullNode(RawIdentifier<1, htt_t> const &id, size_t ref_count, allocator_type const &alloc) noexcept : Hashed<1, htt_t>{id.hash()},
																											  ReferenceCounted{ref_count},
																											  Base{alloc} {
		}

		[[nodiscard]] size_t size() const noexcept {
			return this->edges().size();
		}

		[[nodiscard]] RawIdentifier<1, htt_t> identifier() const noexcept {
			return RawIdentifier<1, htt_t>{this->hash(), this->size(), IdentifierTag::FN};
		}
	};
}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_FULLNODE_HPP
