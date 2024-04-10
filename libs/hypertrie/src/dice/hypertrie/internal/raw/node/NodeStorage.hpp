#ifndef HYPERTRIE_NODESTORAGE_HPP
#define HYPERTRIE_NODESTORAGE_HPP


#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/hypertrie_allocator_trait.hpp"
#include "dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "dice/hypertrie/internal/raw/node/NodeContainer.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"
#include "dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp"

#include "dice/template-library/integral_template_tuple.hpp"


namespace dice::hypertrie::internal::raw {

	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	class NodeStorage {
	public:
		template<size_t depth>
		using SingleEntryNodeStorage_t = SpecificNodeStorage<depth, htt_t, SingleEntryNode, allocator_type>;
		template<size_t depth>
		using FullNodeStorage_t = SpecificNodeStorage<depth, htt_t, FullNode, allocator_type>;

		//TODO: change name
		template<size_t depth, template<size_t, typename, typename...> typename node_type>
		using SpecificNodes = std::conditional_t<
				(is_FullNode_v<node_type>),
				FullNodeStorage_t<depth>,
				SingleEntryNodeStorage_t<depth>>;

		template<size_t depth, template<size_t, typename, typename...> typename node_type>
		using SpecificNodePtr = typename SpecificNodes<depth, node_type>::node_pointer_type;
		//TODO: change name
		using SingleEntryNodes = template_library::integral_template_tuple<SingleEntryNodeStorage_t, (HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) ? 2 : 1, max_depth, allocator_type const &>;
		//TODO: change name
		using FullNodes = template_library::integral_template_tuple<FullNodeStorage_t, 1, max_depth, allocator_type const &>;

	private:
		SingleEntryNodes single_entry_nodes;
		FullNodes full_nodes;

	public:
		explicit NodeStorage(allocator_type const &alloc) noexcept : single_entry_nodes(alloc), full_nodes(alloc) {}

		template<size_t depth, template<size_t, typename, typename...> typename node_type>
		SpecificNodes<depth, node_type> &nodes() noexcept {
			if constexpr (is_FullNode_v<node_type>)
				return full_nodes.template get<depth>();
			else
				return single_entry_nodes.template get<depth>();
		}

		template<size_t depth, template<size_t, typename, typename...> typename node_type>
		[[nodiscard]] SpecificNodes<depth, node_type> const &nodes() const noexcept {
			if constexpr (is_FullNode_v<node_type>)
				return full_nodes.template get<depth>();
			else
				return single_entry_nodes.template get<depth>();
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
		void update_or_create_sen(RawIdentifier<depth, htt_t> identifier,
								  std::optional<SingleEntry<depth, htt_t>> &sen,
								  ssize_t delta_ref_count) noexcept {
			auto &nodes_ = this->template nodes<depth, SingleEntryNode>().nodes();
			auto &node_lifecycle_ = this->template nodes<depth, SingleEntryNode>().node_lifecycle();
			if (delta_ref_count != 0) {
				auto found = nodes_.find(identifier);
				if (found != nodes_.end()) {
					auto node_ptr = found->second;
					assert(ssize_t(node_ptr->ref_count()) + delta_ref_count >= 0);
					node_ptr->ref_count() += delta_ref_count;
					sen = SingleEntry<depth, htt_t>{node_ptr->key(), node_ptr->value()};
					if (node_ptr->ref_count() == 0UL) {
						node_lifecycle_.delete_(node_ptr);
						nodes_.erase(found);
					}
				} else {
					assert(delta_ref_count > 0);
					auto node = node_lifecycle_.new_(sen.value(), delta_ref_count);
					nodes_.insert(
							found,
							{identifier, node});
				}
			}
		}

		/**
		 * Looks up an specific Node, i.e. SingleEntryNode or FullNode, by means of an RawIdentifier. Result is returned as a pointer which might be an fancy pointer.
		 * If no Node for the given identifier exists, a null pointer is returned.
		 * @tparam depth
		 * @tparam node_type
		 * @param identifier
		 * @return
		 */
		template<size_t depth, template<size_t, typename, typename...> typename node_type>
		[[nodiscard]] SpecificNodePtr<depth, node_type> lookup(RawIdentifier<depth, htt_t> identifier) const noexcept {
			auto &nodes_ = this->nodes<depth, node_type>().nodes();
			auto found = nodes_.find(identifier);
			if (found != nodes_.end()) {
				return found->second;
			}
			return {};
		}

		/**
		 * Looks up a Node by means of a RawIdentifier. Result is returned wrapped into an NodeContainer.
		 * If no Node for the given identifier exists, an empty NodeContainer is returned.
		 * @tparam depth
		 * @param identifier
		 * @return
		 */
		template<size_t depth>
		[[nodiscard]] NodeContainer<depth, htt_t, allocator_type> lookup(RawIdentifier<depth, htt_t> identifier) const noexcept {
			if (identifier.empty()) {
				return {};
			}
			if (identifier.is_fn()) {
				auto fn_ptr = lookup<depth, FullNode>(identifier);
				if (fn_ptr != nullptr) {
					return FNContainer<depth, htt_t, allocator_type>{identifier, fn_ptr};
				}
			} else {// identifier.is_sen()
				auto sen_ptr = lookup<depth, SingleEntryNode>(identifier);
				if (sen_ptr != nullptr) {
					return SENContainer<depth, htt_t, allocator_type>{identifier, sen_ptr};
				}
			}
			return {};
		}
	};
}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_NODESTORAGE_HPP
