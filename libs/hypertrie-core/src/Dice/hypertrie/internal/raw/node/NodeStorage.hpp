#ifndef HYPERTRIE_NODESTORAGE_HPP
#define HYPERTRIE_NODESTORAGE_HPP


#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/NodeContainer.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp>

#include <Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp>


namespace hypertrie::internal::raw {

	template<size_t max_depth, HypertrieCoreTrait tri_t>
	class NodeStorage {
	public:
		using tri = tri_t;
		using allocator_type = typename tri::allocator_type;

		// TODO: naming might need some revision
		template<size_t depth>
		using SingleEntryNodeStorage_t = SpecificNodeStorage<depth, tri, SingleEntryNode>;
		template<size_t depth>
		using FullNodeStorage_t = SpecificNodeStorage<depth, tri, FullNode>;

		template<template<size_t, typename> typename node_type>
		constexpr static bool is_full() {
			return std::is_same_v<node_type<2, tri>, FullNode<2, tri>>;
		}

		template<size_t depth, template<size_t, typename> typename node_type>
		using SpecificNodes = std::conditional_t<
				(is_full<node_type>()),
				FullNodeStorage_t<depth>,
				SingleEntryNodeStorage_t<depth>>;

		template<size_t depth, template<size_t, typename> typename node_type>
		using SpecificNodePtr = typename SpecificNodes<depth, node_type>::node_pointer_type;
		using SingleEntryNodes = util::IntegralTemplatedTuple<SingleEntryNodeStorage_t, (HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) ? 2 : 1, max_depth, allocator_type const &>;
		using FullNodes = util::IntegralTemplatedTuple<FullNodeStorage_t, 1, max_depth, allocator_type const &>;

	private:
		SingleEntryNodes single_entry_nodes;
		FullNodes full_nodes;

	public:
		explicit NodeStorage(const typename tri::allocator_type &alloc) : single_entry_nodes(alloc), full_nodes(alloc) {}

		template<size_t depth, template<size_t, typename> typename node_type>
		SpecificNodes<depth, node_type> &nodes() noexcept {
			if constexpr (is_full<node_type>())
				return full_nodes.template get<depth>();
			else
				return single_entry_nodes.template get<depth>();
		}

		template<size_t depth, template<size_t, typename> typename node_type>
		const SpecificNodes<depth, node_type> &nodes() const noexcept {
			if constexpr (is_full<node_type>())
				return full_nodes.template get<depth>();
			else
				return single_entry_nodes.template get<depth>();
		}

		template<size_t depth, template<size_t, typename> typename node_type>
		SpecificNodePtr<depth, node_type> lookup(Identifier<depth, tri> identifier) noexcept {
			auto &nodes_ = nodes<depth, node_type>().nodes();
			auto found = nodes_.find(identifier);
			if (found != nodes_.end()) {
				return found->second;
			} else {
				return {};
			}
		}

		/**
		 * If a sen for identifier is stored already, the delta_ref_count is updated and sen populated.
		 * Otherwise, a new node with the content from sen is created.
		 * @tparam depth
		 * @param identifier
		 * @param sen
		 * @param delta_ref_count
		 */
		template<size_t depth>
		void update_or_create_sen(Identifier<depth, tri> identifier,
						SingleEntry<depth, tri_with_stl_alloc<tri>> &sen,
						ssize_t delta_ref_count) noexcept {
			auto &nodes_ = this->template nodes<depth, SingleEntryNode>().nodes();
			auto &node_lifecycle_ = this->template nodes<depth, SingleEntryNode>().node_lifecycle();
			if (delta_ref_count != 0) {
				auto found = nodes_.find(identifier);
				if (found != nodes_.end()) {
					auto &node_ptr = found->second;
					assert(ssize_t(node_ptr->ref_count()) + delta_ref_count >= 0);
					node_ptr->ref_count() += delta_ref_count;
					sen = SingleEntry<depth, tri_with_stl_alloc<tri>>{node_ptr->key(), node_ptr->value()};
					if (node_ptr->ref_count() == 0UL)
						nodes_.erase(found);
				} else {
					assert(delta_ref_count > 0);
					nodes_.insert(found, {identifier, node_lifecycle_.new_(sen, delta_ref_count)});
				}
			}
		}

		template<size_t depth>
		NodeContainer<depth, tri> lookup(Identifier<depth, tri> identifier) {
			if (identifier.empty())
				return {};
			else if (identifier.is_fn()) {
				auto fn_ptr = lookup<depth, FullNode>(identifier);
				if (fn_ptr != nullptr)
					return FNContainer<depth, tri>{identifier, fn_ptr};
			} else {// identifier.is_sen()
				auto sen_ptr = lookup<depth, SingleEntryNode>(identifier);
				if (sen_ptr != nullptr)
					return SENContainer<depth, tri>{identifier, sen_ptr};
			}
			return {};
		}
	};
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_NODESTORAGE_HPP
