#ifndef HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_IMPL_HPP
#define HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_IMPL_HPP

#include "dice/hypertrie/internal/container/deref_map_iterator.hpp"
#include "dice/hypertrie/internal/raw/node/NodeStorage.hpp"
#include "dice/hypertrie/internal/raw/node/RawIdentifier.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/CartesianUtil.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/CommonLvChanges.hpp"
#include "dice/hypertrie/internal/util/Unreachable.hpp"
#include "dice/template-library/integral_template_tuple.hpp"

#include <algorithm>
#include <vector>

namespace dice::hypertrie::internal::raw::node_context::common_detail {

	/**
	 * Used to indicated the origin of a node in `InsertImpl::insert_into_full_node`.
	 */
	enum struct NodeSource : bool {
		/**
		 * The node was used before but they don't need it anymore.
		 * So we can reuse it. New/changed entries must be tracked but old ones are already fine.
		 * If this is not the origin we need to take care of incrementing the ref_count of old nodes.
		 */
		RequestMove,

		/**
		 * The node has to be copied. Some child mappings need to be altered some added.
		 * For child mappings that stay the same the childs ref_count must be increased.
		 * Obviously, there is now a new node (this one) which references them.
		 */
		RequestCopy,
	};

	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct CommonChangeImpl {
		using key_part_type = typename htt_t::key_part_type;

		template<size_t depth>
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;

		using NodeStorage_t = NodeStorage<max_depth, htt_t, allocator_type>;

		template<size_t depth>
		using SingleEntry_t = SingleEntry<depth, htt_t>;

		template<size_t depth>
		using CartesianOperand_t = CartesianOperand<depth, htt_t>;

		template<size_t depth>
		using SENPtr_t = SENPtr<depth, htt_t, allocator_type>;

		template<size_t depth>
		static Map<key_part_type, std::vector<SingleEntry_t<depth - 1>>> entry_subsets_for_pos(std::vector<SingleEntry_t<depth>> const &entries, size_t pos) noexcept {
			Map<key_part_type, std::vector<SingleEntry_t<depth - 1>>> ret;

			for (auto const &e : entries) {
				ret[e.key()[pos]].emplace_back(e.key().subkey(pos), e.value());
			}

			return ret;
		}

		template<size_t depth>
		static std::array<Map<key_part_type, std::vector<SingleEntry_t<depth - 1>>>, depth> entry_subsets(std::vector<SingleEntry_t<depth>> const &entries) noexcept {
			std::array<Map<key_part_type, std::vector<SingleEntry_t<depth - 1>>>, depth> tmp;
			for (size_t pos = 0; pos < depth; ++pos) {
				tmp[pos] = entry_subsets_for_pos(entries, pos);
			}

			return tmp;
		}

		template<size_t fixed_depth, size_t depth>
		static std::vector<SingleEntry_t<depth - fixed_depth>> slice_entries(std::vector<SingleEntry_t<depth>> const &entries,
																			 RawSliceKey<fixed_depth, htt_t> const &slice_key,
																			 std::optional<size_t> size_hint = std::nullopt) noexcept {
			std::vector<SingleEntry<depth - fixed_depth, htt_t>> result_entries;
			if (size_hint.has_value()) {
				result_entries.reserve(*size_hint);
			}

			for (auto const &entry : entries) {
				if (auto sliced = slice_key.slice(entry.key()); sliced.has_value()) {
					result_entries.emplace_back(*sliced, entry.value());
				}
			}
			return result_entries;
		}

		template<size_t prefix_trim, size_t postfix_trim, size_t depth>
		static std::vector<SingleEntry_t<depth - prefix_trim - postfix_trim>> trim_entries(std::vector<SingleEntry_t<depth>> const &entries) {
			std::vector<SingleEntry_t<depth - prefix_trim - postfix_trim>> ret;
			ret.reserve(entries.size());

			for (auto const &entry : entries) {
				auto &new_entry = ret.emplace_back();
				std::copy(entry.key().begin() + prefix_trim, entry.key().end() - postfix_trim, new_entry.key().begin());

				if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
					new_entry.value_mut() = entry.value();
				}
			}

			return ret;
		}

