#ifndef HYPERTRIE_INSERT_IMPL_RAWNODECONTEXT_HPP
#define HYPERTRIE_INSERT_IMPL_RAWNODECONTEXT_HPP

#include "dice/hypertrie/internal/container/deref_map_iterator.hpp"
#include "dice/hypertrie/internal/raw/iteration/RawIterator.hpp"
#include "dice/hypertrie/internal/raw/node/NodeStorage.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/CartesianUtil.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/CommonChangeImpl.hpp"
#include "dice/hypertrie/internal/raw/node_context/insert_detail/LvChanges.hpp"
#include "dice/hypertrie/internal/raw/node_context/insert_detail/UpwardsLvChanges.hpp"
#include "dice/hypertrie/internal/raw/node_context/slice_detail/SliceImpl.hpp"
#include "dice/template-library/switch_cases.hpp"

#include <algorithm>

namespace dice::hypertrie::internal::raw::node_context::insert_detail {
	using namespace node_context::common_detail;

	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct InsertImpl : CommonChangeImpl<max_depth, htt_t, allocator_type> {
		using super_t = CommonChangeImpl<max_depth, htt_t, allocator_type>;

		// section bind constant template arguments
		using key_part_type = typename htt_t::key_part_type;

		template<size_t depth>
		using LvChanges_t = LvChanges<depth, max_depth, htt_t, allocator_type>;

		using UpwardsLvChanges_t = AllUpwardsLvChanges<max_depth, htt_t, allocator_type>;

		template<size_t depth>
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;

		using NodeStorage_t = NodeStorage<max_depth, htt_t, allocator_type>;

		template<size_t depth>
		using SingleEntry_t = SingleEntry<depth, htt_t>;

		template<size_t depth>
		using NodePtr_t = NodePtr<depth, htt_t, allocator_type>;

		template<size_t depth>
		using XNPtr_t = XNPtr<depth, htt_t, allocator_type>;

		template<size_t depth>
		using FNPtr_t = FNPtr<depth, htt_t, allocator_type>;

		using Requester_t = Requester<max_depth, htt_t>;

		template<size_t depth>
		using RawIterator_t = RawIterator<depth, true, htt_t, allocator_type>;
		// end section


		/**
		 * @brief Apply insertion of `entries` into node `nodec`.
		 *
		 * @param nodec node to insert into
		 * @param entries to insert
		 */
		template<size_t depth>
		static void exec(NodeStorage_t &node_storage,
						 NodePtr_t<depth> &node,
						 std::vector<SingleEntry_t<depth>> &&entries) noexcept {
			if (entries.empty()) {
				return;
			}

			template_library::integral_template_tuple<1UL, depth, LvChanges_t> changes{};
			auto &direct_changes = changes.template get<depth>();
			UpwardsLvChanges_t upwards_lv_changes;

			if (node == nullptr) {
				if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					if (entries.size() == 1) {
						node = NodePtr_t<1>::encode_key_part(entries[0].key()[0]);
						return;
					}
				}

				node = direct_changes.add_node(std::move(entries), node_storage, {});
			} else {
				Requester_t this_requester{
						.who_asked = RawIdentifier<depth + 1, htt_t>{},
						.who_asked_edge = {}};

				node = direct_changes.insert_into_node(node, std::move(entries), this_requester, node_storage);
			}

			direct_changes.template calculate_movables<Operation::Insert>(node_storage);
			apply_down<depth>(node_storage, changes, upwards_lv_changes);

			auto const &remaining_changes = upwards_lv_changes.template for_depth<depth + 1>().edge_reassigns;
			if (auto const it = remaining_changes.find(RawIdentifier_t<depth + 1>{}); it != remaining_changes.end()) {
				assert(it->second.size() == 1);
				auto const &reassign = it->second[0];
				node = static_cast<NodePtr_t<depth>>(reassign.new_child);
			}
		}

