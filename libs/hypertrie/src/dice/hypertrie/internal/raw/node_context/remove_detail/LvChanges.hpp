#ifndef HYPERTRIE_RAWNODECONTEXT_REMOVE_IMPL_LVCHANGES_HPP
#define HYPERTRIE_RAWNODECONTEXT_REMOVE_IMPL_LVCHANGES_HPP

#include <algorithm>
#include <cassert>
#include <vector>

#include "dice/hypertrie/internal/raw/node/NodeStorage.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/CommonLvChanges.hpp"
#include "dice/hypertrie/internal/raw/node_context/remove_detail/UpwardsLvChanges.hpp"
#include "dice/template-library/integral_template_variant.hpp"

namespace dice::hypertrie::internal::raw::node_context::remove_detail {
	using namespace node_context::common_detail;

	enum struct SENCheckValuePolicy : bool {
		WriteValue,
		IgnoreValue,
	};

	/**
	 * A request to check which single entry a node will represent after removing
	 */
	template<size_t max_depth, HypertrieTrait htt_t>
	struct SENCheckOrigin {
		template<size_t other_depth>
		using RawIdentifier_rebind_depth_t = RawIdentifier<other_depth, htt_t>;

		template_library::integral_template_variant<2UL, max_depth + 1, RawIdentifier_rebind_depth_t> who_asked;
		BoundPos<htt_t> who_asked_child;

		typename htt_t::key_part_type *path = {};
		size_t write_ix = 0;

		typename htt_t::value_type *value = nullptr;
	};

	template<size_t depth, size_t max_depth, HypertrieTrait htt_t>
	struct SENChecks {
		std::vector<SingleEntry<depth, htt_t>> to_remove;
		std::vector<SENCheckOrigin<max_depth, htt_t>> sen_checks;
	};

