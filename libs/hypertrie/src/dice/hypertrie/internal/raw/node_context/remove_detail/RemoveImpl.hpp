#ifndef HYPERTRIE_RAWNODECONTEXT_REMOVE_IMPL_HPP
#define HYPERTRIE_RAWNODECONTEXT_REMOVE_IMPL_HPP

#include <algorithm>
#include <vector>

#include "dice/hypertrie/internal/raw/node_context/common_detail/CartesianUtil.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/CommonChangeImpl.hpp"
#include "dice/hypertrie/internal/raw/node_context/remove_detail/LvChanges.hpp"
#include "dice/hypertrie/internal/raw/node_context/remove_detail/UpwardsLvChanges.hpp"

namespace dice::hypertrie::internal::raw::node_context::remove_detail {
	using namespace node_context::common_detail;

	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct RemoveImpl : CommonChangeImpl<max_depth, htt_t, allocator_type> {
		using super_t = CommonChangeImpl<max_depth, htt_t, allocator_type>;

		template<size_t depth>
		using LvChanges_t = LvChanges<depth, max_depth, htt_t, allocator_type>;

		using UpwardsLvChanges_t = AllUpwardsLvChanges<max_depth, htt_t, allocator_type>;

		template<size_t depth>
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;

		template<size_t depth>
		using RawKey_t = RawKey<depth, htt_t>;

		template<size_t depth>
		using SingleEntry_t = SingleEntry<depth, htt_t>;

		using key_part_type_t = typename htt_t::key_part_type;

		using NodeStorage_t = NodeStorage<max_depth, htt_t, allocator_type>;

		template<size_t depth>
		using NodePtr_t = NodePtr<depth, htt_t, allocator_type>;

		template<size_t depth>
		using FNPtr_t = FNPtr<depth, htt_t, allocator_type>;

		template<size_t depth>
		using XNPtr_t = XNPtr<depth, htt_t, allocator_type>;

		template<size_t depth>
		using SENPtr_t = SENPtr<depth, htt_t, allocator_type>;

		using Requester_t = Requester<max_depth, htt_t>;

		template<size_t depth>
		using CartesianOperand_t = CartesianOperand<depth, htt_t>;

		template<size_t depth>
		using RawIterator_t = RawIterator<depth, true, htt_t, allocator_type>;

		template<size_t depth>
		using SliceResult_t = SliceResult<depth, htt_t, allocator_type>;

		/**
		 * Used for functions that call lv_changes.add_node, is called when the function would create a sen.
		 * Because there might be a sen check for the entry.
		 * Need to reuse the same buffer because this and the sen check will resolve to the same
		 * node, and we cannot have two copies of it. Additionally, the sen from the sen check
		 * is already referenced by some edges, and we don't want to repair that.
		 */
		static auto make_sen_notify_func(AllUpwardsLvChanges<max_depth, htt_t, allocator_type> &upwards_lv_changes) noexcept {
			return [&upwards_lv_changes]<NewNodeRcPolicy rc_policy, size_t depth>(RawIdentifier_t<depth> const id) noexcept -> SENPtr_t<depth> {
				auto const &buffers = upwards_lv_changes.template for_depth<depth>().sen_buffers;
				if (auto it = buffers.find(id); it != buffers.end()) {
					if constexpr (rc_policy == NewNodeRcPolicy::Inc) {
						upwards_lv_changes.inc_sen_ref(id);
					}

					return it->second;
				}

				return nullptr;
			};
		}

		static auto find_surviving_child_depth_1(FNPtr_t<1> const fn, std::vector<SingleEntry_t<1>> const &to_remove) noexcept -> std::ranges::borrowed_iterator_t<decltype(fn->edges())> {
			Set<key_part_type_t> remove_set;
			for (SingleEntry_t<1> const &e : to_remove) {
				remove_set.insert(e.key()[0]);
			}

			return std::ranges::find_if(fn->edges(),
										[&](auto const &edge) {
											auto const edge_key_part = [&]() {
												if constexpr (htt_t::is_bool_valued) {
													return edge;
												} else {
													return edge.first;
												}
											}();

											return !remove_set.contains(edge_key_part);
										});
		}

		/**
		 * Removes entries from root node (nodec)
		 *
		 * Invariants:
		 * 		1. must not be called with entries that don't exist in nodec
		 *
		 * @param nodec node container for root node
		 * @param entries entries to remove
		 */
		template<size_t depth>
		static void exec(NodeStorage_t &node_storage,
						 NodePtr_t<depth> &node,
						 std::vector<SingleEntry_t<depth>> &&entries) noexcept {
			if (entries.empty() || node == nullptr) {
				return;
			}

			template_library::integral_template_tuple<1UL, depth, LvChanges_t> changes{};
			auto &direct_changes = changes.template get<depth>();
			UpwardsLvChanges_t upwards_lv_changes;

			// use ok because invariant 1
			auto const new_size = node.size() - entries.size();
			switch (new_size) {
				[[unlikely]] case 0: {
					if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						if (node.is_sen()) {
							node = NodePtr_t<1>{};
							return;
						}
					}

					direct_changes.dec_ref(node.identifier());
					node = NodePtr_t<depth>{};
					break;
				}
				[[unlikely]] case 1: {
					node = direct_changes.start_sen_check(node,
														  RawIdentifier_t<depth + 1>{}.retag_as_fn(),
														  {},
														  std::move(entries),
														  node_storage,
														  upwards_lv_changes);
					break;
				}
				[[likely]] default: {
					Requester_t this_requester{
							.who_asked = RawIdentifier_t<depth + 1>{}.retag_as_fn(),
							.who_asked_edge = {}};

					node = direct_changes.remove_from_node(node,
														   std::move(entries),
														   this_requester,
														   node_storage);
					break;
				}
			}

			direct_changes.template calculate_movables<Operation::Remove>(node_storage);
			apply_down<depth>(node_storage, changes, upwards_lv_changes);

			if (new_size == 0) {
				return;
			}

			{ // check for reassign
				auto const &remaining_changes = upwards_lv_changes.template for_depth<depth + 1>().edge_reassigns;
				if (auto const it = remaining_changes.find(RawIdentifier_t<depth + 1>{}.retag_as_fn()); it != remaining_changes.end()) {
					assert(it->second.size() == 1);
					auto const &reassign = it->second[0];
					node = static_cast<NodePtr_t<depth>>(reassign.new_child);
				}
			}

			if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				auto const &sen_replacements = upwards_lv_changes.template for_depth<2>().child_sen_replacements;
				if (auto const it = sen_replacements.find(RawIdentifier_t<depth + 1>{}.retag_as_fn()); it != sen_replacements.end()) {
					auto const &sen_replace = *sen_replacements.at(RawIdentifier_t<2>{}.retag_as_fn()).begin();
					node = NodePtr_t<1>::encode_key_part(sen_replace.replacement.key()[0]);
				}
			}
		}

		template<NewNodeRcPolicy rc_policy = NewNodeRcPolicy::Inc, size_t depth, size_t prefix_len, size_t postfix_len>
		static XNPtr_t<depth> calc_and_add_xfix_cartesian(NodeStorage_t &node_storage,
														  template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes,
														  RawIdentifier_t<depth> const id_after,
														  std::array<CartesianOperand_t<1>, prefix_len> &&prefix,
														  SliceResult_t<depth - prefix_len - postfix_len> &slice_result,
														  MaterializeEntries<depth - prefix_len - postfix_len, htt_t> auto &&mto_remove_entries,
														  std::array<CartesianOperand_t<1>, postfix_len> &&postfix) noexcept requires (depth > 1 && (prefix_len + postfix_len < depth && (prefix_len > 0 || postfix_len > 0)))  {

			assert(id_after.is_xn());
			assert(id_after != RawIdentifier_t<depth>{}.retag_as_xn());

			static constexpr size_t high_order_operand_depth = depth - prefix_len - postfix_len;
			auto &lv_changes = all_lv_changes.template get<depth>();
			auto &hoo_lv_changes = all_lv_changes.template get<high_order_operand_depth>();

			if constexpr (rc_policy == NewNodeRcPolicy::Inc) {
				lv_changes.inc_ref(id_after);
			}

			if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(id_after); xn_ptr != nullptr) {
				return xn_ptr;
			}

			auto const op_before = slice_result.as_node_ptr();
			auto to_remove_entries = mto_remove_entries();

			if (slice_result.ownership() == Ownership::ContextBorrowed) {
				// fast path
				// can only be used if the slice result has a valid identifier
				// as that is necessary to properly calculate the future id
				//
				// - this slice result is either a FN or XN
				// - ContextBorrowed nodes always have a valid id
				// - Owned XNs (which is the only option for here) will not have a valid id

				auto const size_after = op_before.size() - to_remove_entries.size();
				auto const op_after = [&]() noexcept {
					if (to_remove_entries.empty()) {
						assert(op_before.is_fn());
						hoo_lv_changes.inc_ref(op_before.identifier());

						return op_before;
					}

					Requester_t this_requester{.who_asked = id_after,
											   .who_asked_edge = {.pos = prefix_len, .key_part = {}}};
					return hoo_lv_changes
							.template remove_from_node<NodeBeforeRcPolicy::NoDec, IdentifierTag::FN>(op_before,
																									 std::move(to_remove_entries),
																									 this_requester,
																									 node_storage);
				}();

				PtrOperand<high_order_operand_depth, htt_t, allocator_type> const ptr_operand{
						.size_hint = size_after,
						.ptr = op_after};

				return lv_changes._add_xfix_cartesian(id_after, std::move(prefix), ptr_operand, std::move(postfix), node_storage);
			}

			// slow path
			// for cases where the slice result does not have a valid identifier
			Set<SingleEntry_t<high_order_operand_depth>> to_exclude; {
				for (auto const &entry : to_remove_entries) {
					to_exclude.insert(entry);
				}
			}

			std::vector<SingleEntry_t<high_order_operand_depth>> high_order_operand; {
				high_order_operand.reserve(op_before.size()); // definitely upper bound

				for (RawIterator_t<high_order_operand_depth> iter{op_before}; iter; ++iter) {
					if (!to_exclude.contains(*iter)) {
						high_order_operand.push_back(*iter);
					}
				}
			}

			return lv_changes._add_xfix_cartesian(id_after, std::move(prefix), std::move(high_order_operand), std::move(postfix), node_storage);
		}

		template<size_t depth>
		static auto find_surviving_edge(typename FullNode<depth, htt_t, allocator_type>::single_dim_edges_type const &edges,
										Map<typename htt_t::key_part_type, std::vector<SingleEntry_t<depth - 1>>> const &entry_subset) noexcept {

			auto const remove_count = [&](typename htt_t::key_part_type const kp) noexcept -> size_t {
				if (auto it = entry_subset.find(kp); it != entry_subset.end()) {
					return it->second.size();
				}

				return 0;
			};

			for (auto it = edges.begin(); it != edges.end(); ++it) {
				if (it->second.size() - remove_count(it->first) > 0) {
					return it;
				}
			}

			return edges.end();
		}

		template<NodeSource node_src, size_t depth>
		static void remove_from_full_node(NodeStorage_t &node_storage,
										  template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes,
										  UpwardsLvChanges_t &upwards_lv_changes,
										  FNPtr_t<depth> original,
										  RawIdentifier_t<depth> id_after,
										  typename LvChanges_t<depth>::Change &&change) noexcept {
			static constexpr size_t child_depth = depth - 1;

			auto &fn_storage_ = node_storage.template nodes<depth, FullNode>();
			auto &fn_lifecycle_ = fn_storage_.node_lifecycle();
			auto &fns_ = fn_storage_.nodes();

			auto &lv_changes = all_lv_changes.template get<depth>();

			auto const get_fn_ptr = [&]() noexcept {
				if constexpr (node_src == NodeSource::RequestCopy) {
					return fn_lifecycle_.new_(*original);
				} else {
					return original;
				}
			};

			// nodes with size_after == 0 don't need to be enqueued
			// nodes with size_after == 1 will be enqueued as SEN checks instead of normal changes
			assert(original->size() > change.entries.size() + 1);

			if constexpr (depth == 1) {
				// copy->size() is tracked automatically since we are at depth 1
				// no XNs exist at depth 1
				assert(id_after.is_fn());

				FNPtr_t<1> fn;

				if constexpr (node_src == NodeSource::RequestCopy) {
					if (auto fn_ptr = node_storage.template lookup<1, FullNode>(id_after); fn_ptr != nullptr) {
						upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, fn_ptr);
						return;
					}
				} else {
					// if node was moved fns_ should not contain
					// the future node as that would have made the move useless
					assert(!fns_.contains(id_after));
				}

				if (auto it = lv_changes.new_fns.find(id_after); it != lv_changes.new_fns.end()) {
					// invalid remove planning detected
					// need steal work from populate_placeholder_full_nodes
					// so that node with id_after is not created twice and subsequently leaked

					auto fn_ptr = it->second.placeholder;
					lv_changes.new_fns.erase(it);

					if constexpr (node_src == NodeSource::RequestCopy) {
						*fn_ptr = *original;
					} else /* node_src == NodeSource::RequestMove */ {
						*fn_ptr = std::move(*original);
						fn_lifecycle_.delete_(original); // reclaim moved-from node memory
														 // note: already detached from node storage
					}

					fn = fn_ptr;
				} else {
					fn = get_fn_ptr();
				}

				upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, fn);

				fn->hash() = id_after.hash();
				fn->ref_count() = 0;// ok because will be adjusted from lv_changes.delta_rc

				for (auto const &entry : change.entries) {
					fn->edges().erase(entry.key()[0]);
				}

				fns_.insert(fn);
			} else {
				// can become either FN or XN
				assert(id_after.is_fn() || id_after.is_indeterminate());

				FNPtr_t<depth> fn = nullptr;

				{
					auto const fn_id = id_after.retag_as_fn();

					// before copying try the best we can to avoid it
					if constexpr (node_src == NodeSource::RequestCopy) {
						// before copying try the best we can to avoid it

						if (auto fn_ptr = node_storage.template lookup<depth, FullNode>(fn_id); fn_ptr != nullptr) {
							if (id_after.is_indeterminate()) {
								lv_changes.reassign_refcount(id_after, fn_id);
							}
							upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, fn_ptr);
							return;
						}
					} else {
						// if node was moved fns_ should not contain
						// the future node as that would have made the move useless
						assert(!fns_.contains(fn_id));
					}

					if (auto it = lv_changes.new_fns.find(fn_id); it != lv_changes.new_fns.end()) {
						// invalid remove planning detected
						// need steal work from populate_placeholder_full_nodes
						// so that node with id_after is not created twice and subsequently leaked

						if (id_after.is_indeterminate()) {
							id_after = lv_changes.reassign_refcount(id_after, fn_id);
						}

						auto fn_ptr = it->second.placeholder;
						lv_changes.new_fns.erase(it);

						if constexpr (node_src == NodeSource::RequestCopy) {
							*fn_ptr = *original;
						} else /* node_src == NodeSource::RequestMove */ {
							*fn_ptr = std::move(*original);
							fn_lifecycle_.delete_(original); // reclaim moved-from node memory
															 // note: already detached from node storage
						}

						fn = fn_ptr;
					}
				}

				if (id_after.is_indeterminate()) {
					auto const xn_id = id_after.retag_as_xn();
					if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(xn_id); xn_ptr != nullptr) {
						lv_changes.reassign_refcount(id_after, xn_id);
						upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, xn_ptr);
						return;
					}
				}

				auto &direct_next_lv_changes = all_lv_changes.template get<child_depth>();
				auto entry_subsets = super_t::entry_subsets(change.entries);

				if (id_after.is_indeterminate()) {
					auto const future_edge_counts = calculate_future_edge_counts_after_removal(*original, entry_subsets);

					if constexpr (HypertrieTrait_bool_valued<htt_t>) {
						if (is_general_cartesian<htt_t>(future_edge_counts, original->size() - change.entries.size())) {
							id_after = lv_changes.reassign_refcount(id_after, id_after.retag_as_xn());

							auto const existing_operands = extract_operands(*original);
							auto result_operands = unmerge_operands<depth, htt_t, allocator_type>(existing_operands, entry_subsets);

							auto new_xn = lv_changes.template add_general_cartesian<NewNodeRcPolicy::NoInc>(id_after, std::move(result_operands), node_storage);
							upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, new_xn);

							if constexpr (node_src == NodeSource::RequestMove) {
								super_t::template direct_delete_full_node<LvChanges_t, depth>(node_storage, direct_next_lv_changes, original);
							}

							return;
						}
					}

					if (auto const xfix_props = try_get_xfix_cartesian_properties(future_edge_counts); xfix_props.has_value()) {
						id_after = lv_changes.reassign_refcount(id_after, id_after.retag_as_xn());

						xfix_props->template visit<depth>([&]<size_t prefix_len, size_t postfix_len>() noexcept {
							std::array<CartesianOperand_t<1>, prefix_len> prefix;
							std::array<CartesianOperand_t<1>, postfix_len> postfix;
							RawSliceKey<prefix_len + postfix_len, htt_t> slice_key;

							if constexpr (prefix_len > 0) {
								for (size_t ix = 0; ix < prefix_len; ++ix) {
									// Why can I just read the prefix here?:
									// all operands in the prefix have to be of size == 1
									// full nodes cannot have dimensions without entries
									// => the minimum size of a dimension is 1
									// => if it is size == 1 after it must have been size == 1 before
									// => I can just read the before value which must be equivalent to the after value
									auto const &edges = original->edges(ix);
									auto const edge = find_surviving_edge<depth>(edges, entry_subsets[ix]);
									assert(edge != edges.end());
									auto const key_part = edge->first;

									prefix[ix].emplace_back(SingleEntry_t<1>{{key_part}, typename htt_t::value_type{1}});
									slice_key[ix].pos = ix;
									slice_key[ix].key_part = key_part;
								}
							}

							if constexpr (postfix_len > 0) {
								for (size_t ix = 0; ix < postfix_len; ++ix) {
									// Same reason as for prefix
									auto const &edges = original->edges(depth - postfix_len + ix);
									auto const edge = find_surviving_edge<depth>(edges, entry_subsets[depth - postfix_len + ix]);
									assert(edge != edges.end());
									auto const key_part = edge->first;

									postfix[ix].emplace_back(SingleEntry_t<1>{{key_part}, typename htt_t::value_type{1}});
									slice_key[prefix_len + ix].pos = prefix_len + (depth - prefix_len - postfix_len) + ix;
									slice_key[prefix_len + ix].key_part = key_part;
								}
							}

							auto slice = slice_detail::slice<depth, prefix_len + postfix_len, htt_t, allocator_type>(original, slice_key);

							MaterializeEntries<depth - prefix_len - postfix_len, htt_t> auto sliced_entries = [&]() noexcept {
								return super_t::slice_entries(change.entries, slice_key);
							};

							auto new_xn = calc_and_add_xfix_cartesian<NewNodeRcPolicy::NoInc>(node_storage,
																							  all_lv_changes,
																							  id_after,
																							  std::move(prefix),
																							  slice,
																							  sliced_entries,
																							  std::move(postfix));
							upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, new_xn);
						});

						if constexpr (node_src == NodeSource::RequestMove) {
							super_t::template direct_delete_full_node<LvChanges_t, depth>(node_storage, direct_next_lv_changes, original);
						}

						return;
					}

					id_after = lv_changes.reassign_refcount(id_after, id_after.retag_as_fn());
				}

				assert(id_after.is_fn());

				if (fn == nullptr) {
					fn = get_fn_ptr();
				}

				upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, fn);

				fn->hash() = id_after.hash();
				fn->ref_count() = 0;// ok because will be adjusted by apply_rc_deltas later

				assert(id_after.size() == fn->size() - change.entries.size());
				fn->size() -= change.entries.size();// ok because exec invariant 1

				// apply changes to copy
				for (size_t pos = 0; pos < depth; ++pos) {
					auto &edges = fn->edges(pos);
					auto &entry_subset = entry_subsets[pos];

					if constexpr (node_src == NodeSource::RequestCopy) {
						for (auto const &[key_part, child] : edges) {
							if constexpr (child_depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
								if (child.is_sen()) {
									continue;
								}
							}

							if (entry_subset.contains(key_part)) {
								continue; // ignore children with changes
							}

							direct_next_lv_changes.inc_ref(child.identifier());// copy is now referencing child
						}
					}

					for (auto &&[key_part, subset] : entry_subset) {
						assert(!subset.empty());
						auto it = edges.find(key_part);
						assert(it != edges.end());
						auto &child = container::deref(it);

						static constexpr auto rc_policy = node_src == NodeSource::RequestMove
																  ? NodeBeforeRcPolicy::Dec
																  : NodeBeforeRcPolicy::NoDec;

						// use ok because exec invariant 1
						switch (child.size() - subset.size()) {
							[[unlikely]] case 0: {
								auto const child_ptr_copy = child;
								edges.erase(it);

								if constexpr (rc_policy == NodeBeforeRcPolicy::Dec) {
									if constexpr (child_depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
										if (child_ptr_copy.is_sen()) {
											continue;
										}
									}

									direct_next_lv_changes.dec_ref(child_ptr_copy.identifier());
								}
								break;
							}
							[[unlikely]] case 1: {
								// child will become SEN (need to query single entry to replace)
								child = direct_next_lv_changes.template start_sen_check<rc_policy>(child,
																								   id_after,
																								   {pos, key_part},
																								   std::move(subset),
																								   node_storage,
																								   upwards_lv_changes);
								break;
							}
							[[likely]] default: {
									Requester_t this_requester{
											.who_asked = id_after,
											.who_asked_edge = BoundPos<htt_t>{.pos = pos, .key_part = key_part}};

									child = direct_next_lv_changes.template remove_from_node<rc_policy>(child,
																										std::move(subset),
																										this_requester,
																										node_storage);
								break;
							}
						}
					}
				}

				fns_.insert(fn);
			}
		}

		template<size_t depth>
		static void remove_from_cartesian_node(NodeStorage_t &node_storage,
											   template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes,
											   UpwardsLvChanges_t &upwards_lv_changes,
											   XNPtr_t<depth> original,
											   RawIdentifier_t<depth> id_after,
											   typename LvChanges_t<depth>::Change &&change) noexcept requires (depth > 1) {
			auto &xn_storage_ = node_storage.template nodes<depth, CartesianNode>();
			auto &xns_ = xn_storage_.nodes();
			auto &xn_lifecycle_ = xn_storage_.node_lifecycle();
			auto &lv_changes = all_lv_changes.template get<depth>();

			{
				auto const fn_id = id_after.retag_as_fn();
				if (auto fn_ptr = node_storage.template lookup<depth, FullNode>(fn_id); fn_ptr != nullptr) {
					lv_changes.reassign_refcount(id_after.retag_as_indeterminate(), fn_id);
					upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, fn_ptr);
					return;
				}
			}

			{
				auto const xn_id = id_after.retag_as_xn();
				if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(xn_id); xn_ptr != nullptr) {
					lv_changes.reassign_refcount(id_after.retag_as_indeterminate(), xn_id);
					upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, xn_ptr);
					return;
				}
			}

			auto const fallback_create_node = [&]() noexcept -> NodePtr_t<depth> {
				// calculate exclude set
				Set<SingleEntry_t<depth>> to_exclude; {
					to_exclude.reserve(change.entries.size());
					for (auto const &entry : change.entries) {
						to_exclude.insert(entry);
					}
				}

				std::vector<SingleEntry_t<depth>> new_entries;
				new_entries.reserve(original->size() - to_exclude.size());

				// iterate all entries, include if not in exclude set
				for (RawIterator_t<depth> xn_iter{original}; xn_iter; ++xn_iter) {
					if (!to_exclude.contains(*xn_iter)) {
						new_entries.push_back(*xn_iter);
					}
				}

				return lv_changes.template add_node<NewNodeRcPolicy::NoInc>(std::move(new_entries), node_storage, make_sen_notify_func(upwards_lv_changes));
			};

			if (original->is_general_cartesian()) {
				assert(!id_after.is_sen());
				// TODO: potential optimization here if node after is also general cartesian
				auto const new_node = fallback_create_node();
				lv_changes.reassign_refcount(id_after.retag_as_indeterminate(), new_node.identifier());
				upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, new_node);
			} else {
				// is xfix
				// this node will either stay xfix or generalize to a general cartesian
				// importantly it cannot become a full node because that would require
				// extending the common pre/postfix which is not possible because we are not inserting
				assert(id_after.is_indeterminate() || id_after.is_xn());
				id_after = lv_changes.reassign_refcount(id_after, id_after.retag_as_xn());

				auto const high_order_operand_ix = original->get_xfix_high_order_operand_index();
				assert(high_order_operand_ix.has_value());

				XFixCartesianProperties const props{.prefix_len = *high_order_operand_ix,
													.postfix_len = original->n_operands() - *high_order_operand_ix - 1};

				props.template visit<depth>([&]<size_t prefix_len, size_t postfix_len>() noexcept {
					static constexpr size_t high_order_operand_depth = depth - prefix_len - postfix_len;
					auto to_remove_entries = super_t::template trim_entries<prefix_len, postfix_len>(change.entries);

					if constexpr (high_order_operand_depth > 1) {
						// try to figure out if this node will generalize by analyzing the high order operand

						auto const entry_subsets = super_t::entry_subsets(to_remove_entries);
						auto const child_ptr = static_cast<NodePtr_t<high_order_operand_depth>>(original->operand(*high_order_operand_ix));
						auto const future_edge_counts = calculate_future_edge_counts_after_removal(*child_ptr.template specific_ptr<FullNode>(),
																								   entry_subsets);

						if constexpr (HypertrieTrait_bool_valued<htt_t>) {
							if (is_general_cartesian<htt_t>(future_edge_counts, child_ptr.size() - to_remove_entries.size())) {
								auto new_node = fallback_create_node();
								upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, new_node);
								return;
							}
						}

						if (try_get_xfix_cartesian_properties(future_edge_counts).has_value()) {
							auto new_node = fallback_create_node();
							upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, new_node);
							return;
						}
					}

					auto &hoo_lv_changes = all_lv_changes.template get<high_order_operand_depth>();

					// xfix will not generalize => fast path
					auto xn_ptr = xn_lifecycle_.new_(*original);
					xn_ptr->hash() = id_after.hash();
					xn_ptr->ref_count() = 0;
					assert(id_after.size() == xn_ptr->size() - change.entries.size());
					xn_ptr->size() -= change.entries.size();
					xn_ptr->for_each_operand([&]<size_t ix, size_t operand_depth>(NodePtr_t<operand_depth> &operand) noexcept {
						if constexpr (operand_depth > 0) {
							if constexpr (ix == prefix_len) {
								if constexpr (operand_depth == high_order_operand_depth) {
									Requester_t this_requester{.who_asked = id_after,
															   .who_asked_edge = {.pos = ix, .key_part = {}}};

									operand = hoo_lv_changes.template remove_from_node<NodeBeforeRcPolicy::NoDec, IdentifierTag::FN>(operand,
																																	 std::move(to_remove_entries),
																																	 this_requester,
																																	 node_storage);
								} else {
									HYPERTRIE_UNREACHABLE;
								}
							} else if constexpr (operand_depth == 1) {
								if constexpr (!HypertrieTrait_taggable_key_part<htt_t>) {
									all_lv_changes.template get<1>().inc_ref(operand.identifier());
								}
							} else {
								HYPERTRIE_UNREACHABLE;
							}
						}
					});

					upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, xn_ptr);
					xns_.insert(xn_ptr);
				});
			}
		}

		template<size_t depth>
		static void process_sen_checks(template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes,
									   UpwardsLvChanges_t &upwards_lv_changes) noexcept {

			auto const &lv_changes = all_lv_changes.template get<depth>();

			for (auto const &[node_before, all_sen_checks] : lv_changes.sen_checks) {
				switch (node_before.tag()) {
					case IdentifierTag::FN: {
						auto fn_ptr = node_before.template specific_ptr<FullNode>();

						for (auto const &[_, sen_checks] : all_sen_checks) {
							if constexpr (depth > 1) {
								auto &direct_next_lv_changes = all_lv_changes.template get<depth - 1>();
								auto subsets = super_t::entry_subsets_for_pos(sen_checks.to_remove, 0);

								// surviving entry must be accessible through dim 1
								// so only forward to these children
								for (auto const &[key_part, child] : fn_ptr->edges(0)) {
									auto const &subset = subsets[key_part]; // note to self: this has to be operator[] because sometimes need default

									if (child.size() - subset.size() == 1) {
										// must be the surviving child
										for (auto const &sen_check : sen_checks.sen_checks) {
											direct_next_lv_changes.forward_sen_check(child, sen_check, key_part, subset);
										}
										break;
									}
								}
							} else {
								auto const surviving_child = find_surviving_child_depth_1(fn_ptr, sen_checks.to_remove);
								assert(surviving_child != fn_ptr->edges().end());

								auto const entry = [&]() {
									if constexpr (htt_t::is_bool_valued) {
										return SingleEntry_t<1>{RawKey_t<1>{*surviving_child}, true};
									} else {
										return SingleEntry_t<1>{RawKey_t<1>{surviving_child->first}, surviving_child->second};
									}
								}();

								for (auto const &sen_check : sen_checks.sen_checks) {
									sen_check.who_asked.visit([&]<size_t parent_depth>(RawIdentifier_t<parent_depth> const &parent_id) {
										assert(parent_id.is_fn());

										if constexpr (parent_depth > depth) {
											if constexpr (parent_depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
												upwards_lv_changes.answer_sen_check(parent_id,
																					sen_check.who_asked_child,
																					entry);
											} else {
												std::copy(entry.key().begin(), entry.key().end(), &sen_check.path[sen_check.write_ix]);

												if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
													if (sen_check.value != nullptr) {
														*sen_check.value = entry.value();
													}
												}
											}
										} else {
											HYPERTRIE_UNREACHABLE;
										}
									});
								}
							}
						}

						break;
					}
					case IdentifierTag::SEN: {
						auto const entry = [&, node_before = node_before]() noexcept {
							if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
								return SingleEntry_t<1>{{node_before.decode_key_part()}};
							} else {
								auto sen_ptr = node_before.template specific_ptr<SingleEntryNode>();
								return SingleEntry_t<depth>{sen_ptr->key(), sen_ptr->value()};
							}
						}();

						for (auto const &[_, sen_checks] : all_sen_checks) {
							assert(sen_checks.to_remove.empty());
							for (auto const &sen_check : sen_checks.sen_checks) {
								// immediately reply because full answer is available
								sen_check.who_asked.visit([&]<size_t parent_depth>(RawIdentifier_t<parent_depth> const &parent_id) {
									assert(parent_id.is_fn());

									if constexpr (parent_depth > depth) {
										if constexpr (parent_depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
											upwards_lv_changes.answer_sen_check(parent_id,
																				sen_check.who_asked_child,
																				entry);
										} else {
											std::copy(entry.key().begin(), entry.key().end(), &sen_check.path[sen_check.write_ix]);

											if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
												if (sen_check.value != nullptr) {
													*sen_check.value = entry.value();
												}
											}
										}
									} else {
										HYPERTRIE_UNREACHABLE;
									}
								});
							}
						}

						break;
					}
					case IdentifierTag::XN: {
						if constexpr (depth > 1) {
							auto &next_lv_changes = all_lv_changes.template subtuple<1UL, depth - 1>();
							auto const xn_ptr = node_before.template specific_ptr<CartesianNode>();
							auto const existing_operands = extract_operands(*xn_ptr);

							for (auto const &pair : all_sen_checks) {
								auto const &sen_checks = pair.second;

								if constexpr (HypertrieTrait_bool_valued<htt_t>) {
									if (xn_ptr->is_general_cartesian()) {
										auto const sizes = get_operand_sizes<depth, htt_t, allocator_type>(existing_operands);
										auto const to_remove_operands = inverse_cartesian_product2(sen_checks.to_remove, sizes);

										for (auto const &sen_check : sen_checks.sen_checks) {
											xn_ptr->for_each_operand([&, write_off = size_t{0}]<size_t ix, size_t operand_depth>(NodePtr_t<operand_depth> const &operand) mutable noexcept {
												if constexpr (operand_depth == 1) {
													if (operand.is_sen()) {// size == 1
														auto const key_part = [&]() noexcept {
															if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
																return operand.decode_key_part();
															} else {
																auto const sen_ptr = operand.template specific_ptr<SingleEntryNode>();
																return sen_ptr->key()[0];
															}
														}();

														sen_check.path[sen_check.write_ix + write_off] = key_part;
													} else {
														next_lv_changes
																.template get<1>()
																.template forward_sen_check_to_cartesian_operand<SENCheckValuePolicy::IgnoreValue>(operand,
																																				   sen_check,
																																				   write_off,
																																				   to_remove_operands[ix]);
													}
												} else {
													HYPERTRIE_UNREACHABLE;
												}

												write_off += operand_depth;
											});
										}

										continue;
									}
								}

								auto const high_order_operand_ix = xn_ptr->get_xfix_high_order_operand_index();
								assert(high_order_operand_ix.has_value()); // node must either be xfix or general

								XFixCartesianProperties const props{.prefix_len = *high_order_operand_ix,
																	.postfix_len = xn_ptr->n_operands() - *high_order_operand_ix - 1};

								props.visit<depth>([&]<size_t prefix_len, size_t postfix_len>() noexcept {
									auto const to_remove_from_high_order_operand = super_t::template trim_entries<prefix_len, postfix_len>(sen_checks.to_remove);

									for (auto const &sen_check : sen_checks.sen_checks) {
										xn_ptr->for_each_operand([&, write_off = size_t{0}]<size_t ix, size_t operand_depth>(NodePtr_t<operand_depth> const &operand) mutable noexcept {
											if constexpr (operand_depth > 0) {
												if constexpr (ix == prefix_len) {
													if constexpr (operand_depth == depth - prefix_len - postfix_len) {
														assert(!operand.is_sen());
														next_lv_changes
																.template get<operand_depth>()
														        .template forward_sen_check_to_cartesian_operand<SENCheckValuePolicy::WriteValue>(operand,
																																				  sen_check,
																																				  write_off,
																																				  to_remove_from_high_order_operand);
													} else {
														HYPERTRIE_UNREACHABLE;
													}
												} else {
													if constexpr (operand_depth == 1) {
														assert(operand.is_sen());

														auto const key_part = [&]() noexcept {
															if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
																return operand.decode_key_part();
															} else {
																auto const sen_ptr = operand.template specific_ptr<SingleEntryNode>();
																return sen_ptr->key()[0];
															}
														}();

														sen_check.path[sen_check.write_ix + write_off] = key_part;
													} else {
														HYPERTRIE_UNREACHABLE;
													}
												}

												write_off += operand_depth;
											}
										});
									}
								});
							}

							break;
						} else {
							HYPERTRIE_UNREACHABLE;
						}
					}
					case IdentifierTag::Indeterminate: {
						HYPERTRIE_UNREACHABLE;
					}
				}
			}
		}

		template<size_t start_depth, size_t depth>
		static void apply_down(NodeStorage_t &node_storage,
							   template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes,
							   UpwardsLvChanges_t &upwards_lv_changes) noexcept {

			auto &fns_ = node_storage.template nodes<depth, FullNode>().nodes();
			auto &lv_changes = all_lv_changes.template get<depth>();

			process_sen_checks(all_lv_changes, upwards_lv_changes);

			for (auto &&[node_before, changesets] : lv_changes.node_changes) {
				assert(!node_before.is_sen());// check for accidental sen enqueue

				for (auto &&[id_after, change] : changesets) {
					assert(id_after != RawIdentifier_t<depth>{});// check for accidental empty after node enqueue

					if (decltype(lv_changes.fn_moves.end()) it; (it = lv_changes.fn_moves.find(id_after.retag_as_indeterminate())) != lv_changes.fn_moves.end() ||
																(it = lv_changes.fn_moves.find(id_after.retag_as_fn())) != lv_changes.fn_moves.end()) {
						// the condition implies that this node will definitely be created using a move,
						// but it does not imply that this node is the one getting moved for that purpose.
						// When this node is not the one getting moved this changeset is effectively just dropped
						// because it is unnecessary to evaluate for the purpose of creating a new node.
						// But of course we can have some requesters attached here, so we need to move them over
						// to the changeset that is actually getting processed.

						NodePtr_t<depth> const move_node_before{it->second};
						RawIdentifier_t<depth> const move_id_after{it->first};

						// Is the found changeset a different one than the current one?
						if (move_node_before != node_before || move_id_after != id_after) {
							// move requesters to changeset that will actually be evaluated

							auto &target_reqs = [&]() -> auto & {
								auto outer_it = lv_changes.node_changes.find(move_node_before);
								assert(outer_it != lv_changes.node_changes.end());

								auto inner_it = outer_it->second.find(move_id_after);
								assert(inner_it != outer_it->second.end());

								return inner_it->second.requesters;
							}();

							target_reqs.reserve(target_reqs.size() + change.requesters.size());
							std::copy(change.requesters.begin(), change.requesters.end(), std::back_inserter(target_reqs));
						}

						if (id_after.is_indeterminate() && it->first.is_fn()) {
							lv_changes.reassign_refcount(id_after, it->first);
						}

						continue;
					}

					switch (node_before.tag()) {
						case IdentifierTag::FN: {
							auto fn_ptr = node_before.template specific_ptr<FullNode>();
							remove_from_full_node<NodeSource::RequestCopy>(node_storage,
																		   all_lv_changes, upwards_lv_changes,
																		   fn_ptr, id_after, std::move(change));
							break;
						}
						case IdentifierTag::XN: {
							if constexpr (depth > 1) {
								auto xn_ptr = node_before.template specific_ptr<CartesianNode>();
								remove_from_cartesian_node(node_storage,
														   all_lv_changes, upwards_lv_changes,
														   xn_ptr, id_after, std::move(change));
								break;
							} else {
								HYPERTRIE_UNREACHABLE;
							}
						}
						default: {
							HYPERTRIE_UNREACHABLE;
						}
					}
				}
			}

			for (auto const &[id_after, fn_before] : lv_changes.fn_moves) {
				assert(id_after.is_fn() || id_after.is_indeterminate());
				assert(!fns_.contains(id_after.retag_as_fn()));

				auto fn_it = fns_.find(fn_before);
				assert(fn_it != fns_.end());

				// detach node
				fns_.erase(fn_it);

				auto &&change = lv_changes.node_changes.find(fn_before)->second.find(id_after)->second;
				remove_from_full_node<NodeSource::RequestMove>(node_storage,
															   all_lv_changes, upwards_lv_changes,
															   fn_before, id_after, std::move(change));
			}

			super_t::template populate_placeholder_full_nodes<LvChanges_t, Operation::Remove>(node_storage, all_lv_changes, make_sen_notify_func(upwards_lv_changes));

			if constexpr (depth > 1) {
				super_t::template populate_placeholder_cartesian_nodes<LvChanges_t>(node_storage, all_lv_changes, make_sen_notify_func(upwards_lv_changes));
			}

			super_t::template apply_rc_deltas<LvChanges_t>(node_storage, all_lv_changes);

			if constexpr (depth > 1) {
				auto &direct_next_lv_changes = all_lv_changes.template get<depth - 1>();
				direct_next_lv_changes.template calculate_movables<Operation::Remove>(node_storage);

				auto &next_lv_changes = all_lv_changes.template subtuple<1, depth - 1>();
				apply_down<start_depth, depth - 1>(node_storage, next_lv_changes, upwards_lv_changes);
			} else if constexpr (!HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				apply_up<start_depth, 1>(node_storage, upwards_lv_changes);
			} else if constexpr (start_depth >= 2) {
				apply_up<start_depth, 2>(node_storage, upwards_lv_changes);
			}
		}

		template<size_t start_depth, size_t depth>
		static void apply_up(NodeStorage_t &node_storage, UpwardsLvChanges_t const &upwards_lv_changes) noexcept {
			auto &changes = upwards_lv_changes.template for_depth<depth>();

			if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				auto &sens_ = node_storage.template nodes<depth, SingleEntryNode>().nodes();

				for (auto const &[sen_id, sen_buf] : changes.sen_buffers) {
					assert(!sens_.contains(sen_id));
					sens_.emplace(sen_buf);
				}
			}

			if constexpr (depth > 1) {
				for (auto const &[id_after, sen_replaces] : changes.child_sen_replacements) {
					switch (id_after.tag()) {
						case IdentifierTag::FN: {
							if constexpr (depth == 2 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
								auto fn_ptr = node_storage.template lookup<depth, FullNode>(id_after);
								assert(fn_ptr != nullptr);

								for (auto const &sen_replace : sen_replaces) {
									auto [found, child_it] = fn_ptr->find(sen_replace.child_to_replace.pos, sen_replace.child_to_replace.key_part);
									assert(found);
									container::deref(child_it) = NodePtr_t<1>::encode_key_part(sen_replace.replacement.key()[0]);
								}
								break;
							} else {
								HYPERTRIE_UNREACHABLE;
							}
						}
						case IdentifierTag::XN: {
							if constexpr (depth > 1 && HypertrieTrait_taggable_key_part<htt_t>) {
								auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(id_after);
								assert(xn_ptr != nullptr);

								for (auto const &sen_replace : sen_replaces) {
									assert(xn_ptr->discriminant()[sen_replace.child_to_replace.pos] == 1);
									xn_ptr->operand(sen_replace.child_to_replace.pos) = NodePtr_t<1>::encode_key_part(sen_replace.replacement.key()[0]);
								}
								break;
							} else {
								HYPERTRIE_UNREACHABLE;
							}
						}
						default: {
							// sens cannot have sen replacements
							// indeterminate nodes are not supposed to be enqueued
							HYPERTRIE_UNREACHABLE;
						}
					}
				}

				for (auto const &[id, edge_reassigns] : changes.edge_reassigns) {
					switch (id.tag()) {
						case IdentifierTag::FN: {
							auto fn_ptr = node_storage.template lookup<depth, FullNode>(id);
							for (auto const &[pos, new_child] : edge_reassigns) {
								auto [found, child_it] = fn_ptr->find(pos.pos, pos.key_part);
								assert(found);
								container::deref(child_it) = new_child;
							}
							break;
						}
						case IdentifierTag::XN: {
							auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(id);
							for (auto const &[pos, new_child] : edge_reassigns) {
								auto &operand = xn_ptr->operand(pos.pos);
								operand = new_child;
							}
							break;
						}
						default: {
							HYPERTRIE_UNREACHABLE;
						}
					}
				}
			}

			if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				for (auto const &[id, delta] : changes.sen_rc_deltas) {
					auto sen_ptr = node_storage.template lookup<depth, SingleEntryNode>(id);
					assert(sen_ptr != nullptr);
					sen_ptr->ref_count() += delta;
				}
			}

			if constexpr (depth < start_depth) {
				apply_up<start_depth, depth + 1>(node_storage, upwards_lv_changes);
			}
		}
	};

} // namespace dice::hypertrie::internal::raw::node_context::remove_detail

#endif // HYPERTRIE_RAWNODECONTEXT_REMOVE_IMPL_HPP