		template<template<size_t> typename LvChanges_t, size_t depth>
		static void apply_rc_deltas(NodeStorage_t &node_storage,
									template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes) noexcept {

			auto const &lv_changes = all_lv_changes.template get<depth>();

			for (const auto &[id, delta] : lv_changes.rc_deltas) {
				if (delta == 0) {
					continue;
				}

				switch (id.tag()) {
					case IdentifierTag::FN: {
						auto &fn_storage_ = node_storage.template nodes<depth, FullNode>();
						auto &fns_ = fn_storage_.nodes();
						auto &fn_lifecycle_ = fn_storage_.node_lifecycle();

						if (auto fn_it = fns_.find(id); fn_it != fns_.end()) {
							auto fn_ptr = *fn_it;

							fn_ptr->ref_count() += delta;
							if (fn_ptr->ref_count() == 0) {
								// delete node

								if constexpr (depth > 1) {
									auto &direct_next_lv_changes = all_lv_changes.template get<depth - 1>();

									// adjust children refcounts
									for (size_t pos = 0; pos < depth; ++pos) {
										for (auto const &[_, child] : fn_ptr->edges(pos)) {
											if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
												if (child.is_sen()) {
													continue;
												}
											}

											direct_next_lv_changes.dec_ref(child.identifier());
										}
									}
								}

								fn_lifecycle_.delete_(fn_ptr);
								fns_.erase(fn_it);
							}
						}
						//else {
						// the only reason for this happening should
						// be that the node referred to by id was moved

						// !!!DO NOT USE THIS ASSERTION UNLESS YOU ARE WORKING WITH SMALL HYPERTRIES!!!
						//assert((std::find_if(lv_changes.fn_moves.begin(), lv_changes.fn_moves.end(), [id = id](auto const &move) {
						//		   return move.second == id;
						//	   })) != lv_changes.fn_moves.end());
						//}
						break;
					}
					case IdentifierTag::SEN: {
						if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
							auto &sen_storage_ = node_storage.template nodes<depth, SingleEntryNode>();
							auto &sens_ = sen_storage_.nodes();
							auto &sen_lifecycle_ = sen_storage_.node_lifecycle();

							auto sen_it = sens_.find(id);
							assert(sen_it != sens_.end());
							auto sen_ptr = *sen_it;

							sen_ptr->ref_count() += delta;
							if (sen_ptr->ref_count() == 0) {
								sen_lifecycle_.delete_(sen_ptr);
								sens_.erase(sen_it);
							}
						} else {
							HYPERTRIE_UNREACHABLE;
						}
						break;
					}
					case IdentifierTag::XN: {
						if constexpr (depth > 1) {
							auto &next_lv_changes = all_lv_changes.template subtuple<1UL, depth - 1>();

							auto &xn_storage_ = node_storage.template nodes<depth, CartesianNode>();
							auto &xns_ = xn_storage_.nodes();
							auto &xn_lifecycle_ = xn_storage_.node_lifecycle();

							auto xn_it = xns_.find(id);
							assert(xn_it != xns_.end());
							auto xn_ptr = *xn_it;

							xn_ptr->ref_count() += delta;
							if (xn_ptr->ref_count() == 0) {
								// delete node
								xn_ptr->for_each_operand([&]<size_t, size_t operand_depth>(NodePtr<operand_depth, htt_t, allocator_type> const operand) noexcept {
									if constexpr (operand_depth > 0) {
										if constexpr (operand_depth == 1 && HypertrieTrait_taggable_key_part<htt_t>) {
											if (operand.is_sen()) {
												return;
											}
										}

										next_lv_changes.template get<operand_depth>().dec_ref(operand.identifier());
									}
								});

								xn_lifecycle_.delete_(xn_ptr);
								xns_.erase(xn_it);
							}
						} else {
							HYPERTRIE_UNREACHABLE;
						}
						break;
					}
					case IdentifierTag::Indeterminate: {
						HYPERTRIE_UNREACHABLE;
					}
				}
			}
		}

		template<template<size_t> typename LvChanges_t, Operation op, size_t depth, typename F = decltype([]<NewNodeRcPolicy, size_t id_depth>(RawIdentifier_t<id_depth>) noexcept -> SENPtr_t<id_depth> { return nullptr; })>
		static void populate_placeholder_full_nodes(NodeStorage_t &node_storage,
													template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes,
													F &&sen_notify_func) noexcept {

			auto &fn_storage_ = node_storage.template nodes<depth, FullNode>();
			auto &fns_ = fn_storage_.nodes();
			auto const &lv_changes = all_lv_changes.template get<depth>();

			for (auto &&[_id, create_spec] : lv_changes.new_fns) {
				auto &&[fn, entries] = create_spec;

				if constexpr (depth == 1) {
					auto &edges = fn->edges();
					for (auto const &entry : entries) {
						if constexpr (HypertrieTrait_bool_valued<htt_t>) {
							edges.insert(entry.key()[0]);
						} else {
							edges.emplace(entry.key()[0], entry.value());
						}
					}
				} else {
					auto &next_lv_changes = all_lv_changes.template get<depth - 1>();

					fn->size() = entries.size();

					for (size_t pos = 0; pos < depth; ++pos) {
						auto &edges = fn->edges(pos);

						auto children_entries = entry_subsets_for_pos(entries, pos);
						for (auto &&[key_part, child_entries] : children_entries) {
							if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
								if (child_entries.size() == 1) {
									edges.emplace(key_part, NodePtr<depth - 1, htt_t, allocator_type>::encode_key_part(child_entries[0].key()[0]));
									continue;// inplace, no need to add node further down
								}
							}

							edges.emplace(key_part, next_lv_changes.add_node(std::move(child_entries), node_storage, sen_notify_func));
						}
					}
				}

				fns_.insert(fn);
			}
		}

		template<template<size_t> typename LvChanges_t, size_t depth, typename F = decltype([]<NewNodeRcPolicy, size_t id_depth>(RawIdentifier_t<id_depth>) noexcept -> SENPtr_t<id_depth> { return nullptr; })>
		static void populate_placeholder_cartesian_nodes(NodeStorage_t &node_storage,
														 template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes,
														 F &&sen_notify_func) noexcept requires (depth > 1) {

			auto &xn_storage_ = node_storage.template nodes<depth, CartesianNode>();
			auto &xns_ = xn_storage_.nodes();

			auto &lv_changes = all_lv_changes.template get<depth>();
			auto &next_lv_changes = all_lv_changes.template subtuple<1UL, depth - 1>();

			for (auto &&[id, create_spec] : lv_changes.new_xns) {
				auto &&[xn, operands] = create_spec;

				xn->size() = 1;

				for (size_t ix = 0; ix < depth; ++ix) {
					auto &&operand = operands[ix];

					std::move(operand).visit([&, xn = xn]<size_t operand_depth>(typename LvChanges_t<depth>::template VariantCartesianOperand_t<operand_depth> &&operand) noexcept {
						visit(util::Overloaded{
									  [&](CartesianOperand_t<operand_depth> &&operand_entries) noexcept {
										  if constexpr (operand_depth > 0) {
											  xn->size() *= operand_entries.size();
											  xn->discriminant().set(ix, operand_depth);

											  if constexpr (operand_depth == 1 && HypertrieTrait_taggable_key_part<htt_t>) {
												  if (operand_entries.size() == 1) {
													  xn->operand(ix) = NodePtr<1, htt_t, allocator_type>::encode_key_part(operand_entries[0].key()[0]);
													  return;
												  }
											  }

											  xn->operand(ix) = next_lv_changes.template get<operand_depth>().add_node(std::move(operand_entries), node_storage, sen_notify_func);
										  }
									  },
									  [&](PtrOperand<operand_depth, htt_t, allocator_type> &&operand) noexcept {
										  if constexpr (operand_depth > 0) {
											  xn->size() *= operand.size_hint;
											  xn->discriminant().set(ix, operand_depth);
											  xn->operand(ix) = operand.ptr;
										  }
									  }},
							  std::move(operand));
					});
				}

				xns_.insert(xn);
			}
		}

		template<template<size_t> typename LvChanges_t, size_t depth> requires (depth > 1)
		static void direct_delete_full_node(NodeStorage_t &node_storage,
											LvChanges_t<depth - 1> &direct_next_lv_changes,
											FNPtr<depth, htt_t, allocator_type> fn_ptr) noexcept {
			for (size_t pos = 0; pos < depth; ++pos) {
				for (auto const &[_, child] : fn_ptr->edges(pos)) {
					if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						if (child.is_sen()) {
							continue;
						}
					}

					direct_next_lv_changes.dec_ref(child.identifier());
				}
			}

			node_storage.template nodes<depth, FullNode>().node_lifecycle().delete_(fn_ptr);
		}
	};

} // namespace dice::hypertrie::internal::raw::node_context::common_detail

#endif//HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_IMPL_HPP
