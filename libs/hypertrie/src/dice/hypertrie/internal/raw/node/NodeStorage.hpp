#ifndef HYPERTRIE_NODESTORAGE_HPP
#define HYPERTRIE_NODESTORAGE_HPP


#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "dice/hypertrie/internal/raw/node/CartesianNode.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"
#include "dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp"

#include "dice/template-library/integral_template_tuple.hpp"


namespace dice::hypertrie::internal::raw {

	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	class NodeStorage {
	public:
		template<size_t depth, template<size_t, typename, typename...> typename node_type>
		using SpecificNodeStorage_t = SpecificNodeStorage<depth, htt_t, node_type, allocator_type>;

		template<size_t depth, template<size_t, typename, typename...> typename node_type>
		using SpecificNodePtr = typename SpecificNodeStorage_t<depth, node_type>::node_pointer;

		template<size_t depth>
		using SingleEntryNodeStorage_t = SpecificNodeStorage_t<depth, SingleEntryNode>;

		template<size_t depth>
		using FullNodeStorage_t = SpecificNodeStorage_t<depth, FullNode>;

		template<size_t depth>
		using CartesianNodeStorage_t = SpecificNodeStorage_t<depth, CartesianNode>;

		using SingleEntryNodes = template_library::integral_template_tuple<HypertrieTrait_bool_valued_and_taggable_key_part<htt_t> ? 2UL : 1UL, max_depth, SingleEntryNodeStorage_t>;
		using FullNodes = template_library::integral_template_tuple<1UL, max_depth, FullNodeStorage_t>;
		using CartesianNodes = template_library::integral_template_tuple<2UL, max_depth, CartesianNodeStorage_t>;

	private:
		SingleEntryNodes single_entry_nodes;
		FullNodes full_nodes;
		CartesianNodes cartesian_nodes;

	public:
		explicit NodeStorage(allocator_type const &alloc) noexcept : single_entry_nodes{template_library::uniform_construct, alloc},
																	 full_nodes{template_library::uniform_construct, alloc},
																	 cartesian_nodes{template_library::uniform_construct, alloc} {
		}

		template<size_t depth, template<size_t, typename, typename...> typename node_type>
		SpecificNodeStorage_t<depth, node_type> &nodes() noexcept {
			if constexpr (is_FullNode_v<node_type>) {
				return full_nodes.template get<depth>();
			} else if constexpr (is_CartesianNode_v<node_type>) {
				static_assert(depth > 1,
							  "Cartesian nodes only exist at depths > 1");
				return cartesian_nodes.template get<depth>();
			} else {
				static_assert(depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>,
				        	  "SingleEntry nodes only exists on depth 1 in non-inplace configurations");
				return single_entry_nodes.template get<depth>();
			}
		}

		template<size_t depth, template<size_t, typename, typename...> typename node_type>
		[[nodiscard]] SpecificNodeStorage_t<depth, node_type> const &nodes() const noexcept {
			if constexpr (is_FullNode_v<node_type>) {
				return full_nodes.template get<depth>();
			} else if constexpr (is_CartesianNode_v<node_type>) {
				static_assert(depth > 1,
							  "Cartesian nodes only exist at depths > 1");
				return cartesian_nodes.template get<depth>();
			} else {
				static_assert(depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>,
							  "SingleEntry nodes only exists on depth 1 in non-inplace configurations");
				return single_entry_nodes.template get<depth>();
			}
		}

		template<size_t depth>
		SENPtr<depth, htt_t, allocator_type> create_sen(SingleEntry<depth, htt_t> const &entry,
														size_t ref_count) noexcept requires (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
			auto &sen_storage_ = this->template nodes<depth, SingleEntryNode>();
			auto &sen_lifecycle_ = sen_storage_.node_lifecycle();
			auto &sens_ = sen_storage_.nodes();

			assert(!sens_.contains(RawIdentifier<depth, htt_t>{entry}));

			auto sen_ptr = sen_lifecycle_.new_(entry, ref_count);
			sens_.insert(sen_ptr);
			return sen_ptr;
		}

		template<size_t depth>
		XNPtr<depth, htt_t, allocator_type> create_placeholder_cartesian(RawIdentifier<depth, htt_t> const &id) noexcept {
			auto &node_storage_ = this->template nodes<depth, CartesianNode>();
			auto node_ptr = node_storage_.node_lifecycle().new_(id, 0UL);
			[[maybe_unused]] auto [_, inserted] = node_storage_.nodes().insert(node_ptr);
			assert(inserted);

			return node_ptr;
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
		[[nodiscard]] SpecificNodePtr<depth, node_type> lookup(RawIdentifier<depth, htt_t> const &identifier) const noexcept {
			static_assert(!is_CartesianNode_v<node_type> || depth > 1,
						  "Cartesian nodes only exist at depths > 1");

			static_assert(!is_SingleEntryNode_v<node_type> || (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>),
						  "SingleEntry nodes only exists on depth 1 in non-inplace configurations");

			auto &nodes_ = this->template nodes<depth, node_type>().nodes();
			if (auto it = nodes_.find(identifier); it != nodes_.end()) {
				return *it;
			}

			return {};
		}

		/**
		 * Looks up a Node by means of a RawIdentifier. Result is returned wrapped into an NodePtr.
		 * If no Node for the given identifier exists, a null NodePtr is returned.
		 * @tparam depth
		 * @param identifier
		 * @return
		 */
		template<size_t depth>
		[[nodiscard]] NodePtr<depth, htt_t, allocator_type> lookup(RawIdentifier<depth, htt_t> identifier) const noexcept {
			if (identifier.empty()) {
				return {};
			}

			switch (identifier.tag()) {
				case IdentifierTag::FN: {
					if (auto fn_ptr = lookup<depth, FullNode>(identifier); fn_ptr == nullptr) {
						return fn_ptr;
					}
					return NodePtr<depth, htt_t, allocator_type>{};
				}
				case IdentifierTag::SEN: {
					if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						if (auto sen_ptr = lookup<depth, SingleEntryNode>(identifier); sen_ptr != nullptr) {
							return sen_ptr;
						}
						return NodePtr<depth, htt_t, allocator_type>{};
					} else {
						HYPERTRIE_UNREACHABLE;
					}
				}
				case IdentifierTag::XN: {
					if constexpr (depth > 1) {
						if (auto xn_ptr = lookup<depth, CartesianNode>(identifier); xn_ptr == nullptr) {
							return xn_ptr;
						}
						return NodePtr<depth, htt_t, allocator_type>{};
					} else {
						HYPERTRIE_UNREACHABLE;
					}
				}
				case IdentifierTag::Indeterminate: {
					HYPERTRIE_UNREACHABLE;
				}
			}
		}
	};
}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_NODESTORAGE_HPP
