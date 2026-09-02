#ifndef HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_LVCHANGES_HPP
#define HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_LVCHANGES_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/NodePtr.hpp"

#include "dice/hypertrie/internal/raw/iteration/RawIterator.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/CartesianUtil.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/CommonUpwardsLvChanges.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/Container.hpp"
#include "dice/hypertrie/internal/raw/node_context/slice_detail/SliceImpl.hpp"
#include "dice/template-library/integral_template_variant.hpp"

#include <algorithm>

namespace dice::hypertrie::internal::raw::node_context::common_detail {

	/**
	 * Whether or not decrement the reference count
	 * of the node usually referred to by id_before
	 */
	enum struct NodeBeforeRcPolicy : bool {
		Dec,
		NoDec,
	};

	/**
	 * Whether or not to increment the reference
	 * count of the newly created node
	 */
	enum struct NewNodeRcPolicy : bool {
		Inc,
		NoInc,
	};

	enum struct Operation : bool {
		Insert,
		Remove,
	};

	template<typename F, size_t depth, typename htt_t>
	concept MaterializeEntries = HypertrieTrait<htt_t> && std::is_nothrow_invocable_r_v<std::vector<SingleEntry<depth, htt_t>>, F>;

	template<size_t operand_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct PtrOperand {
		size_t size_hint;
		NodePtr<operand_depth, htt_t, allocator_type> ptr;
	};

	template<size_t depth, size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct CommonLvChanges {
		using key_part_type = typename htt_t::key_part_type;
		using SingleEntry_t = SingleEntry<depth, htt_t>;
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;
		using NodePtr_t = NodePtr<depth, htt_t, allocator_type>;
		using FNPtr_t = FNPtr<depth, htt_t, allocator_type>;
		using XNPtr_t = XNPtr<depth, htt_t, allocator_type>;
        using SENPtr_t = SENPtr<depth, htt_t, allocator_type>;

		using NodeStorage_t = NodeStorage<max_depth, htt_t, allocator_type>;

		template<size_t operand_depth>
		using CartesianOperand_t = CartesianOperand<operand_depth, htt_t>;

		template<size_t operand_depth>
		using VariantCartesianOperand_t = std::variant<PtrOperand<operand_depth, htt_t, allocator_type>, CartesianOperand<operand_depth, htt_t>>;

		using VariantCartesianOperands = std::array<template_library::integral_template_variant<0UL, depth - 1, VariantCartesianOperand_t>, depth>;

		struct Change {
			std::vector<SingleEntry_t> entries;
			std::vector<Requester<max_depth, htt_t>> requesters;
		};

		struct NewFN {
			FNPtr_t placeholder;
			std::vector<SingleEntry_t> entries;
		};

		struct NewXN {
			XNPtr_t placeholder;
			VariantCartesianOperands operands;
		};

		/**
		 * New FNs that need to be created
		 * placeholder -> entries to insert
		 *
		 * TODO: need to add code to clean up these notes for transactions v1
		 */
		Map<RawIdentifier_t, NewFN> new_fns;

		/**
		 * New XNs that need to be created
		 * placeholder -> operands (= op1 x op2 x ... x opn) to assign
		 *
		 * TODO: need to add code to clean up these notes for transactions v1
		 */
		Map<RawIdentifier_t, NewXN> new_xns;

		/**
		 * Insertions into nodes (could be insertions into any of the 3 node kinds)
		 * id before insertion -> (id after insertions -> (entries to be inserted, ids of nodes requesting the insert))
		 *
		 * TODO: potential perf, split off fn_changes to make calculate_movables faster
		 */
		Map<NodePtr_t, Map<RawIdentifier_t, Change>> node_changes;

		/**
		 * Full nodes that can be created by moving the node instead of creating a copy
		 * this map references entries in node_changes.
		 *
		 * id_after -> id_before
		 */
		Map<RawIdentifier_t, FNPtr_t> fn_moves;

		/**
		 * Reference count deltas for all nodes (/ node kinds)
		 * id -> delta
		 *
		 * TODO: potential perf, create additional NodePtr -> ssize_t rc_deltas map to avoid .identifier() overhead on inc/dec ref calls
		 */
		Map<RawIdentifier_t, ssize_t> rc_deltas;

		/**
		 * Increment a node's refcount
		 */
		inline void inc_ref(RawIdentifier_t const id, size_t const n = 1) noexcept {
			assert(id.retag_as_indeterminate() != RawIdentifier_t{});
			if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				assert(!id.is_sen());
			}

			this->rc_deltas[id] += n;
		}

