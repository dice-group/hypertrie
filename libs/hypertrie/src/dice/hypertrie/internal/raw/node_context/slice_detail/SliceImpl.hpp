#ifndef HYPERTRIE_RAWNODECONTEXT_SLICEDETAIL_SLICEIMPL_HPP
#define HYPERTRIE_RAWNODECONTEXT_SLICEDETAIL_SLICEIMPL_HPP

#include "dice/hypertrie/internal/raw/node_context/SliceResult.hpp"
#include "dice/hypertrie/internal/util/Unreachable.hpp"

namespace dice::hypertrie::internal::raw::node_context::slice_detail {

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	typename htt_t::value_type get(NodePtr<depth, htt_t, allocator_type> node,
								   RawKey<depth, htt_t> const &key) noexcept;

	template<HypertrieTrait htt_t>
		requires (HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
	typename htt_t::value_type get(typename htt_t::key_part_type key_part,
								   RawKey<1, htt_t> const &key) noexcept {
		return key_part == key[0];
	}

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
	typename htt_t::value_type get(SENPtr<depth, htt_t, allocator_type> sen,
								   RawKey<depth, htt_t> const &key) noexcept {
		if (sen == nullptr) {
			return typename htt_t::value_type{};
		}

		if (sen->key() == key) {
			return sen->value();
		}

		return typename htt_t::value_type{};
	}

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	typename htt_t::value_type get(FNPtr<depth, htt_t, allocator_type> fn,
								   RawKey<depth, htt_t> const &key) noexcept {
		if (fn == nullptr) {
			return typename htt_t::value_type{};
		}

		if constexpr (depth > 1) {
			// TODO: we could benchmark if it makes a difference whether we use here random, max_card_pos or min_card_pos?
			auto pos = fn->min_card_pos();
			auto const child = fn->child(pos, key[pos]);

			if (child == nullptr) {
				return typename htt_t::value_type{};
			}

			return get(child, key.subkey(pos));
		} else {
			return fn->child(0, key[0]);
		}
	}

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	typename htt_t::value_type get(XNPtr<depth, htt_t, allocator_type> xn,
								   RawKey<depth, htt_t> const &key) noexcept {
		if (xn == nullptr) {
			return typename htt_t::value_type{};
		}

		if constexpr (HypertrieTrait_bool_valued<htt_t>) {
			bool value = true;
			xn->for_each_operand([&, read_ix = size_t{0}]<size_t, size_t operand_depth>(NodePtr<operand_depth, htt_t, allocator_type> const operand) mutable noexcept {
				if constexpr (operand_depth > 0) {
					if (value) {
						RawKey<operand_depth, htt_t> subkey; {
							assert((std::distance(key.begin() + read_ix, key.begin() + read_ix + operand_depth) == operand_depth));
							std::copy_n(key.begin() + read_ix, operand_depth, subkey.begin());
						}

						if (auto const child_value = get(operand, subkey); child_value == typename htt_t::value_type{}) {
							// check for presence only
							value = false;
						}

						read_ix += operand_depth;
					}
				}
			});

			return value;
		} else {
			// general cartesians do not exist, so must be xfix
			typename htt_t::value_type value{1};

			auto const high_order_operand_ix = xn->get_xfix_high_order_operand_index();
			xn->for_each_operand([&, read_ix = size_t{0}]<size_t ix, size_t operand_depth>(NodePtr<operand_depth, htt_t, allocator_type> const operand) mutable noexcept {
				if constexpr (operand_depth > 0) {
					if (value != typename htt_t::value_type{}) {
						RawKey<operand_depth, htt_t> subkey; {
							assert((std::distance(key.begin() + read_ix, key.begin() + read_ix + operand_depth) == operand_depth));
							std::copy_n(key.begin() + read_ix, operand_depth, subkey.begin());
						}

						if (ix == high_order_operand_ix) {
							assert(!operand.is_sen());
							value = get(operand, subkey);
						} else if constexpr (operand_depth == 1) {
							assert(operand.is_sen());
							// only check for presence
							if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
								if (operand.decode_key_part() != subkey[0]) {
									value = {};
								}
							} else {
								if (auto const child_value = get(operand, subkey); child_value == typename htt_t::value_type{}) {
									value = {};
								}
							}
						} else {
							// xfix cartesians only have exactly one higher-order operand
							HYPERTRIE_UNREACHABLE;
						}

						read_ix += operand_depth;
					}
				}
			});

			return value;
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
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	typename htt_t::value_type get(NodePtr<depth, htt_t, allocator_type> node,
								   RawKey<depth, htt_t> const &key) noexcept {
		if (node == nullptr) {
			return typename htt_t::value_type{};
		}

		switch (node.tag()) {
			case IdentifierTag::FN: {
				return get<depth, htt_t, allocator_type>(node.template specific_ptr<FullNode>(), key);
			}
			case IdentifierTag::SEN: {
				if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					return get(node.decode_key_part(), key);
				}

				return get<depth, htt_t, allocator_type>(node.template specific_ptr<SingleEntryNode>(), key);
			}
			case IdentifierTag::XN: {
				if constexpr (depth > 1) {
					return get<depth, htt_t, allocator_type>(node.template specific_ptr<CartesianNode>(), key);
				} else {
					HYPERTRIE_UNREACHABLE;
				}
			}
			case IdentifierTag::Indeterminate: {
				HYPERTRIE_UNREACHABLE;
			}
		}
	}

	template <size_t depth, size_t fixed_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	using specific_slice_result = std::conditional_t<(depth > fixed_depth), SliceResult<depth - fixed_depth, htt_t, allocator_type>, typename htt_t::value_type>;

	template<size_t depth, size_t fixed_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	specific_slice_result<depth, fixed_depth, htt_t, allocator_type> slice(NodePtr<depth, htt_t, allocator_type> node,
																		   RawSliceKey<fixed_depth, htt_t> const &raw_slice_key,
																		   SliceResultStorage<depth - fixed_depth, htt_t, allocator_type> *result_storage = nullptr) noexcept;

	template<size_t fixed_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (fixed_depth <= 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
	specific_slice_result<1, fixed_depth, htt_t, allocator_type> slice(typename htt_t::key_part_type key_part,
																	   RawSliceKey<fixed_depth, htt_t> const &slice_key) noexcept {
		using SliceResult_t = SliceResult<1 - fixed_depth, htt_t, allocator_type>;

		if constexpr (fixed_depth == 0) {
			return SliceResult_t{key_part};
		} else {
			return get(key_part, {slice_key[0].key_part});
		}
	}

	template<size_t depth, size_t fixed_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
	specific_slice_result<depth, fixed_depth, htt_t, allocator_type> slice(SENPtr<depth, htt_t, allocator_type> sen,
																		   RawSliceKey<fixed_depth, htt_t> const &raw_slice_key,
																		   SingleEntryNode<depth - fixed_depth, htt_t> *result_storage = nullptr) noexcept {
		static constexpr size_t result_depth = depth - fixed_depth;
		using SliceResult_t = SliceResult<depth - fixed_depth, htt_t, allocator_type>;

		if constexpr (fixed_depth == 0) {
			if (result_storage == nullptr) {
				return SliceResult_t{Ownership::Owned, new SingleEntryNode<depth, htt_t>{*sen}};
			}

			*result_storage = SingleEntryNode<depth, htt_t>{*sen};
			return SliceResult_t{Ownership::EphemeralBorrowed, result_storage};
		} else if constexpr (depth == fixed_depth) {
			return get(sen, raw_slice_key.template to_key<depth>());
		} else {
			auto const maybe_slice = raw_slice_key.slice(sen->key());
			if (!maybe_slice.has_value()) {
				return SliceResult_t{};
			}

			SingleEntry<result_depth, htt_t> entry{*maybe_slice, sen->value()};

			if constexpr (result_depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				return SliceResult_t{entry.key()[0]};
			} else {
				if (result_storage == nullptr) {
					return SliceResult_t{Ownership::Owned, new SingleEntryNode<result_depth, htt_t>{entry, 0}};
				}

				*result_storage = SingleEntryNode<result_depth, htt_t>{entry, 0};
				return SliceResult_t{Ownership::EphemeralBorrowed, result_storage};
			}
		}
	}

	template<size_t depth, size_t fixed_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	SliceResult<depth - fixed_depth, htt_t, allocator_type> slice_cartesian_node_rek(CartesianNode<depth, htt_t, allocator_type> const &xnode,
																					 RawSliceKey<fixed_depth, htt_t> const &slice_key,
																					 SliceResultStorage<depth - fixed_depth, htt_t, allocator_type> *result_storage = nullptr) noexcept {
		using SliceResult_t = SliceResult<depth - fixed_depth, htt_t, allocator_type>;

		if constexpr (fixed_depth == 0) {
			if constexpr (depth > 1) {
				if (result_storage == nullptr) {
					return SliceResult_t{Ownership::Owned, new CartesianNode<depth, htt_t, allocator_type>{xnode}};
				}

				result_storage->xn = xnode;
				return SliceResult_t{Ownership::EphemeralBorrowed, &result_storage->xn};
			} else {
				HYPERTRIE_UNREACHABLE;
			}
		} else if constexpr (fixed_depth == depth) {
			return get<depth, htt_t, allocator_type>(&xnode, slice_key.template to_key<depth>());
		} else {
			auto const [pos, key_part] = slice_key[0];
			auto const [slice_now, slice_rest] = xnode.discriminant().slice_index(pos);

			return dice::template_library::switch_cases<1, depth>(xnode.discriminant()[slice_now], [&, slice_now = slice_now, slice_rest = slice_rest, key_part = key_part](auto child_depth) -> SliceResult_t {
				auto const operand = static_cast<NodePtr<child_depth, htt_t, allocator_type>>(xnode.operand(slice_now));

				switch (operand.tag()) {
					case IdentifierTag::FN: {
						auto const fn_child = operand.template specific_ptr<FullNode>();

						if constexpr (child_depth == 1) {
							auto const sub_child = fn_child->child(slice_rest, key_part);
							if (sub_child == decltype(sub_child){}) {
								return SliceResult_t{};
							}

							auto const subxnode = xnode.drop_operand(slice_now, fn_child->size());

							if (subxnode.is_fully_sen()) {
								assert(xnode.is_xfix_cartesian());
								assert(*xnode.get_xfix_high_order_operand_index() == slice_now);

								SingleEntry<depth - 1, htt_t> se;

								if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
									se.value_mut() = sub_child;
								}

								for (size_t ix = 0; ix < subxnode.n_operands(); ++ix) {
									assert(subxnode.discriminant()[ix] == 1);
									auto const subop = static_cast<NodePtr<1, htt_t, allocator_type>>(subxnode.operand(ix));

									if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
										se.key()[ix] = subop.decode_key_part();
									} else {
										auto const sen_ptr = subop.template specific_ptr<SingleEntryNode>();
										se.key()[ix] = sen_ptr->key()[0];
									}
								}

								if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
									return slice<fixed_depth - 1, htt_t, allocator_type>(se.key()[0], slice_key.subkey_i(0));
								} else {
									SingleEntryNode<depth - 1, htt_t> sen{se, 0};

									// this call is fine ownership wise because slice(SEN) will create a copy for the slice result
									return slice<depth - 1, fixed_depth - 1, htt_t, allocator_type>(&sen,
																									slice_key.subkey_i(0),
																									result_storage == nullptr ? nullptr : &result_storage->sen);
								}
							}

							if (subxnode.n_operands() == 1) {
								auto const roperand = static_cast<NodePtr<depth - 1, htt_t, allocator_type>>(xnode.operand(1 - slice_now));
								return slice<depth - 1, fixed_depth - 1, htt_t, allocator_type>(roperand, slice_key.subkey_i(0), result_storage);
							}

							return slice_cartesian_node_rek(subxnode, slice_key.subkey_i(0), result_storage);
						} else {
							// must be xfix cartesian, otherwise there would be no high order operand
							auto const sub_child = fn_child->child(slice_rest, key_part);

							if (sub_child == decltype(sub_child){}) {
								return SliceResult_t{};
							}

							switch (sub_child.tag()) {
								case IdentifierTag::SEN: {
									// this is an xfix cartesian, therefore if we hit
									// a sen while slicing the high order operand the whole result
									// must be a sen

									auto const entry = [child_depth, sub_child]() noexcept {
										if constexpr (child_depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
											return SingleEntry<1, htt_t>{{sub_child.decode_key_part()}};
										} else {
											auto const sen_ptr = sub_child.template specific_ptr<SingleEntryNode>();
											return SingleEntry<child_depth - 1, htt_t>{sen_ptr->key(), sen_ptr->value()};
										}
									}();

									SingleEntry<depth - 1, htt_t> se;
									if constexpr (!HypertrieTrait_bool_valued<htt_t>) {
										se.value_mut() = entry.value();
									}

									for (size_t ix = 0; ix < slice_now; ++ix) {
										auto const poperand = static_cast<NodePtr<1, htt_t, allocator_type>>(xnode.operand(ix));

										if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
											se.key()[ix] = poperand.decode_key_part();
										} else {
											auto const sp = poperand.template specific_ptr<SingleEntryNode>();
											se.key()[ix] = sp->key()[0];
										}
									}

									for (size_t ix = 0; ix < child_depth - 1; ++ix) {
										se.key()[slice_now + ix] = entry.key()[ix];
									}

									for (size_t ix = 0; ix < xnode.n_operands() - slice_now - 1; ++ix) {
										auto const poperand = static_cast<NodePtr<1, htt_t, allocator_type>>(xnode.operand(slice_now + 1 + ix));

										if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
											se.key()[slice_now + child_depth - 1 + ix] = poperand.decode_key_part();
										} else {
											auto const sp = poperand.template specific_ptr<SingleEntryNode>();
											se.key()[slice_now + child_depth - 1 + ix] = sp->key()[0];
										}
									}

									if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
										return slice<fixed_depth - 1, htt_t, allocator_type>(se.key()[0], slice_key.subkey_i(0));
									} else {
										SingleEntryNode<depth - 1, htt_t> sen{se, 0};

										// this call is fine ownership wise because slice(SEN) will create a copy for the slice result
										return slice<depth - 1, fixed_depth - 1, htt_t, allocator_type>(&sen,
																										slice_key.subkey_i(0),
																										result_storage == nullptr ? nullptr : &result_storage->sen);
									}
								}
								case IdentifierTag::XN: {
									// this is an xfix cartesian, therefore if we hit
									// an xn while slicing the high order operand the whole result
									// must be a xn

									if constexpr (child_depth - 1 > 1) {
										auto const sub_xn = sub_child.template specific_ptr<CartesianNode>();
										auto const new_xnode = xnode.replace_operand_flatten(slice_now, *sub_xn, std::make_pair(fn_child->size(), sub_xn->size()));
										return slice_cartesian_node_rek(new_xnode, slice_key.subkey_i(0), result_storage);
									} else {
										HYPERTRIE_UNREACHABLE;
									}
								}
								case IdentifierTag::FN: {
									auto const new_xnode = xnode.replace_operand(slice_now, sub_child, std::make_pair(fn_child->size(), sub_child.size()));
									return slice_cartesian_node_rek(new_xnode, slice_key.subkey_i(0), result_storage);
								}
								case IdentifierTag::Indeterminate: {
									HYPERTRIE_UNREACHABLE;
								}
							}
						}
					}
					case IdentifierTag::SEN: {
						if constexpr (child_depth == 1) {
							auto const child_keyp = [&]() noexcept {
								if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
									return operand.decode_key_part();
								} else {
									auto const sen_child = operand.template specific_ptr<SingleEntryNode>();
									return sen_child->key()[0];
								}
							}();

							if (child_keyp != key_part) {
								return SliceResult_t{};
							}

							if (xnode.n_operands() - 1 == 1) {
								auto const roperand = static_cast<NodePtr<depth - child_depth, htt_t, allocator_type>>(xnode.operand(1 - slice_now));
								return slice(roperand, slice_key.subkey_i(0), result_storage);
							}

							auto const subxnode = xnode.drop_operand(slice_now, 1);
							return slice_cartesian_node_rek(subxnode, slice_key.subkey_i(0), result_storage);
						} else {
							HYPERTRIE_UNREACHABLE;
						}
					}
					default: {
						HYPERTRIE_UNREACHABLE;
					}
				}
			});
		}
	}

	template<size_t depth, size_t fixed_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	specific_slice_result<depth, fixed_depth, htt_t, allocator_type> slice(XNPtr<depth, htt_t, allocator_type> xn,
																		   RawSliceKey<fixed_depth, htt_t> const &slice_key,
																		   SliceResultStorage<depth - fixed_depth, htt_t, allocator_type> *result_storage = nullptr) noexcept {
		using SliceResult_t = SliceResult<depth - fixed_depth, htt_t, allocator_type>;

		if constexpr (fixed_depth == 0) {
			return SliceResult_t{Ownership::ContextBorrowed, xn->identifier(), xn};
		} else if constexpr (fixed_depth == depth) {
			return get(xn, slice_key.template to_key<depth>());
		} else {
			return slice_cartesian_node_rek(*xn, slice_key, result_storage);
		}
	}

	template<size_t depth, size_t fixed_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	specific_slice_result<depth, fixed_depth, htt_t, allocator_type> slice_rek(NodePtr<depth, htt_t, allocator_type> node,
																			   RawSliceKey<fixed_depth, htt_t> const &slice_key,
																			   SliceResultStorage<depth - fixed_depth, htt_t, allocator_type> *result_storage = nullptr) noexcept {

		static constexpr size_t result_depth = depth - fixed_depth;
		using SliceResult_t = SliceResult<result_depth, htt_t, allocator_type>;

		switch (node.tag()) {
			case IdentifierTag::FN: {
				auto const fn = node.template specific_ptr<FullNode>();
				size_t const slice_key_i = fn->min_fixed_keypart_i(slice_key);
				auto const &[pos, key_part] = slice_key[slice_key_i];

				auto const child = fn->child(pos, key_part);
				if (child == decltype(child){}) {
					return SliceResult_t{};
				}

				if constexpr (fixed_depth - 1 == 0) {
					return SliceResult_t{Ownership::ContextBorrowed, child};
				} else {
					return slice_rek(child, slice_key.subkey_i(slice_key_i), result_storage);
				}
			}
			case IdentifierTag::SEN: {
				if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					return slice(node.decode_key_part(), slice_key);
				} else {
					return slice<depth, fixed_depth, htt_t, allocator_type>(node.template specific_ptr<SingleEntryNode>(),
																			slice_key,
																			result_storage == nullptr ? nullptr : &result_storage->sen);
				}
			}
			case IdentifierTag::XN: {
				if constexpr (depth > 1) {
					return slice<depth, fixed_depth, htt_t, allocator_type>(node.template specific_ptr<CartesianNode>(),
																			slice_key,
																			result_storage);
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
	 * Returns a pair of a node container and a boolean which states if the pointed node is managed (true) or if it is unmanaged (false) and MUST be deleted by the user manually.
	 * Only compressed nodes can be managed.
	 * if fixed_depth == depth, just a scalar is returned
	 * @tparam depth depth of the node container
	 * @tparam fixed_depth number of fixed key_parts in the slice key
	 * @param nodec a container with a node.
	 * @param raw_slice_key the slice key
	 * @return see above
	 */
	template<size_t depth, size_t fixed_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	specific_slice_result<depth, fixed_depth, htt_t, allocator_type> slice(NodePtr<depth, htt_t, allocator_type> node,
																		   RawSliceKey<fixed_depth, htt_t> const &raw_slice_key,
																		   SliceResultStorage<depth - fixed_depth, htt_t, allocator_type> *result_storage) noexcept {
		using SliceResult_t = SliceResult<depth - fixed_depth, htt_t, allocator_type>;

		if constexpr (fixed_depth == 0) {
			return SliceResult_t{Ownership::ContextBorrowed, node};
		} else if constexpr (depth == fixed_depth) {
			return get(node, raw_slice_key.template to_key<depth>());
		} else {
			return slice_rek(node, raw_slice_key, result_storage);
		}
	}

	template<size_t diag_depth, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	specific_slice_result<depth, diag_depth, htt_t, allocator_type> diagonal_slice(NodePtr<depth, htt_t, allocator_type> node,
																				   RawKeyPositions<depth> const &positions,
																				   typename htt_t::key_part_type fixed_key_part,
																				   SliceResultStorage<depth - diag_depth, htt_t, allocator_type> *result_storage) noexcept {
		return slice<depth, diag_depth, htt_t, allocator_type>(node,
															   positions.template to_slice_key<diag_depth, htt_t>(fixed_key_part),
															   result_storage);
	}

} // namespace dice::hypertrie::internal::raw::node_context::slice_detail

#endif//HYPERTRIE_RAWNODECONTEXT_SLICEDETAIL_SLICEIMPL_HPP