	template<size_t depth, size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct LvChanges : CommonLvChanges<depth, max_depth, htt_t, allocator_type> {
		using super_t = CommonLvChanges<depth, max_depth, htt_t, allocator_type>;

		using typename super_t::key_part_type;
		using typename super_t::SingleEntry_t;
		using typename super_t::RawIdentifier_t;
		using typename super_t::Change;
		using typename super_t::NodeStorage_t;
		using typename super_t::NodePtr_t;

		using SENCheck_t = SENCheckOrigin<max_depth, htt_t>;
		using DeduplicatedSENChecks_t = SENChecks<depth, max_depth, htt_t>;
		using Requester_t = Requester<max_depth, htt_t>;

		/**
		 * id_before -> (sen_id_after -> [(who asked, path, ...)])
		 *
		 * A request to id_before to figure out which single entry would be left after removing.
		 * Note: sen_id_after only exists for deduplication purposes and does not have a semantic meaning.
		 */
		Map<NodePtr_t, Map<RawIdentifier_t, DeduplicatedSENChecks_t>> sen_checks;

		/**
		 * Ask a child to remove entries
		 *
		 * @param id_before the child's id before removing any entries
		 * @param entries the entries the child is supposed to remove
		 * @param origin who is asking
		 * @return the precalculated id the child will probably have after removing
		 */
		template<NodeBeforeRcPolicy rc_policy = NodeBeforeRcPolicy::Dec, IdentifierTag tag_hint = IdentifierTag::Indeterminate>
		NodePtr_t remove_from_node(NodePtr_t const node_before,
								   std::vector<SingleEntry_t> &&entries,
								   Requester_t const &requester,
								   NodeStorage_t &node_storage) noexcept {
			assert(!entries.empty());
			assert(requester.depth() > depth);

			auto id_before = node_before.identifier();
			assert(!id_before.is_indeterminate());
			assert(id_before.retag_as_indeterminate() != RawIdentifier_t{});

			if constexpr (rc_policy == NodeBeforeRcPolicy::Dec) {
				if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					if (!node_before.is_sen()) {
						this->dec_ref(id_before);
					}
				} else {
					this->dec_ref(id_before);
				}
			}

			auto &changes = this->node_changes[node_before];

			if constexpr (depth == 1) {
				// can only become FN
				static_assert(tag_hint == IdentifierTag::Indeterminate || tag_hint == IdentifierTag::FN);

				auto const id_after = id_before.combine_remove(RawIdentifier_t{entries}, IdentifierTag::FN);
				this->inc_ref(id_after);

				if (auto it = changes.find(id_after); it != changes.end()) {
					it->second.requesters.push_back(requester);
				} else {
					changes.emplace(id_after, Change{.entries = std::move(entries),
													 .requesters = {requester}});
				}

				return NodePtr_t{};
			} else {
				// can either become FN or XN
				auto const id_after = id_before.combine_remove(RawIdentifier_t{entries}, tag_hint);

				if constexpr (tag_hint == IdentifierTag::Indeterminate || tag_hint == IdentifierTag::FN) {
					auto const fn_id_after = id_after.retag_as_fn();

					if (auto fn_ptr = node_storage.template lookup<depth, FullNode>(fn_id_after); fn_ptr != nullptr) {
						this->inc_ref(fn_id_after);
						return fn_ptr;
					}

					if (auto it = changes.find(fn_id_after); it != changes.end()) {
						this->inc_ref(fn_id_after);
						container::deref(it).requesters.push_back(requester);
						return NodePtr_t{}; // will be replaced later
					}
				}

				if constexpr (tag_hint == IdentifierTag::Indeterminate || tag_hint == IdentifierTag::XN) {
					auto const xn_id_after = id_after.retag_as_xn();

					if (auto xn_ptr = node_storage.template lookup<depth, CartesianNode>(xn_id_after); xn_ptr != nullptr) {
						this->inc_ref(xn_id_after);
						return xn_ptr;
					}

					if (auto it = changes.find(xn_id_after); it != changes.end()) {
						this->inc_ref(xn_id_after);
						container::deref(it).requesters.push_back(requester);
						return NodePtr_t{}; // will be replaced later
					}
				}

				this->inc_ref(id_after);

				// still indeterminate, this means we need to message requester later
				// since we cannot calculate the correct id yet

				if (auto it = changes.find(id_after); it != changes.end()) {
					// exists but is currently indeterminate
					it->second.requesters.push_back(requester);
				} else {
					changes.emplace(id_after, Change{.entries = std::move(entries),
													 .requesters = {requester}});
				}

				return NodePtr_t{}; // will be replaced later
			}
		}

		/**
		 * Ask a child to perform a SEN check
		 *
		 * @param child child to ask
		 * @param cs sen check parameters
		 */
		template<NodeBeforeRcPolicy rc_policy = NodeBeforeRcPolicy::Dec>
		NodePtr_t start_sen_check(NodePtr_t const child,
								  RawIdentifier<depth + 1, htt_t> const who_asked,
								  BoundPos<htt_t> const who_asked_edge,
								  std::vector<SingleEntry_t> &&to_remove,
								  NodeStorage_t &node_storage,
								  AllUpwardsLvChanges<max_depth, htt_t, allocator_type> &upwards_lv_changes) noexcept {

			auto child_id_before = child.identifier();
			assert(!child_id_before.is_indeterminate());
			assert(child_id_before.retag_as_indeterminate() != RawIdentifier_t{});

			if constexpr (rc_policy == NodeBeforeRcPolicy::Dec) {
				this->dec_ref(child_id_before);
			}

			auto const id_after = child_id_before.combine_remove(RawIdentifier_t{to_remove}, IdentifierTag::SEN);

			if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				if (auto sen_ptr = node_storage.template lookup<depth, SingleEntryNode>(id_after); sen_ptr != nullptr) {
					this->inc_ref(id_after);
					return sen_ptr;
				}

				upwards_lv_changes.inc_sen_ref(id_after);
			}