		/**
		 * Decrement a node's refcount
		 */
		inline void dec_ref(RawIdentifier_t const id, size_t const n = 1) noexcept {
			assert(id.retag_as_indeterminate() != RawIdentifier_t{});
			if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				assert(!id.is_sen());
			}

			this->rc_deltas[id] -= n;
		}

		/**
		 * Retrieve the refcount delta for a node
		 */
		inline ssize_t get_rc_delta(RawIdentifier_t const id) const noexcept {
			auto const it = this->rc_deltas.find(id);
			if (it == this->rc_deltas.end()) {
				return 0;
			}

			return it->second;
		}

		/**
		 * Reassign the refcount delta of owner_before to owner_after (by addition)
		 */
		RawIdentifier_t reassign_refcount(RawIdentifier_t const owner_before, RawIdentifier_t const owner_after) {
			assert(owner_before.is_indeterminate());
			assert(!owner_after.is_indeterminate());

			if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				assert(!owner_after.is_sen());
			}

			if (auto it = this->rc_deltas.find(owner_before); it != this->rc_deltas.end()) {
				auto const old_rc = it->second;
				this->rc_deltas.erase(it);
				this->inc_ref(owner_after, old_rc);
			}
			return owner_after;
		}

		template<NewNodeRcPolicy rc_policy = NewNodeRcPolicy::Inc>
		XNPtr_t add_general_cartesian(RawIdentifier_t const id,
									  std::array<CartesianOperand<1, htt_t>, depth> &&operands,
									  NodeStorage_t &node_storage) noexcept requires (depth > 1 && HypertrieTrait_bool_valued<htt_t>) {
			assert(id.is_xn());
			assert(id != RawIdentifier_t{}.retag_as_xn());

			if constexpr (rc_policy == NewNodeRcPolicy::Inc) {
				this->inc_ref(id);
			}

			if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(id); xn_ptr != nullptr) {
				return xn_ptr;
			}

			assert(!this->new_xns.contains(id));

			VariantCartesianOperands variant_operands; {
				for (size_t ix = 0; ix < depth; ++ix) {
					variant_operands[ix] = VariantCartesianOperand_t<1>{std::move(operands[ix])};
				}
			}

			auto placeholder = node_storage.create_placeholder_cartesian(id);
			this->new_xns.emplace(id, NewXN{.placeholder = placeholder,
											.operands = std::move(variant_operands)});
			return placeholder;
		}

		template<size_t prefix_len, size_t postfix_len, typename Operand>
		XNPtr_t _add_xfix_cartesian(RawIdentifier_t const id,
									std::array<CartesianOperand_t<1>, prefix_len> &&prefix,
									Operand &&high_order_operand,
									std::array<CartesianOperand_t<1>, postfix_len> &&postfix,
									NodeStorage_t &node_storage) noexcept requires (prefix_len + postfix_len < depth && (prefix_len > 0 || postfix_len > 0)) {
			assert(id.is_xn());
			assert(id != RawIdentifier_t{}.retag_as_xn());
			assert((std::ranges::all_of(prefix, [](auto const &operand) { return operand.size() == 1; })));
			assert((std::ranges::all_of(postfix, [](auto const &operand) { return operand.size() == 1; })));

			VariantCartesianOperands variant_operands; {
				if constexpr (prefix_len > 0) {
					for (size_t ix = 0; ix < prefix_len; ++ix) {
						variant_operands[ix] = VariantCartesianOperand_t<1>{std::move(prefix[ix])};
					}
				}

				variant_operands[prefix_len] = VariantCartesianOperand_t<depth - prefix_len - postfix_len>{std::forward<Operand>(high_order_operand)};

				if constexpr (postfix_len > 0) {
					for (size_t ix = 0; ix < postfix_len; ++ix) {
						variant_operands[prefix_len + 1 + ix] = VariantCartesianOperand_t<1>{std::move(postfix[ix])};
					}
				}
			}

			auto placeholder = node_storage.create_placeholder_cartesian(id);
			this->new_xns.emplace(id, NewXN{.placeholder = placeholder,
											.operands = std::move(variant_operands)});
			return placeholder;
		}

		template<NewNodeRcPolicy rc_policy = NewNodeRcPolicy::Inc>
		XNPtr_t _calc_and_add_xfix_cartesian(RawIdentifier_t const id,
											 XFixCartesianProperties const &props,
											 std::array<CartesianOperand_t<1>, depth> const &operands,
											 std::vector<SingleEntry_t> const &entries,
											 NodeStorage_t &node_storage) noexcept requires (depth > 1)  {
			assert(id.is_xn());
			assert(id != RawIdentifier_t{}.retag_as_xn());

			return props.template visit<depth>([&]<size_t prefix_len, size_t postfix_len>() noexcept {
				std::array<CartesianOperand_t<1>, prefix_len> prefix;
				if constexpr (prefix_len > 0) {
					std::copy_n(operands.begin(), prefix_len, prefix.begin());
				}

				std::vector<SingleEntry<depth - prefix_len - postfix_len, htt_t>> high_order_operand; {
					for (auto const &e : entries) {
						auto &added = high_order_operand.emplace_back();
						std::copy(e.key().begin() + prefix_len, e.key().end() - postfix_len, added.key().begin());
						if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
							added.value_mut() = e.value();
						}
					}

					// TODO: can I just assert that?
					std::sort(high_order_operand.begin(), high_order_operand.end());
					auto uniq_end = std::unique(high_order_operand.begin(), high_order_operand.end());
					high_order_operand.erase(uniq_end, high_order_operand.end());
				}

				std::array<CartesianOperand_t<1>, postfix_len> postfix;
				if constexpr (postfix_len > 0) {
					std::copy_n(operands.end() - postfix_len, postfix_len, postfix.begin());
				}

				return _add_xfix_cartesian(id, std::move(prefix), std::move(high_order_operand), std::move(postfix), node_storage);
			});
		}

		template<NewNodeRcPolicy rc_policy = NewNodeRcPolicy::Inc>
		XNPtr_t calc_and_add_xfix_cartesian(RawIdentifier_t const id,
											XFixCartesianProperties const &props,
											std::array<CartesianOperand_t<1>, depth> const &operands,
											std::vector<SingleEntry_t> const &entries,
											NodeStorage_t &node_storage) noexcept requires (depth > 1) {
			assert(id.is_xn());
			assert(id != RawIdentifier_t{}.retag_as_xn());
			assert(props.prefix_len > 0 || props.postfix_len > 0);
			assert(props.prefix_len + props.postfix_len < depth);

			if constexpr (rc_policy == NewNodeRcPolicy::Inc) {
				this->inc_ref(id);
			}

			if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(id); xn_ptr != nullptr) {
				return xn_ptr;
			}

			assert(!this->new_xns.contains(id));
			return _calc_and_add_xfix_cartesian(id, props, operands, entries, node_storage);
		}

		/**
		 * Overload for calc_and_add_xfix cartesian that lazily materializes
		 * the entries only when necessary
		 */
		template<NewNodeRcPolicy rc_policy = NewNodeRcPolicy::Inc>
		XNPtr_t calc_and_add_xfix_cartesian(RawIdentifier_t const id,
											XFixCartesianProperties const &props,
											std::array<CartesianOperand_t<1>, depth> const &operands,
											MaterializeEntries<depth, htt_t> auto &&mentries,
											NodeStorage_t &node_storage) noexcept requires (depth > 1) {
			assert(id.is_xn());
			assert(id != RawIdentifier_t{}.retag_as_xn());
			assert(props.prefix_len > 0 || props.postfix_len > 0);
			assert(props.prefix_len + props.postfix_len < depth);

			if constexpr (rc_policy == NewNodeRcPolicy::Inc) {
				this->inc_ref(id);
			}

			if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(id); xn_ptr != nullptr) {
				return xn_ptr;
			}

			assert(!this->new_xns.contains(id));
			return _calc_and_add_xfix_cartesian(id, props, operands, mentries(), node_storage);
		}

		template<NewNodeRcPolicy rc_policy = NewNodeRcPolicy::Inc>
		FNPtr_t add_full_node(RawIdentifier_t const id,
							  std::vector<SingleEntry_t> &&entries,
							  NodeStorage_t &node_storage) noexcept {
			assert(id.is_fn());
			assert(id != RawIdentifier_t{}.retag_as_fn());

			if constexpr (rc_policy == NewNodeRcPolicy::Inc) {
				this->inc_ref(id);
			}

			if (auto fn_ptr = node_storage.template lookup<depth, FullNode>(id); fn_ptr != nullptr) {
				return fn_ptr;
			}

			if (auto it = this->new_fns.find(id); it != this->new_fns.end()) {
				return it->second.placeholder;
			}

			assert(this->new_fns.find(id) == this->new_fns.end());
			auto placeholder = node_storage.template nodes<depth, FullNode>().node_lifecycle().new_with_alloc(id, 0UL);
			this->new_fns.emplace(id, NewFN{.placeholder = placeholder,
											.entries = std::move(entries)});
			return placeholder;
		}

		/**
		 * Overload for add_full_node that lazily materializes
		 * the entries only when necessary
		 */
		template<NewNodeRcPolicy rc_policy = NewNodeRcPolicy::Inc>
		FNPtr_t add_full_node(RawIdentifier_t const id,
							  MaterializeEntries<depth, htt_t> auto &&mentries,
							  NodeStorage_t &node_storage) noexcept {
			assert(id.is_fn());
			assert(id != RawIdentifier_t{}.retag_as_fn());

			if constexpr (rc_policy == NewNodeRcPolicy::Inc) {
				this->inc_ref(id);
			}

			if (auto fn_ptr = node_storage.template lookup<depth, FullNode>(id); fn_ptr != nullptr) {
				return fn_ptr;
			}

			if (auto it = this->new_fns.find(id); it != this->new_fns.end()) {
				return it->second.placeholder;
			}

			assert(!this->new_fns.contains(id));
			auto placeholder = node_storage.template nodes<depth, FullNode>().node_lifecycle().new_with_alloc(id, 0UL);
			this->new_fns.emplace(id, NewFN{.placeholder = placeholder,
											.entries = mentries()});
			return placeholder;
		}

		template<NewNodeRcPolicy rc_policy = NewNodeRcPolicy::Inc>
		std::optional<NodePtr_t> try_add_cartesian(RawIdentifier_t const id,
												   std::vector<SingleEntry_t> const &entries,
												   NodeStorage_t &node_storage) noexcept requires (depth > 1) {
			assert(id.is_xn());
			assert(id != RawIdentifier_t{}.retag_as_xn());

			if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(id); xn_ptr != nullptr) {
				if constexpr (rc_policy == NewNodeRcPolicy::Inc) {
					this->inc_ref(id);
				}

				return xn_ptr;
			}

			assert(!this->new_xns.contains(id));

			auto operands = inverse_cartesian_product(entries);

			if constexpr (HypertrieTrait_bool_valued<htt_t>) {
				if (is_general_cartesian(operands, entries.size())) {
					return this->template add_general_cartesian<rc_policy>(id, std::move(operands), node_storage);
				}
			}

			if (auto const xfix_props = try_get_xfix_cartesian_properties(operands); xfix_props.has_value()) {
				return this->template calc_and_add_xfix_cartesian<rc_policy>(id, *xfix_props, operands, entries, node_storage);
			}

			return std::nullopt;
		}

		template<NewNodeRcPolicy rc_policy = NewNodeRcPolicy::Inc, typename F = decltype([]<NewNodeRcPolicy>(RawIdentifier_t) noexcept -> SENPtr_t { return nullptr; })>
		NodePtr_t add_node(std::vector<SingleEntry_t> &&entries, NodeStorage_t &node_storage, F &&sen_notify_func) noexcept {
			assert(entries.size() > 0);

			if (entries.size() == 1) {
				if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					RawIdentifier_t const id{entries[0]};
					assert(id.is_sen());

					if (auto sen_ptr = std::forward<F>(sen_notify_func).template operator()<rc_policy>(id); sen_ptr != nullptr) {
						return sen_ptr;
					}

					if constexpr (rc_policy == NewNodeRcPolicy::Inc) {
						this->inc_ref(id);
					}

					if (auto sen_ptr = node_storage.template lookup<depth, SingleEntryNode>(id); sen_ptr != nullptr) {
						return sen_ptr;
					}

					return node_storage.create_sen(entries[0], 0);
				} else {
					HYPERTRIE_UNREACHABLE;
				}
			}

			RawIdentifier_t const id_after{entries};

			if constexpr (depth > 1) {
				if (auto const id = try_add_cartesian<rc_policy>(id_after.retag_as_xn(), entries, node_storage); id.has_value()) {
					// node was able to be represented by cartesian
					return *id;
				}
			}

			return this->template add_full_node<rc_policy>(id_after.retag_as_fn(), std::move(entries), node_storage);
		}

		template<Operation op>
		void calculate_movables(NodeStorage_t &node_storage) noexcept {
			auto const &fns_ = node_storage.template nodes<depth, FullNode>().nodes();

			Map<RawIdentifier_t, std::vector<FNPtr_t>> candidates;

			for (auto &[node_before, changes] : this->node_changes) {
				assert(!node_before.is_indeterminate());

				if (!node_before.is_fn()) {
					// only interested in moving FNs
					continue;
				}

				auto fn = node_before.template specific_ptr<FullNode>();
				auto const id_before = fn->identifier();

				if (static_cast<ssize_t>(fn->ref_count()) + this->get_rc_delta(id_before) + this->get_rc_delta(id_before.retag_as_indeterminate()) > 0) {
					// can only move nodes if the original will not be
					// referenced after applying all changes
					continue;
				}

				for (auto const &[id_after, _] : changes) {
					if constexpr (depth > 1) {
						auto const &xns_ = node_storage.template nodes<depth, CartesianNode>().nodes();
						if (xns_.contains(id_after.retag_as_xn())) {
							continue;
						}
					}

					if (auto it = fns_.find(id_after.retag_as_fn()); it != fns_.end()) {
						if constexpr (op == Operation::Insert) {
							if ((*it)->size() != 0) {
								continue;
							}
							// is placeholder, we can still consider this one to be the move target
						} else /* op == Operation::Remove */ {
							continue;
						}
					}

					if (auto it = candidates.find(id_after); it != candidates.end()) {
						it->second.emplace_back(fn);
					} else {
						candidates.emplace(id_after, std::vector<FNPtr_t>{fn});
					}
				}
			}

			Set<FNPtr_t> used_id_befores;

			for (auto const &[id_after, node_befores] : candidates) {
				if (id_after.is_indeterminate() && candidates.contains(id_after.retag_as_fn())) {
					continue;
				}

				auto it = std::find_if(node_befores.begin(), node_befores.end(), [&](auto const &node_before) noexcept {
					return !used_id_befores.contains(node_before);
				});

				if (it == node_befores.end()) {
					continue;
				}

				used_id_befores.emplace(*it);
				this->fn_moves.emplace(id_after, *it);
			}
		}
	};

	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct CommonLvChanges<0, max_depth, htt_t, allocator_type> {
	};

} // namespace dice::hypertrie::internal::raw::node_context::common_detail

#endif//HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_LVCHANGES_HPP
