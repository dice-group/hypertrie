#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include "dice/hypertrie/internal/container/deref_map_iterator.hpp"
#include "dice/hypertrie/internal/raw/node/NodeStorage.hpp"
#include "dice/hypertrie/internal/raw/node_context/SliceResult.hpp"
#include "dice/hypertrie/internal/raw/node_context/insert_detail/InsertImpl.hpp"
#include "dice/hypertrie/internal/raw/node_context/remove_detail/RemoveImpl.hpp"
#include "dice/hypertrie/internal/raw/node_context/slice_detail/SliceImpl.hpp"
#include "dice/hypertrie/internal/util/PermutationSort.hpp"
#include "dice/hypertrie/internal/raw/RawKey.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct RawHypertrieContext {
		using tri = htt_t;
		using value_type = typename htt_t::value_type;
		using key_part_type = typename htt_t::key_part_type;

		template<size_t depth, size_t fixed_depth>
		using SliceResultStorage_t = SliceResultStorage<depth - fixed_depth, htt_t, allocator_type>;

		NodeStorage<max_depth, htt_t, allocator_type> node_storage_;
		size_t proxy_generation_ = 0;

		explicit RawHypertrieContext(allocator_type const &alloc) noexcept : node_storage_(alloc) {}

		template<size_t depth>
		void inc_ref_count(NodePtr<depth, htt_t, allocator_type> node) noexcept {
			assert(node != nullptr);
			if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				assert(!node.is_sen());
			}

			auto &ref_count = node.visit_ptr([](auto node_ptr) -> auto & {
				return node_ptr->ref_count();
			});
			ref_count += 1;
		}

		template<size_t depth>
		void decr_ref_count(NodePtr<depth, htt_t, allocator_type> node) noexcept {
			assert(node != nullptr);
			if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				assert(!node.is_sen());
			}

			auto &ref_count = node.visit_ptr([](auto node_ptr) -> auto & {
				return node_ptr->ref_count();
			});
			assert(ref_count >= 1);

			if (ref_count > 1) {
				ref_count -= 1;
			} else {
				using I_t = node_context::insert_detail::InsertImpl<max_depth, htt_t, allocator_type>;

				template_library::integral_template_tuple<1UL, depth, I_t::template LvChanges_t> changes{};
				auto &first_changes = changes.template get<depth>();
				typename I_t::UpwardsLvChanges_t upwards_lv_changes;

				first_changes.dec_ref(node.identifier(), 1);
				node_context::insert_detail::InsertImpl<max_depth, htt_t, allocator_type>::template apply_down<depth>(node_storage_, changes, upwards_lv_changes);
			}
		}

		template<size_t depth>
		[[nodiscard]] size_t size(FNPtr<depth, htt_t, allocator_type> fn) const noexcept {
			if (fn == nullptr) {
				return 0;
			}

			return fn->size();
		}

		template<size_t depth>
			requires (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
		[[nodiscard]] size_t size(SENPtr<depth, htt_t, allocator_type> sen) const noexcept {
			return static_cast<size_t>(sen != nullptr);
		}

		template<size_t depth>
			requires (depth > 1)
		[[nodiscard]] size_t size(XNPtr<depth, htt_t, allocator_type> xn) const noexcept {
			if (xn == nullptr) {
				return 0;
			}

			return xn->size();
		}

		template<size_t depth>
		[[nodiscard]] size_t size(NodePtr<depth, htt_t, allocator_type> node) const noexcept {
			if (node == nullptr) {
				return 0;
			}

			switch (node.tag()) {
				case IdentifierTag::FN: {
					return node.template specific_ptr<FullNode>()->size();
				}
				case IdentifierTag::SEN: {
					return 1;
				}
				case IdentifierTag::XN: {
					return node.template specific_ptr<CartesianNode>()->size();
				}
				case IdentifierTag::Indeterminate: {
					HYPERTRIE_UNREACHABLE;
				}
			}
		}

		template<size_t depth>
		[[nodiscard]] std::vector<size_t> get_cards(NodePtr<depth, htt_t, allocator_type> node,
													std::vector<pos_type> const &positions) const noexcept {
			assert(positions.size() <= depth);

			if (node == nullptr) {
				return std::vector<size_t>(positions.size(), 0);
			}

			switch (node.tag()) {
				case IdentifierTag::FN: {
					return node.template specific_ptr<FullNode>()->get_cards(positions);
				}
				case IdentifierTag::SEN: {
					return std::vector<size_t>(positions.size(), 1);
				}
				case IdentifierTag::XN: {
					if constexpr (depth > 1) {
						return node.template specific_ptr<CartesianNode>()->get_cards(positions);
					} else {
						HYPERTRIE_UNREACHABLE;
					}
				}
				case IdentifierTag::Indeterminate: {
					HYPERTRIE_UNREACHABLE;
				}
			}
		}

		template<size_t depth>
		[[nodiscard]] static std::vector<size_t> get_cards([[maybe_unused]] SENPtr<depth, htt_t, allocator_type> sen,
														   std::vector<pos_type> const &positions) noexcept {
			assert(positions.size() <= depth);
			if (sen == nullptr) {
				return std::vector<size_t>(positions.size(), 0);
			}

			return std::vector<size_t>(positions.size(), 1);
		}

		template<size_t depth>
			requires (depth > 1)
		[[nodiscard]] static std::vector<size_t> get_cards(XNPtr<depth, htt_t, allocator_type> xn,
														   std::vector<pos_type> const &positions) noexcept {
			assert(positions.size() <= depth);
			if (xn == nullptr) {
				return std::vector<size_t>(positions.size(), 0);
			}

			return xn->get_cards(positions);
		}

		template<size_t depth>
		[[nodiscard]] static std::vector<size_t> get_cards(FNPtr<depth, htt_t, allocator_type> fn,
														   std::vector<pos_type> const &positions) noexcept {
			assert(positions.size() <= depth);
			if (fn == nullptr) {
				return std::vector<size_t>(positions.size(), 0);
			}

			return fn->get_cards(positions);
		}

		/**
		 * Checks two nodes for equality, this function has a special case for non-context-borrowed cartesians
		 * which do not have a valid identifier
		 *
		 * @param a operand 1
		 * @param a_own ownership of operand 1
		 * @param b operand 2
		 * @param b_own ownership of operand 2
		 * @return if a and b refer to equal nodes
		 */
		template<size_t depth>
		[[nodiscard]] static bool equal(NodePtr<depth, htt_t, allocator_type> const &a, Ownership a_own,
										NodePtr<depth, htt_t, allocator_type> const &b, Ownership b_own) noexcept {
			if (a == b) {
				assert(a_own == b_own);
				return true;
			}

			if (a.tag() != b.tag()) {
				return false;
			}

			switch (a.tag()) {
				case IdentifierTag::FN: {
					return a.template specific_ptr<FullNode>()->identifier() == b.template specific_ptr<FullNode>()->identifier();
				}
				case IdentifierTag::SEN: {
					if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						// will be handled by a == b above.
						HYPERTRIE_UNREACHABLE;
					} else {
						return static_cast<SingleEntry<depth, htt_t> const &>(*a.template specific_ptr<SingleEntryNode>())
							   == static_cast<SingleEntry<depth, htt_t> const &>(*b.template specific_ptr<SingleEntryNode>());
					}
				}
				case IdentifierTag::XN: {
					if constexpr (depth > 1) {
						XNPtr<depth, htt_t, allocator_type> const a_xn = a.template specific_ptr<CartesianNode>();
						XNPtr<depth, htt_t, allocator_type> const b_xn = b.template specific_ptr<CartesianNode>();

						if (a_own == Ownership::ContextBorrowed && b_own == Ownership::ContextBorrowed) {
							return a_xn->identifier() == b_xn->identifier();
						}

						if (a_xn->n_operands() != b_xn->n_operands() || a_xn->discriminant() != b_xn->discriminant()) {
							return false;
						}

						bool eq = true;
						a_xn->for_each_operand([&b_xn, &eq]<size_t ix, size_t operand_depth>(NodePtr<operand_depth, htt_t, allocator_type> const &a_op) noexcept {
							if constexpr (operand_depth > 0) {
								auto const &b_op = static_cast<NodePtr<operand_depth, htt_t, allocator_type> const &>(b_xn->operand(ix));

								// explicitly specifying ContextBorrowed because children of Owned or EphemeralBorrowed XNs
								// are guaranteed to be ContextBorrowed (slicing can only produce a single new node)
								eq = eq && equal(a_op, Ownership::ContextBorrowed, b_op, Ownership::ContextBorrowed);
							}
						});

						return eq;
					} else {
						HYPERTRIE_UNREACHABLE;
					}
				}
				case IdentifierTag::Indeterminate: {
					HYPERTRIE_UNREACHABLE;
				}
			}
		}

		/**
		 * Calculates a hash for a node. This function has a special case for non-context-borrowed cartesians
		 * which do not have a valid identifier.
		 *
		 * @param node node to hash
		 * @param ownership ownership of node
		 * @return hash of node
		 */
		template<size_t depth>
		[[nodiscard]] static size_t hash(NodePtr<depth, htt_t, allocator_type> const &node, Ownership ownership) noexcept {
			switch (node.tag()) {
				case IdentifierTag::FN: {
					return node.template specific_ptr<FullNode>()->hash();
				}
				case IdentifierTag::SEN: {
					if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						return RawIdentifier<1, htt_t>::hash_single_entry(SingleEntry<1, htt_t>{{node.decode_key_part()}, true});
					} else {
						return node.template specific_ptr<SingleEntryNode>()->hash();
					}
				}
				case IdentifierTag::XN: {
					if constexpr (depth > 1) {
						XNPtr<depth, htt_t, allocator_type> const xn_ptr = node.template specific_ptr<CartesianNode>();

						if (ownership == Ownership::ContextBorrowed) {
							return xn_ptr->hash();
						}

						return ::dice::hash::dice_hash_templates<::dice::hash::Policies::wyhash>::dice_hash(std::tie(xn_ptr->discriminant(), xn_ptr->operands()));
					} else {
						HYPERTRIE_UNREACHABLE;
					}
				}
				case IdentifierTag::Indeterminate: {
					HYPERTRIE_UNREACHABLE;
				}
			}
		}

		template<size_t depth>
		value_type set(NodePtr<depth, htt_t, allocator_type> &node, RawKey<depth, htt_t> const &key, value_type const &new_value) noexcept {
			value_type const old_value = get(node, key);

			if (new_value == old_value) {
				return new_value;
			}

			if (new_value == value_type{}) {
				remove(node, {SingleEntry<depth, htt_t>{key, old_value}});
				return old_value;
			}

			if (old_value == value_type{}) {
				insert(node, {SingleEntry<depth, htt_t>{key, new_value}});
			} else {
				assert(false);
				// TODO: implement
				// change_values(nodec, {entry});
			}

			return old_value;
		}

		template<size_t depth>
		void change_values([[maybe_unused]] NodePtr<depth, htt_t, allocator_type> &node,
						   [[maybe_unused]] std::vector<SingleEntry<depth, htt_t>> const &entries) noexcept {
			assert(false);
			proxy_generation_ += 1;
			// TODO: implement
		}

		/**
		 * Entries must not yet be contained in nodec
		 * @tparam depth depth of the hypertrie
		 * @param nodec nodec
		 * @param entries
		 */
		template<size_t depth>
		void insert(NodePtr<depth, htt_t, allocator_type> &node,
					std::vector<SingleEntry<depth, htt_t>> entries) noexcept {
			node_context::insert_detail::InsertImpl<max_depth, htt_t, allocator_type>::exec(node_storage_, node, std::move(entries));
			proxy_generation_ += 1;
		}

		template<size_t depth>
		void remove(NodePtr<depth, htt_t, allocator_type> &node,
					std::vector<SingleEntry<depth, htt_t>> entries) noexcept {
			node_context::remove_detail::RemoveImpl<max_depth, htt_t, allocator_type>::exec(node_storage_, node, std::move(entries));
			proxy_generation_ += 1;
		}

		template <size_t depth, size_t fixed_depth>
		using specific_slice_result = std::conditional_t<(depth > fixed_depth), SliceResult<depth - fixed_depth, htt_t, allocator_type>, value_type>;

		template<size_t depth>
		static value_type get(NodePtr<depth, htt_t, allocator_type> node,
							  RawKey<depth, htt_t> const &key) noexcept {
			return node_context::slice_detail::get(node, key);
		}

		template<size_t depth, size_t fixed_keyparts>
		static specific_slice_result<depth, fixed_keyparts> slice(NodePtr<depth, htt_t, allocator_type> node,
																  RawSliceKey<fixed_keyparts, htt_t> const &slice_key,
																  SliceResultStorage_t<depth, fixed_keyparts> *result_storage = nullptr) noexcept {
			if (node == nullptr) {
				return {};
			}

			return node_context::slice_detail::slice(node, slice_key, result_storage);
		}

		template<size_t depth, size_t fixed_depth>
		static specific_slice_result<depth, fixed_depth> diagonal_slice(NodePtr<depth, htt_t, allocator_type> node,
																		RawKeyPositions<depth> const &diagonal_positions,
																		key_part_type fixed_key_part,
																		SliceResultStorage_t<depth, fixed_depth> *result_storage = nullptr) noexcept {
			if (node == nullptr) {
				return {};
			}

			return slice(node, diagonal_positions.template to_slice_key<fixed_depth, htt_t>(fixed_key_part), result_storage);
		}
	};

	template<size_t depth, size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct RawIndexProxy {
		using key_type = RawKey<depth, htt_t>;
		using value_type = typename htt_t::value_type;
		using node_context_type = RawHypertrieContext<max_depth, htt_t, allocator_type>;
		using node_ptr_type = NodePtr<depth, htt_t, allocator_type>;

		friend struct RawHypertrieContext<max_depth, htt_t, allocator_type>;

	private:
		node_context_type *context_ = nullptr;
		node_ptr_type *node_ptr_;
		key_type key_;
		std::optional<value_type> mutable cached_value_;
		size_t mutable generation_;
	public:
		RawIndexProxy(node_context_type *context, node_ptr_type *node_ptr, key_type key) noexcept : context_{context},
																									node_ptr_{node_ptr},
																									key_{key} {
		}

		RawIndexProxy(RawIndexProxy const &other) noexcept = default;
		RawIndexProxy(RawIndexProxy &&other) noexcept = default;
		RawIndexProxy &operator=(RawIndexProxy const &other) noexcept = default;
		RawIndexProxy &operator=(RawIndexProxy &&other) noexcept = default;
		~RawIndexProxy() noexcept = default;

		void set(value_type new_value) noexcept {
			cached_value_ = new_value;
			context_->set(*node_ptr_, key_, new_value);
		}

		value_type get() const noexcept {
			if (cached_value_.has_value() && generation_ == context_->proxy_generation_) {
				return *cached_value_;
			}

			cached_value_ = context_->get(*node_ptr_, key_);
			generation_ = context_->proxy_generation_;
			return *cached_value_;
		}

		RawIndexProxy &operator=(value_type const new_value) noexcept {
			this->set(new_value);
			return *this;
		}

		operator value_type() const noexcept {
			return this->get();
		}
	};

}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_NODECONTEXT_HPP