			auto [cs, placeholder] = [&]() noexcept -> std::pair<std::optional<SENCheck_t>, NodePtr_t> {
				if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					return std::make_pair(SENCheck_t{.who_asked = who_asked,
													 .who_asked_child = who_asked_edge,
													 .path = nullptr,
													 .write_ix = 0,
													 .value = nullptr},
										  nullptr);
				} else {
					auto const [ph, is_new] = upwards_lv_changes.create_placeholder_sen(id_after, node_storage);
					if (is_new) {
						return std::make_pair(SENCheck_t{.who_asked = who_asked,
														 .who_asked_child = who_asked_edge,
														 .path = ph->key().data(),
														 .write_ix = 0,
														 .value = [ph = ph]() noexcept {
															 if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
																 return &ph->value_mut();
															 } else {
																 (void) ph;
																 return nullptr;
															 }
														 }()},
											  ph);
					}

					return std::make_pair(std::nullopt, ph);
				}
			}();

			if (cs.has_value()) {
				auto &sen_checks_for_child = this->sen_checks[child];
				if (auto it = sen_checks_for_child.find(id_after); it != sen_checks_for_child.end()) {
					container::deref(it).sen_checks.push_back(std::move(*cs));
				} else {
					sen_checks_for_child.emplace(id_after,
												 DeduplicatedSENChecks_t{.to_remove = std::move(to_remove),
																		 .sen_checks = {std::move(*cs)}});
				}
			}

			return placeholder;
		}

		/**
		 * forwards a received sen check to one child
		 *
		 * @param child child to forward to
		 * @param cs received sen check
		 * @param child_pos position/edge of the child in parent
		 */
		void forward_sen_check(NodePtr_t const child,
							   SENCheck_t const &cs,
							   typename htt_t::key_part_type const child_edge_key_part,
							   std::vector<SingleEntry_t> const &entry_subset) noexcept {
			auto id_before = child.identifier();
			assert(!id_before.is_indeterminate());
			assert(id_before.retag_as_indeterminate() != RawIdentifier_t{});

			auto const id_after = id_before.combine_remove(RawIdentifier_t{entry_subset}, IdentifierTag::SEN);
			cs.path[cs.write_ix] = child_edge_key_part;

			SENCheck_t new_cs{.who_asked       = cs.who_asked,
							  .who_asked_child = cs.who_asked_child,
							  .path            = cs.path,
							  .write_ix        = cs.write_ix + 1,
							  .value = cs.value};

			auto &sen_checks_for_child = this->sen_checks[child];
			if (sen_checks_for_child.contains(id_after)) {
				sen_checks_for_child[id_after].sen_checks.push_back(std::move(new_cs));
			} else {
				DeduplicatedSENChecks_t acs{.to_remove  = entry_subset,
											.sen_checks = {std::move(new_cs)}};

				sen_checks_for_child.emplace(id_after, std::move(acs));
			}
		}

		template<SENCheckValuePolicy val_policy>
		void forward_sen_check_to_cartesian_operand(NodePtr_t const operand, SENCheck_t const &cs,
													size_t operand_offset,
													std::vector<SingleEntry_t> const &entry_subset) noexcept {
			auto id_before = operand.identifier();
			assert(!id_before.is_indeterminate());
			assert(id_before.retag_as_indeterminate() != RawIdentifier_t{});

			auto const id_after = id_before.combine_remove(RawIdentifier_t{entry_subset}, IdentifierTag::SEN);

			SENCheck_t new_cs{.who_asked       = cs.who_asked,
							  .who_asked_child = cs.who_asked_child,
							  .path            = cs.path,
							  .write_ix        = cs.write_ix + operand_offset,
							  .value           = [&]() noexcept {
								  if constexpr (val_policy == SENCheckValuePolicy::WriteValue) {
									  return cs.value;
								  } else {
									  return nullptr;
								  }
							  }()};

			auto &sen_checks_for_child = this->sen_checks[operand];
			if (sen_checks_for_child.contains(id_after)) {
				sen_checks_for_child[id_after].sen_checks.push_back(std::move(new_cs));
			} else {
				DeduplicatedSENChecks_t acs{.to_remove  = entry_subset,
											.sen_checks = {std::move(new_cs)}};

				sen_checks_for_child.emplace(id_after, std::move(acs));
			}
		}
	};

} // namespace dice::hypertrie::internal::raw::node_context::remove_detail

#endif // HYPERTRIE_RAWNODECONTEXT_REMOVE_IMPL_LVCHANGES_HPP
