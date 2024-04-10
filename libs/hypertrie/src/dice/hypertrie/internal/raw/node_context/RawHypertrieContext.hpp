#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include "dice/hypertrie/internal/container/deref_map_iterator.hpp"
#include "dice/hypertrie/internal/raw/node/NodeStorage.hpp"
#include "dice/hypertrie/internal/raw/node_context/SliceResult.hpp"
#include "dice/hypertrie/internal/raw/node_context/insert_impl_RawNodeContext.hpp"

#include "dice/hypertrie/internal/raw/RawKey.hpp"

namespace dice::hypertrie::internal::raw {


	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct RawHypertrieContext {
		using tri = htt_t;
		using value_type = typename htt_t::value_type;
		using key_part_type = typename htt_t::key_part_type;

		NodeStorage<max_depth, htt_t, allocator_type> node_storage_;

		explicit RawHypertrieContext(allocator_type const &alloc) noexcept : node_storage_(alloc) {}

		template<size_t depth>
		void inc_ref_count(NodeContainer<depth, htt_t, allocator_type> const &nodec) noexcept {
			assert(not nodec.is_null_ptr() and not nodec.empty());
			if (nodec.is_sen()) {
				assert(not(HypertrieTrait_bool_valued_and_taggable_key_part<htt_t> and depth == 1));
				++nodec.template specific<SingleEntryNode>().node_ptr()->ref_count();
			} else {
				++nodec.template specific<SingleEntryNode>().node_ptr()->ref_count();
			}
		}

		template<size_t depth>
		void decr_ref_count(NodeContainer<depth, htt_t, allocator_type> const &nodec) noexcept {
			assert(not nodec.is_null_ptr() and not nodec.empty());
			size_t &ref_count = [&]() -> size_t & {
				if (nodec.is_sen()) {
					auto ptr = nodec.template specific<SingleEntryNode>().node_ptr();
					return ptr->ref_count();
				}
				return nodec.template specific<FullNode>().node_ptr()->ref_count();
			}();

			assert(ref_count >= 1);

			if (ref_count > 1) {
				--ref_count;
			} else {
				ContextLevelChanges<depth, htt_t, allocator_type> changes;
				changes.inc_ref(nodec.raw_identifier(), -1);
				insert_impl_RawNodeContext<max_depth, htt_t, allocator_type>::template apply<depth>(node_storage_, changes);
			}
		}

		template<size_t depth>
		[[nodiscard]] size_t size(FNContainer<depth, htt_t, allocator_type> const &fn_container) const noexcept {
			if (not fn_container.empty()) {
				assert(not fn_container.is_null_ptr());
				return fn_container.node_ptr()->size();
			}
			return 0;
		}

		template<size_t depth>
		[[nodiscard]] size_t size(SENContainer<depth, htt_t, allocator_type> const &sen_container) const noexcept {
			return sen_container.empty() ? 0 : 1;
		}

		template<size_t depth>
		[[nodiscard]] size_t size(NodeContainer<depth, htt_t, allocator_type> const &nodec) const noexcept {
			if (nodec.is_sen()) {
				return this->template size<depth>(nodec.template specific<SingleEntryNode>());
			}
			return this->template size<depth>(nodec.template specific<FullNode>());
		}

		template<size_t depth>
		auto set(NodeContainer<depth, htt_t, allocator_type> &nodec, SingleEntry<depth, htt_t> const &entry) noexcept {
			if constexpr (HypertrieTrait_bool_valued_and_taggable_key_part<htt_t> and depth == 1) {
				if (nodec.empty()) {
					insert(nodec, {entry});
				}
			}

			if (entry.value() == value_type{}) {
				assert(false);// TODO: implement deleting entries
			}

			value_type const old_value = get(nodec, entry.key());
			if (entry.value() == old_value) {
				return entry.value();
			}
			if (old_value == value_type{}) {
				insert(nodec, {entry});
			}
			/*
			else {
				change_values(nodec, {entry});
			}
			*/
			return old_value;
		}

		template<size_t depth>
		void change_values([[maybe_unused]] NodeContainer<depth, htt_t, allocator_type> &nodec,
						   [[maybe_unused]] std::vector<SingleEntry<depth, htt_t>> const &entries) noexcept {
			assert(false);
			// TODO: implement
		}


		/**
		 * Entries must not yet be contained in nodec
		 * @tparam depth depth of the hypertrie
		 * @param nodec nodec
		 * @param entries
		 */
		template<size_t depth>
		void insert(NodeContainer<depth, htt_t, allocator_type> &nodec,
					std::vector<SingleEntry<depth, htt_t>> const &entries) noexcept {
			insert_impl_RawNodeContext<max_depth, htt_t, allocator_type>::exec(node_storage_, nodec, entries);
		}

		/**
		 * Resolves a keypart by a given position
		 * @tparam depth the depth of the node container
		 * @param nodec the node container, must be a uncompressed node
		 * @param pos the position at which the key_part should be resolved
		 * @param key_part the key part
		 * @return an NodeContainer of depth - 1. It might be empty if there is no child for the given pos and key part. <br/>
		 * If depth is 1, a value_type is return
		 */
		template<size_t depth>
		inline auto resolve(FNContainer<depth, htt_t, allocator_type> const &nodec, pos_type pos, key_part_type key_part) noexcept
				-> std::conditional_t<(depth > 1), NodeContainer<depth - 1, htt_t, allocator_type>, value_type> {
			assert(pos < depth);
			if (nodec.empty()) {
				return {};
			}
			auto value_or_child_id = nodec.node_ptr()->child(pos, key_part);
			if constexpr (depth == 2 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				if (value_or_child_id.is_sen()) {
					return SENContainer<depth - 1, htt_t, allocator_type>{value_or_child_id};
				}
				return FNContainer<depth - 1, htt_t, allocator_type>{value_or_child_id, node_storage_.template lookup<depth - 1, FullNode>(value_or_child_id)};
			} else if constexpr (depth > 1) {
				if (not value_or_child_id.empty()) {
					return node_storage_.template lookup(value_or_child_id);
				}
				return {};
			} else {// depth == 1
				return value_or_child_id;
			}
		}

		template<size_t depth, ByteAllocator allocator_type2>
		static value_type get(SENContainer<depth, htt_t, allocator_type2> const &nodec, RawKey<depth, htt_t> key) noexcept {
			auto key_tri = RawKey<depth, htt_t>(key);
			if (nodec.empty()) {
				return {};
			}
			if constexpr (depth == 1 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				return nodec.raw_identifier().get_entry().key()[0] == key_tri[0];
			} else {
				if (nodec.node_ptr()->key() == key_tri) {
					if constexpr (htt_t::is_bool_valued) {
						return true;
					} else {
						return nodec.node_ptr()->value();
					}
				}
				return {};
			}
		}

		template<size_t depth>
		value_type get(FNContainer<depth, htt_t, allocator_type> const &nodec, RawKey<depth, htt_t> key) noexcept {
			if (nodec.empty()) {
				return {};
			}
			if constexpr (depth > 1) {
				// TODO: we could benchmark if it makes a difference whether we use here random, max_card_pos or min_card_pos?
				auto pos = nodec.node_ptr()->min_card_pos();
				NodeContainer<depth - 1, htt_t, allocator_type> child = this->template resolve<depth>(nodec, pos, key[pos]);
				return get<depth - 1>(child, key.subkey(pos));
			} else {
				return this->template resolve<1>(nodec, 0UL, key[0]);
			}
		}

		//tri2 = tri is needed, because otherwise the key cannot be inserted via initializer list.
		/**
		 * Retrieves the value for a key.
		 * @tparam depth the depth of the node container
		 * @param nodec the node container
		 * @param key the key
		 * @return the value associated to the key.
		 */
		template<size_t depth>
		value_type get(NodeContainer<depth, htt_t, allocator_type> const &nodec, RawKey<depth, htt_t> key) noexcept {
			if (nodec.is_sen()) {
				SENContainer<depth, htt_t, allocator_type> sen_nodec = nodec.template specific<SingleEntryNode>();
				return get(sen_nodec, key);
			}
			FNContainer<depth, htt_t, allocator_type> fn_nodec = nodec.template specific<FullNode>();
			return get(fn_nodec, key);
		}


		template <size_t DEPTH, size_t FIXED_KEYPARTS, ByteAllocator alloc2>
		using specific_slice_result = std::conditional_t<(DEPTH > FIXED_KEYPARTS), SliceResult<DEPTH - FIXED_KEYPARTS, htt_t, alloc2>, value_type>;

		template<size_t depth, size_t fixed_keyparts, ByteAllocator alloc2>
		static auto slice(SENContainer<depth, htt_t, alloc2> const &nodec, RawSliceKey<fixed_keyparts, htt_t> raw_slice_key) noexcept
				-> specific_slice_result<depth, fixed_keyparts, alloc2> {
			static constexpr size_t result_depth = depth - fixed_keyparts;

			using SliceResult_t = SliceResult<depth - fixed_keyparts, htt_t, alloc2>;

			if constexpr (fixed_keyparts == 0) {
				return SliceResult_t::make_sen_with_stl_alloc(false, nodec.raw_identifier(), new SingleEntryNode<depth, htt_t>{*nodec.node_ptr()});
			} else if constexpr (depth == fixed_keyparts) {
				RawKey<depth, htt_t> raw_key;
				for (size_t i = 0; i < depth; ++i) {
					raw_key[i] = raw_slice_key[i].key_part;
				}
				return get(nodec, raw_key);
			} else {
				if (nodec.empty()) {
					return {};
				}
				auto slice_opt = raw_slice_key.slice(nodec.node_ptr()->key());
				if (slice_opt.has_value()) {
					auto slice = slice_opt.value();
					SingleEntry<result_depth, htt_t> entry{slice, nodec.node_ptr()->value()};
					RawIdentifier<result_depth, htt_t> identifier{entry};
					if constexpr (result_depth == 1 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
						return SliceResult_t::make_with_provided_alloc(identifier);
					else {
						return SliceResult_t::make_sen_with_stl_alloc(false, identifier, new SingleEntryNode<result_depth, htt_t>(entry));
					}
				}
				return SliceResult_t{};
			}
		}

		/**
		 * Returns a pair of a node container and a boolean which states if the pointed node is managed (true) or if it is unmanaged (false) and MUST be deleted by the user manually.
		 * Only compressed nodes can be managed.
		 * if fixed_depth == depth, just a scalar is returned
		 * @tparam depth depth of the node container
		 * @tparam fixed_keyparts number of fixed key_parts in the slice key
		 * @param nodec a container with a node.
		 * @param raw_slice_key the slice key
		 * @return see above
		 */
		template<size_t depth, size_t fixed_keyparts>
		auto slice(const NodeContainer<depth, htt_t, allocator_type> &nodec, RawSliceKey<fixed_keyparts, htt_t> raw_slice_key) noexcept
				-> specific_slice_result<depth, fixed_keyparts, allocator_type> {
			using SliceResult_t = SliceResult<depth - fixed_keyparts, htt_t, allocator_type>;
			if constexpr (fixed_keyparts == 0) {
				return SliceResult_t::make_with_provided_alloc(nodec);
			} else if constexpr (depth == fixed_keyparts) {
				RawKey<depth, htt_t> raw_key;
				for (size_t i = 0; i < depth; ++i) {
					raw_key[i] = raw_slice_key[i].key_part;
				}
				return get(nodec, raw_key);
			} else {
				return nodec.empty() ? SliceResult_t{} : slice_rek(nodec, raw_slice_key);
			}
		}

		template<size_t current_depth, size_t fixed_keyparts>
		auto slice_rek(const NodeContainer<current_depth, htt_t, allocator_type> &nodec, RawSliceKey<fixed_keyparts, htt_t> const &raw_slice_key) noexcept
				-> specific_slice_result<current_depth, fixed_keyparts, allocator_type> {

			static constexpr size_t result_depth = current_depth - fixed_keyparts;

			using SliceResult_t = SliceResult<result_depth, htt_t, allocator_type>;
			if (nodec.is_sen()) {
				SENContainer<current_depth, htt_t, allocator_type> sen_nodec = nodec.template specific<SingleEntryNode>();
				return slice(sen_nodec, raw_slice_key);
			}
			FNContainer<current_depth, htt_t, allocator_type> fn_nodec = nodec.template specific<FullNode>();
			size_t const slice_key_i = fn_nodec.node_ptr()->min_fixed_keypart_i(raw_slice_key);
			auto const &fixed_value = raw_slice_key[slice_key_i];
			auto child = this->template resolve(fn_nodec, (pos_type) fixed_value.pos, fixed_value.key_part);
			if (child.empty()) {
				return SliceResult_t{};
			}
			if constexpr (fixed_keyparts == 1) {
				return SliceResult_t::make_with_provided_alloc(child);
			} else {
				return slice_rek<current_depth - 1, fixed_keyparts - 1>(child, raw_slice_key.subkey_i(slice_key_i));
			}
		}

		template<size_t depth, size_t fixed_keyparts>
		auto diagonal_slice(NodeContainer<depth, htt_t, allocator_type> const &nodec, RawKeyPositions<depth> const &diagonal_positions, key_part_type fixed_key_part,
							SingleEntryNode<depth - fixed_keyparts, htt_t> *sen_result_cache = nullptr) noexcept
				-> specific_slice_result<depth, fixed_keyparts, allocator_type> {
			// TODO: implement for SENContainer<depth, tri_with_stl_alloc<htt_t>>
			using SliceResult_t = SliceResult<depth - fixed_keyparts, htt_t, allocator_type>;
			if constexpr (fixed_keyparts == 0) {
				return SliceResult<depth, htt_t, allocator_type>::make_with_provided_alloc(nodec);
			} else if constexpr (depth == fixed_keyparts) {
				RawKey<depth, htt_t> raw_key;
				for (size_t i = 0; i < depth; ++i) {
					raw_key[i] = fixed_key_part;
				}
				return get(nodec, raw_key);
			} else {
				if (nodec.empty()) {
					return SliceResult_t{};
				}
				return diagonal_slice_rek<depth, fixed_keyparts>(nodec, diagonal_positions, fixed_key_part, sen_result_cache);
			}
		}

		template<size_t current_depth, size_t fixed_keyparts>
		auto diagonal_slice_rek(NodeContainer<current_depth, htt_t, allocator_type> const &nodec, RawKeyPositions<current_depth> const &diagonal_positions, key_part_type fixed_key_part,
								SingleEntryNode<current_depth - fixed_keyparts, htt_t> *sen_result_cache = nullptr) noexcept
				-> specific_slice_result<current_depth, fixed_keyparts, allocator_type> {

			static constexpr size_t result_depth = current_depth - fixed_keyparts;

			using SliceResult_t = SliceResult<result_depth, htt_t, allocator_type>;
			if (nodec.is_sen()) {
				SENContainer<current_depth, htt_t, allocator_type> sen_nodec = nodec.template specific<SingleEntryNode>();
				auto slice_opt = diagonal_positions.template slice<fixed_keyparts>(sen_nodec.node_ptr()->key(), fixed_key_part);
				if (slice_opt.has_value()) {
					auto slice = slice_opt.value();
					SingleEntry<result_depth, htt_t> entry{slice, sen_nodec.node_ptr()->value()};
					RawIdentifier<result_depth, htt_t> identifier{entry};
					if constexpr (result_depth == 1 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						return SliceResult_t::make_sen_with_stl_alloc(true, identifier, nullptr);
					} else {
						if (sen_result_cache != nullptr) {
							*sen_result_cache = SingleEntryNode<current_depth - fixed_keyparts, htt_t>{entry};
							return SliceResult_t::make_sen_with_stl_alloc(true, identifier, sen_result_cache);
						}
						return SliceResult_t::make_sen_with_stl_alloc(false, identifier, new SingleEntryNode<result_depth, htt_t>(entry));
					}
				}
				return SliceResult_t{};
			}
			FNContainer<current_depth, htt_t, allocator_type> fn_nodec = nodec.template specific<FullNode>();
			const size_t pos = fn_nodec.node_ptr()->min_card_pos(diagonal_positions);
			auto child = this->template resolve(fn_nodec, (pos_type) pos, fixed_key_part);
			if (child.empty()) {
				return SliceResult_t{};
			}
			if constexpr (fixed_keyparts == 1) {
				return SliceResult_t::make_with_provided_alloc(child);
			} else {
				return diagonal_slice_rek<current_depth - 1, fixed_keyparts - 1>(child, diagonal_positions.sub_raw_key_positions(pos), fixed_key_part, sen_result_cache);
			}
		}
	};


}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_NODECONTEXT_HPP
