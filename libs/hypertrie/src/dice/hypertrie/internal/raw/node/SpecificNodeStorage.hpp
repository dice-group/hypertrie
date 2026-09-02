#ifndef HYPERTRIE_SPECIFICNODESTORAGE_HPP
#define HYPERTRIE_SPECIFICNODESTORAGE_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/AllocateNode.hpp"
#include "dice/hypertrie/internal/raw/node/NodeTypes_reflection.hpp"
#include "dice/hypertrie/internal/raw/node/RawIdentifier.hpp"
#include "dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t, template<size_t, typename, typename...> typename node_type_t, ByteAllocator allocator_type>
	class SpecificNodeStorage {
	public:
		using node_type = instantiate_Node_t<node_type_t, depth, htt_t, allocator_type>;
		using node_allocator_traits = typename std::allocator_traits<allocator_type>::template rebind_traits<node_type>;
		using node_allocator_type = typename node_allocator_traits::allocator_type;
		using node_pointer = typename node_allocator_traits::pointer;

		struct hasher_type {
			using is_transparent = void;

			size_t operator()(node_pointer const &node_ptr) const noexcept {
				// TODO benchmark hashing algorithms, maybe sparse_map is not as sensitive as unordered_dense and we can use a more primitive algorithm
				return hash::dice_hash_templates<hash::Policies::wyhash>::dice_hash(node_ptr->hash());
			}

			size_t operator()(RawIdentifier<depth, htt_t> const &identifier) const noexcept {
				return hash::dice_hash_templates<hash::Policies::wyhash>::dice_hash(identifier.hash());
			}
		};

		struct equal_type {
			using is_transparent = void;

			bool operator()(node_pointer const &lhs, node_pointer const &rhs) const noexcept {
				if constexpr (is_SingleEntryNode_v<node_type_t>) {
					return static_cast<SingleEntry<depth, htt_t> const &>(*lhs) == static_cast<SingleEntry<depth, htt_t> const &>(*rhs);
				} else {
					return lhs->identifier() == rhs->identifier();
				}
			}

			bool operator()(node_pointer const &lhs, RawIdentifier<depth, htt_t> const &rhs) const noexcept {
				return lhs->identifier() == rhs;
			}

			bool operator()(RawIdentifier<depth, htt_t> const &lhs, node_pointer const &rhs) const noexcept {
				return lhs == rhs->identifier();
			}
		};

		using node_storage_type = typename htt_t::template set_type<node_pointer, node_allocator_type, hasher_type, equal_type>;
		using node_lifecycle_type = AllocateNode<node_type_t, depth, htt_t, allocator_type>;

	private:
		node_storage_type nodes_;
		[[no_unique_address]] node_lifecycle_type lifecycle_;

	public:
		SpecificNodeStorage(allocator_type const &alloc) noexcept : nodes_{alloc},
																	lifecycle_{alloc} {
		}

		SpecificNodeStorage(SpecificNodeStorage const &other) = delete;
		SpecificNodeStorage &operator=(SpecificNodeStorage const &other) = delete;

		SpecificNodeStorage(SpecificNodeStorage &&other) noexcept = default;
		SpecificNodeStorage &operator=(SpecificNodeStorage &&other) = delete;

		~SpecificNodeStorage() noexcept {
			for (auto node_ptr : nodes_) {
				lifecycle_.delete_(node_ptr);
			}
		}

		[[nodiscard]] node_lifecycle_type &node_lifecycle() noexcept {
			return lifecycle_;
		}

		[[nodiscard]] node_storage_type &nodes() noexcept {
			return nodes_;
		}

		[[nodiscard]] node_storage_type const &nodes() const noexcept {
			return nodes_;
		}
	};
}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_SPECIFICNODESTORAGE_HPP
