#ifndef HYPERTRIE_NODECONTEXT_HPP
#define HYPERTRIE_NODECONTEXT_HPP

#include <Dice/hypertrie/internal/raw/node/NodeStorage.hpp>
#include <Dice/hypertrie/internal/raw/node_context/insert_impl_RawNodeContext.hpp>

#include <Dice/hypertrie/internal/raw/RawKey.hpp>

namespace hypertrie::internal::raw {

	// TODO: move into own file
	template<size_t result_depth, HypertrieCoreTrait tri_t>
	struct SliceResult {
		using tri = tri_t;

		using VariantType = std::variant<NodeContainer<result_depth, tri>, NodeContainer<result_depth, tri_with_stl_alloc<tri>>>;

		VariantType variant_type_;

	private:
		template<class... Args>
		auto make_managed(Args &&...args) {
			SliceResult slice_result;
			slice_result.emplace<0>(std::forward<Args...>(args...));
		}

		template<class... Args>
		auto make_unmanaged(Args &&...args) {
			SliceResult slice_result;
			slice_result.emplace<0>(std::forward<Args...>(args...));
		}

		NodeContainer<result_depth, tri> get_managed() const {
			std::get<0>(variant_type_);
		}

		NodeContainer<result_depth, tri_with_stl_alloc<tri>> get_unmanaged() const {
			std::get<1>(variant_type_);
		}
	};

	template<size_t max_depth, HypertrieCoreTrait_bool_valued tri_t>
	struct RawHypertrieContext {
		using tri = tri_t;
		using value_type = typename tri::value_type;
		using key_part_type = typename tri::key_part_type;

		NodeStorage<max_depth, tri_t> node_storage_;

		explicit RawHypertrieContext(const typename tri::allocator_type &alloc) : node_storage_(alloc) {}

		/**
		 * Entries must not yet be contained in nodec
		 * @tparam depth depth of the hypertrie
		 * @param nodec nodec
		 * @param entries
		 */
		template<size_t depth>
		void insert(NodeContainer<depth, tri> &nodec,
					const std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> &entries) {
			insert_impl_RawNodeContext<max_depth, tri>::exec(node_storage_, nodec, entries);
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
		inline auto resolve(FNContainer<depth, tri> &nodec, pos_type pos, key_part_type key_part)
				-> std::conditional_t<(depth > 1), NodeContainer<depth - 1, tri>, value_type> {
			assert(pos < depth);
			if (nodec.empty())
				return {};
			else {
				auto value_or_child_id = nodec.node_ptr()->child(pos, key_part);
				if constexpr (depth == 2 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) {
					if (value_or_child_id.is_sen())
						return SENContainer<depth - 1, tri>{value_or_child_id, {}};
					else
						return {value_or_child_id, node_storage_.template lookup<depth - 1, FullNode>(value_or_child_id)};
				} else if constexpr (depth > 1) {
					if (not value_or_child_id.empty())
						return node_storage_.template lookup(value_or_child_id);
					else
						return {};
				} else {// depth == 1
					return value_or_child_id;
				}
			}
		}

		/**
		 * Retrieves the value for a key.
		 * @tparam depth the depth of the node container
		 * @param nodec the node container
		 * @param key the key
		 * @return the value associated to the key.
		 */
		template<size_t depth>
		auto get(const NodeContainer<depth, tri> &nodec, RawKey<depth, tri> key) -> value_type {
			if (nodec.empty())
				return {};
			else if (nodec.is_sen()) {
				SENContainer<depth, tri> sen_nodec = nodec.template specific<SingleEntryNode>();
				if constexpr (depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) {
					return nodec.identifier().get_entry().key()[0] == key[0];
				} else {
					if (sen_nodec.node_ptr()->key() == key) {
						if constexpr (tri::is_bool_valued) return true;
						else
							return sen_nodec.node_ptr()->value();
					} else
						return {};
				}
			} else {
				FNContainer<depth, tri> fn_nodec = nodec.template specific<FullNode>();
				if constexpr (depth > 1) {
					// TODO: better max_card_pos?
					auto pos = fn_nodec.node_ptr()->min_card_pos();
					NodeContainer<depth - 1, tri> child = this->template resolve<depth>(fn_nodec, pos, key[pos]);

					return get<depth - 1>(child, key.template subkey(pos));
				} else {
					return this->template resolve<1>(fn_nodec, 0UL, key[0]);
				}
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
		auto slice(const NodeContainer<depth, tri> &nodec, RawSliceKey<fixed_keyparts, tri> raw_slice_key)
				-> std::conditional_t<(depth > fixed_keyparts), SliceResult<depth - fixed_keyparts, tri>, value_type> {
			using Res = SliceResult<depth - fixed_keyparts, tri>;
			if (nodec.empty())
				return Res::make_managed();
			return slice_rek(nodec, raw_slice_key);
		}

		template<size_t current_depth, size_t fixed_keyparts>
		auto slice_rek(const NodeContainer<current_depth, tri> &nodec, const RawSliceKey<fixed_keyparts, tri> &raw_slice_key)
				-> std::conditional_t<(current_depth > fixed_keyparts), SliceResult<current_depth - fixed_keyparts, tri>, value_type> {

			constexpr static const size_t result_depth = current_depth - fixed_keyparts;

			using Res = SliceResult<result_depth, tri>;
			if (nodec.is_sen()) {
				FNContainer<current_depth, tri> sen_nodec = nodec.template specific<SingleEntryNode>();

				auto slice_opt = raw_slice_key.slice(sen_nodec.node_ptr()->key());
				if (slice_opt.has_value()) {
					auto slice = slice_opt.value();
					if constexpr (result_depth > 0) {
						SingleEntry<result_depth, tri_with_stl_alloc<tri>> entry{slice, sen_nodec.node_ptr()->value()};
						Identifier<result_depth, tri_with_stl_alloc<tri>> identifier{entry};
						if constexpr (result_depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>)
							return Res::make_managed(identifier, nullptr);
						else
							return Res::make_unmanaged(identifier, new decltype(entry)(entry));
					} else {
						return sen_nodec.node_ptr()->value();
					}
				} else {
					return Res::make_managed();
				}
			} else {
				FNContainer<current_depth, tri> fn_nodec = nodec.template specific<FullNode>();
				if constexpr (fixed_keyparts == 1)
					return Res::make_managed(this->template resolve(fn_nodec, raw_slice_key[0].pos, raw_slice_key[0].key_part));
				else {

					const size_t slice_key_i = fn_nodec.node_ptr()->min_fixed_keypart_i(raw_slice_key);
					auto child = this->template resolve(fn_nodec, raw_slice_key[slice_key_i].pos, raw_slice_key[slice_key_i].key_part);
					if (child.empty())
						return Res::make_managed();
					else
						return slice_rek<current_depth - 1, fixed_keyparts - 1>(child, raw_slice_key.subkey_i(slice_key_i));
				}
			}
		}
	};
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_NODECONTEXT_HPP