		template<size_t start_depth, size_t depth>
		static void apply_down(NodeStorage_t &node_storage,
							   template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes,
							   UpwardsLvChanges_t &upwards_lv_changes) noexcept {

			auto &fns_ = node_storage.template nodes<depth, FullNode>().nodes();
			auto &lv_changes = all_lv_changes.template get<depth>();

			for (auto &&[node_before, changes] : lv_changes.node_changes) {
				for (auto &&[id_after, change] : changes) {
					if (decltype(lv_changes.fn_moves.end()) it; (it = lv_changes.fn_moves.find(id_after.retag_as_indeterminate())) != lv_changes.fn_moves.end() ||
																(it = lv_changes.fn_moves.find(id_after.retag_as_fn())) != lv_changes.fn_moves.end()) {
						// this condition implies that this node will definitely be created using a move,
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

						if (id_after.is_indeterminate() && move_id_after.is_fn()) {
							lv_changes.reassign_refcount(id_after, move_id_after);
						}

						continue;
					}

					switch (node_before.tag()) {
						case IdentifierTag::FN: {
							auto fn_ptr = node_before.template specific_ptr<FullNode>();
							insert_into_full_node<depth, NodeSource::RequestCopy>(node_storage, all_lv_changes, upwards_lv_changes,
																				  id_after, fn_ptr, std::move(change));
							break;
						}
						case IdentifierTag::SEN: {
							auto const entry = [&, node_before = node_before]() {
								if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
									return SingleEntry<1, htt_t>{{node_before.decode_key_part()}};
								} else {
									auto const sen_ptr = node_before.template specific_ptr<SingleEntryNode>();
									return SingleEntry_t<depth>{sen_ptr->key(), sen_ptr->value()};
								}
							}();

							change.entries.push_back(entry);

							auto const ptr = lv_changes.template add_node<NewNodeRcPolicy::NoInc>(std::move(change.entries), node_storage, {});
							upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, ptr);

							if constexpr (depth >= 2) {
								lv_changes.reassign_refcount(id_after, ptr.identifier());
							} else {
								assert(!id_after.is_indeterminate());
							}
							break;
						}
						case IdentifierTag::XN: {
							if constexpr (depth > 1) {
								auto xn_ptr = node_before.template specific_ptr<CartesianNode>();
								insert_into_cartesian_node(node_storage, all_lv_changes, upwards_lv_changes,
														   id_after, xn_ptr, std::move(change));
								break;
							} else {
								HYPERTRIE_UNREACHABLE;
							}
						}
						case IdentifierTag::Indeterminate: {
							// id before must always be fully-determined
							HYPERTRIE_UNREACHABLE;
						}
					}
				}
			}

			for (auto &&[id_after, fn_before] : lv_changes.fn_moves) {
				assert(id_after.is_fn() || id_after.is_indeterminate());

				auto fn_it = fns_.find(fn_before);
				assert(fn_it != fns_.end());

				// detach node
				fns_.erase(fn_it);

				auto &&change = lv_changes.node_changes.find(fn_before)->second.find(id_after)->second;
				insert_into_full_node<depth, NodeSource::RequestMove>(node_storage, all_lv_changes, upwards_lv_changes,
																	  id_after, fn_before, std::move(change));
			}

			super_t::template populate_placeholder_full_nodes<LvChanges_t, Operation::Insert>(node_storage, all_lv_changes, {});

			if constexpr (depth > 1) {
				super_t::template populate_placeholder_cartesian_nodes<LvChanges_t>(node_storage, all_lv_changes, {});
			}

			super_t::template apply_rc_deltas<LvChanges_t>(node_storage, all_lv_changes);

			if constexpr (depth > 1) {
				auto &direct_next_lv_changes = all_lv_changes.template get<depth - 1>();
				direct_next_lv_changes.template calculate_movables<Operation::Insert>(node_storage);

				auto &next_lv_changes = all_lv_changes.template subtuple<1UL, depth - 1>();
				apply_down<start_depth, depth - 1>(node_storage, next_lv_changes, upwards_lv_changes);
			} else if constexpr (start_depth >= 2) {
				apply_up<start_depth, 2>(node_storage, upwards_lv_changes);
			}
		}

		template<size_t start_depth, size_t depth>
		static void apply_up(NodeStorage_t &node_storage, UpwardsLvChanges_t const &upwards_lv_changes) {
			auto const &current_changes = upwards_lv_changes.template for_depth<depth>();

			for (auto &[id, edge_reassigns] : current_changes.edge_reassigns) {
				switch (id.tag()) {
					case IdentifierTag::FN: {
						auto fn_ptr = node_storage.template lookup<depth, FullNode>(id);
						for (auto const &[pos, new_child] : edge_reassigns) {
							auto [found, child_it] = fn_ptr->find(pos.pos, pos.key_part);
							assert(found);

							auto &child = child_it.value();
							child = new_child;
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

			if constexpr (depth < start_depth) {
				apply_up<start_depth, depth + 1>(node_storage, upwards_lv_changes);
			}
		}

		/**
		 * @brief Inserts `entries` into `full_node`
		 * 
		 * @tparam depth depth of the full_node where entries are inserted
		 * @tparam node_src indicates how `full_node` was created for more information see @ref enum NodeSource
		 * @param next_level_changes
		 * @param full_nodes_
		 * @param id_after
		 * @param full_node
		 * @param entries
		 */
		template<size_t depth, NodeSource node_src>
		static void insert_into_full_node(NodeStorage_t &node_storage,
										  template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes,
										  UpwardsLvChanges_t &upwards_lv_changes,
										  RawIdentifier_t<depth> id_after,
										  FNPtr_t<depth> original,
										  typename LvChanges_t<depth>::Change &&change) noexcept {
			static constexpr size_t child_depth = depth - 1;

			auto &fn_storage_ = node_storage.template nodes<depth, FullNode>();
			auto &fn_lifecycle_ = fn_storage_.node_lifecycle();
			auto &fns_ = fn_storage_.nodes();

			auto &lv_changes = all_lv_changes.template get<depth>();

			auto const get_fn_ptr = [&]() {
				if constexpr (node_src == NodeSource::RequestCopy) {
					return fn_lifecycle_.new_(*original);
				} else {
					return original;
				}
			};

			if constexpr (depth == 1) {
				// can only stay FN
				assert(id_after.is_fn());

				FNPtr_t<1> fn;

				if constexpr (node_src == NodeSource::RequestCopy) {
					if (auto it = fns_.find(id_after); it != fns_.end()) {
						upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, *it);
						return;
					}
				} else {
					// if node was moved fns_ should not contain a node with the same id
					// as that would have made the move useless
					assert(!fns_.contains(id_after));
				}

				if (auto it = lv_changes.new_fns.find(id_after); it != lv_changes.new_fns.end()) {
					// invalid insert planning detected
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

				fn->ref_count() = 0;
				fn->hash() = id_after.hash();

				auto &edges = fn->edges();
				for (auto const &entry : change.entries) {
					assert(!edges.contains(entry.key()[0]));

					if constexpr (HypertrieTrait_bool_valued<htt_t>) {
						edges.emplace(entry.key()[0]);
					} else {
						edges.emplace(entry.key()[0], entry.value());
					}
				}

				fns_.emplace(fn);
				return;
			} else /* depth > 1 */ {
				// can become either FN or XN
				assert(id_after.is_indeterminate() || id_after.is_fn());

				FNPtr_t<depth> fn = nullptr;

				{
					auto const fn_id = id_after.retag_as_fn();

					if constexpr (node_src == NodeSource::RequestCopy) {
						if (auto it = fns_.find(fn_id); it != fns_.end()) {
							if (id_after.is_indeterminate()) {
								id_after = lv_changes.reassign_refcount(id_after, fn_id);
							}

							upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, *it);
							return;
						}
					} else {
						// if node was moved fns_ should not contain a node with the same id
						// as that would have made the move useless
						assert(!fns_.contains(id_after));
					}

					if (auto it = lv_changes.new_fns.find(fn_id); it != lv_changes.new_fns.end()) {
						// invalid insert planning detected
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
					// id_after is either FN or Indeterminate (as per assertion above)
					// therefore only need to check for XN existence if id_after is indeterminate

					auto const xn_id = id_after.retag_as_xn();
					if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(xn_id); xn_ptr != nullptr) {
						lv_changes.reassign_refcount(id_after, xn_id);
						upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, xn_ptr);
						return;
					}
				}

				auto &direct_next_lv_changes = all_lv_changes.template get<child_depth>();
				auto all_newly_inserted_children = super_t::entry_subsets(change.entries);

				if (id_after.is_indeterminate()) {
					if constexpr (HypertrieTrait_bool_valued<htt_t>) {
						auto const future_edge_counts = calculate_future_edge_counts_after_insertion(*original, all_newly_inserted_children);

						if (is_general_cartesian<htt_t>(future_edge_counts, original->size() + change.entries.size())) {
							id_after = lv_changes.reassign_refcount(id_after, id_after.retag_as_xn());

							auto const existing_operands = extract_operands(*original);
							auto new_operands = calculate_newly_inserted_operand_members<depth, htt_t, allocator_type>(existing_operands, inverse_cartesian_product(change.entries));
							auto result_operands = merge_operands<depth, htt_t, allocator_type>(existing_operands, std::move(new_operands));

							auto xn_ptr = lv_changes.template add_general_cartesian<NewNodeRcPolicy::NoInc>(id_after, std::move(result_operands), node_storage);
							upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, xn_ptr);

							if constexpr (node_src == NodeSource::RequestMove) {
								super_t::template direct_delete_full_node<LvChanges_t, depth>(node_storage, direct_next_lv_changes, original);
							}

							return;
						}
					}

					// Why can this node not become an xfix cartesian?
					// 1. For this node to become an xfix cartesian means the combined old entries + new entries must have an xfix.
					// 2. This is a FN, therefore it does not have an xfix, as it wouldn't be a FN if it had one.
					// 		=> no single dimension has only 1 child.
					// 4. We are inserting, which means we do, under no circumstances, remove edges.
					// 		=> The future node can also not have an edge with only 1 child.
					// 		=> The future node can also not be an xfix.
					assert((!try_get_xfix_cartesian_properties(calculate_future_edge_counts_after_insertion(*original, all_newly_inserted_children)).has_value()));

					id_after = lv_changes.reassign_refcount(id_after, id_after.retag_as_fn());
				}

				assert(id_after.is_fn());

				if (fn == nullptr) {
					fn = get_fn_ptr();
				}

				upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, fn);

				fn->hash() = id_after.hash();
				fn->ref_count() = 0; // ok because will be adjusted from lv_changes.rc_deltas later

				assert(id_after.size() == fn->size() + change.entries.size());
				fn->size() += change.entries.size();

				for (size_t pos = 0; pos < depth; ++pos) {
					auto &edges = fn->edges(pos);
					auto &newly_inserted_children = all_newly_inserted_children[pos];

					// If a full_node is copied, some child mappings are altered some are added.
					// For child mappings that stay the same the childs ref_count must be increased.
					// Obviously, there is now a new full_node (this one) which references them.
					if constexpr (node_src == NodeSource::RequestCopy) {
						for (auto const &[key_part, child] : edges) {
							if constexpr (child_depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
								if (child.is_sen()) {
									continue; // ignore inplace children
								}
							}

							if (newly_inserted_children.contains(key_part)) {
								continue; // ignore children with changes
							}

							direct_next_lv_changes.inc_ref(child.identifier());
						}
					}

					for (auto &&[key_part, child_inserted_entries] : newly_inserted_children) {
						assert(child_inserted_entries.size() > 0);

						if (auto child_it = edges.find(key_part); child_it != edges.end()) {
							auto &child = container::deref(child_it);

							static constexpr auto rc_policy = node_src == NodeSource::RequestMove
																	  ? NodeBeforeRcPolicy::Dec
																	  : NodeBeforeRcPolicy::NoDec;

							Requester_t const this_requester{.who_asked = id_after,
															 .who_asked_edge = BoundPos<htt_t>{.pos = pos,
																							   .key_part = key_part}};

							child = direct_next_lv_changes.template insert_into_node<rc_policy>(child,
																								std::move(child_inserted_entries),
																								this_requester,
																								node_storage);
						} else {
							if constexpr (child_depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
								if (child_inserted_entries.size() == 1) {
									edges.emplace(key_part, NodePtr_t<1>::encode_key_part(child_inserted_entries[0].key()[0]));
									continue;
								}
							}

							edges.emplace(key_part, direct_next_lv_changes.add_node(std::move(child_inserted_entries), node_storage, {}));
						}
					}
				}

				fns_.insert(fn); // this insert will sometimes replace the fn ptr with itself in the node storage, i.e. be a noop
								 // I could handle this separately, but it doesn't make a difference if I do
			}
		}

		template<size_t depth>
		static void insert_into_cartesian_node(NodeStorage_t &node_storage,
											   template_library::integral_template_tuple<1UL, depth, LvChanges_t> &all_lv_changes,
											   UpwardsLvChanges_t &upwards_lv_changes,
											   RawIdentifier_t<depth> id_after,
											   XNPtr_t<depth> original,
											   typename LvChanges_t<depth>::Change &&change) noexcept requires (depth > 1) {
			assert(id_after.is_indeterminate());

			auto &lv_changes = all_lv_changes.template get<depth>();
			auto &next_lv_changes = all_lv_changes.template subtuple<1UL, depth - 1>();

			{
				auto const fn_id = id_after.retag_as_fn();
				if (auto fn_ptr = node_storage.template lookup<depth, FullNode>(fn_id); fn_ptr != nullptr) {
					lv_changes.reassign_refcount(id_after, fn_id);
					upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, fn_ptr);
					return;
				}
			}

			{
				auto const xn_id = id_after.retag_as_xn();
				if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(xn_id); xn_ptr != nullptr) {
					lv_changes.reassign_refcount(id_after, xn_id);
					upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, xn_ptr);
					return;
				}
			}

			auto &xn_storage_ = node_storage.template nodes<depth, CartesianNode>();
			auto &xn_lifecycle_ = xn_storage_.node_lifecycle();
			auto &xns_ = xn_storage_.nodes();

			auto const existing_operands = extract_operands(*original);
			auto newly_inserted_operands = calculate_newly_inserted_operand_members<depth, htt_t, allocator_type>(existing_operands, inverse_cartesian_product(change.entries));
			auto const future_operand_sizes = calculate_future_operand_sizes_after_insertion<depth, htt_t, allocator_type>(existing_operands, newly_inserted_operands);

			if constexpr (HypertrieTrait_bool_valued<htt_t>) {
				if (is_general_cartesian<htt_t>(future_operand_sizes, original->size() + change.entries.size())) {
					id_after = lv_changes.reassign_refcount(id_after, id_after.retag_as_xn());

					if (original->is_general_cartesian()) {
						// was general cartesian before as well
						// fast path
						auto copy = xn_lifecycle_.new_(*original);
						copy->hash() = id_after.hash();
						copy->ref_count() = 0;

						assert(id_after.size() == copy->size() + change.entries.size());
						copy->size() += change.entries.size();

						copy->for_each_operand([&]<size_t ix, size_t operand_depth>(NodePtr_t<operand_depth> &operand) noexcept {
							if constexpr (operand_depth == 1) {
								auto &&new_operand = newly_inserted_operands[ix];

								if (!new_operand.empty()) {
									// cartesian nodes will not have cartesian children
									// => child must become FN

									Requester_t this_requester{.who_asked = id_after,
															   .who_asked_edge = {.pos = ix, .key_part = {}}};

									operand = next_lv_changes.template get<1>().template insert_into_node<NodeBeforeRcPolicy::NoDec, IdentifierTag::FN>(operand,
																																						std::move(new_operand),
																																						this_requester,
																																						node_storage);
								} else {
									if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
										if (operand.is_sen()) {
											return;
										}
									}

									next_lv_changes.template get<1>().inc_ref(operand.identifier());
								}
							} else {
								HYPERTRIE_UNREACHABLE;
							}
						});

						upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, copy);
						xns_.insert(copy);
					} else {
						auto merged = merge_operands<depth, htt_t, allocator_type>(existing_operands, std::move(newly_inserted_operands));
						auto xn_ptr = lv_changes.template add_general_cartesian<NewNodeRcPolicy::NoInc>(id_after, std::move(merged), node_storage);
						upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, xn_ptr);
					}

					return;
				}
			}

			MaterializeEntries<depth, htt_t> auto const entries = [&]() noexcept -> std::vector<SingleEntry_t<depth>> && {
				for (RawIterator_t<depth> iter{original}; iter; ++iter) {
					change.entries.emplace_back(*iter);
				}

				return std::move(change.entries);
			};

			if (auto const xfix_props = try_get_xfix_cartesian_properties(future_operand_sizes); xfix_props.has_value()) {
				id_after = lv_changes.reassign_refcount(id_after, id_after.retag_as_xn());

				auto const original_operand_sizes = get_operand_sizes<depth, htt_t, allocator_type>(existing_operands);
				if (auto original_xfix_props = try_get_xfix_cartesian_properties(original_operand_sizes); original_xfix_props == xfix_props) {
					// stays same kind of xfix => fast path
					auto copy = xn_lifecycle_.new_(*original);
					copy->hash() = id_after.hash();
					copy->ref_count() = 0;

					assert(id_after.size() == copy->size() + change.entries.size());
					copy->size() += change.entries.size();

					xfix_props->template visit<depth>([&]<size_t prefix_len, size_t postfix_len>() noexcept {
						auto const high_order_operand = static_cast<NodePtr_t<depth - prefix_len - postfix_len>>(copy->operand(prefix_len));
						auto new_high_order_operand_entries = super_t::template trim_entries<prefix_len, postfix_len>(change.entries);

						copy->for_each_operand([&]<size_t ix, size_t operand_depth>(NodePtr_t<operand_depth> &child) noexcept {
							if constexpr (operand_depth > 0) {
								if constexpr (ix == prefix_len) {
									if constexpr (operand_depth == depth - prefix_len - postfix_len) {
										Requester_t this_requester{.who_asked = id_after,
																   .who_asked_edge = {.pos = ix, .key_part = {}}};

										child = next_lv_changes
														.template get<depth - prefix_len - postfix_len>()
														.template insert_into_node<NodeBeforeRcPolicy::NoDec, IdentifierTag::FN>(high_order_operand,
																																 std::move(new_high_order_operand_entries),
																																 this_requester,
																																 node_storage);
									} else {
										HYPERTRIE_UNREACHABLE;
									}
								} else if constexpr (operand_depth == 1) {
									if constexpr (!HypertrieTrait_taggable_key_part<htt_t>) {
										next_lv_changes.template get<operand_depth>().inc_ref(child.identifier());
									}
								} else {
									HYPERTRIE_UNREACHABLE;
								}
							}
						});
					});

					upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, copy);
					xns_.insert(copy);
				} else {
					auto merged = merge_operands<depth, htt_t, allocator_type>(existing_operands, std::move(newly_inserted_operands));

					auto placeholder = lv_changes.template calc_and_add_xfix_cartesian<NewNodeRcPolicy::NoInc>(id_after,
																											   *xfix_props,
																											   merged,
																											   entries,
																											   node_storage);
					upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, placeholder);
				}

				return;
			}

			id_after = lv_changes.reassign_refcount(id_after, id_after.retag_as_fn());

			auto fn_ptr = lv_changes.template add_full_node<NewNodeRcPolicy::NoInc>(id_after, entries, node_storage);
			upwards_lv_changes.template answer_edge_reassigns<depth>(change.requesters, fn_ptr);
		}
	};
}// namespace dice::hypertrie::internal::raw::node_context::insert_detail
#endif//HYPERTRIE_INSERT_IMPL_RAWNODECONTEXT_HPP
