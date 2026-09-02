#ifndef HYPERTRIE_RAWNODECONTEXT_CARTESIANDETAIL_CARTESIANUTIL_HPP
#define HYPERTRIE_RAWNODECONTEXT_CARTESIANDETAIL_CARTESIANUTIL_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <numeric>
#include <variant>

#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/CartesianNode.hpp"
#include "dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "dice/hypertrie/internal/raw/node/NodeStorage.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"
#include "dice/hypertrie/internal/raw/node_context/common_detail/Container.hpp"
#include "dice/hypertrie/internal/util/Overloaded.hpp"
#include "dice/hypertrie/internal/util/Unreachable.hpp"
#include "dice/template-library/integral_template_variant.hpp"

namespace dice::hypertrie::internal::raw::node_context::common_detail {

	/**
	 * An operand in a cartesian product.
	 */
	template<size_t operand_depth, HypertrieTrait htt_t>
	using CartesianOperand = std::vector<SingleEntry<operand_depth, htt_t>>;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	using FullNodeEdgesPtr = typename FullNode<depth, htt_t, allocator_type>::single_dim_edges_type const *;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct AnyFullNodeEdgesPtr {
		template<size_t fn_depth>
		using FullNodeEdgesPtr_t = FullNodeEdgesPtr<fn_depth, htt_t, allocator_type>;

		using type = template_library::integral_template_variant<1UL, depth, FullNodeEdgesPtr_t>;
	};

	/**
	 * The semantics of this are essentially identical to CartesianOperand
	 * except that these operands are just pointers to already existing data in
	 * the hypertrie, i.e. the edge maps in full nodes
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	using ExtractedOperand = std::variant<typename htt_t::key_part_type,
										  typename AnyFullNodeEdgesPtr<depth, htt_t, allocator_type>::type>;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	using ExtractedOperands = std::array<ExtractedOperand<depth, htt_t, allocator_type>, depth>;

	/**
	 * Calculates the operands of the cartesian product spanned by entries.
	 * The result is incorrect if entries is not indeed a cartesian product.
	 * Note: in valued configurations the result operand's values will always be filled with 1s.
	 */
	template<size_t depth, HypertrieTrait htt_t>
		requires (depth > 1)
	std::array<CartesianOperand<1, htt_t>, depth> inverse_cartesian_product(std::vector<SingleEntry<depth, htt_t>> const &entries) noexcept {
		std::array<Set<typename htt_t::key_part_type>, depth> dedup_operands;
		for (auto const &entry : entries) {
			for (size_t key_ix = 0; key_ix < depth; ++key_ix) {
				auto const key_part = entry.key()[key_ix];

				if (!dedup_operands[key_ix].contains(key_part)) {
					dedup_operands[key_ix].insert(key_part);
				}
			}
		}

		std::array<CartesianOperand<1, htt_t>, depth> ret_operands;
		for (size_t ix = 0; ix < depth; ++ix) {
			auto &ret_operand = ret_operands[ix];
			auto const &dedup_operand = dedup_operands[ix];

			ret_operand.reserve(dedup_operand.size());
			for (auto const key_part : dedup_operand) {
				ret_operand.push_back(SingleEntry<1, htt_t>{{key_part}, typename htt_t::value_type{1} /*dummy value*/});
			}
		}

		return ret_operands;
	}

	/**
	 * Given a cartesian product with operand sizes of operand_sizes
	 * return which operand elements need to be removed if you were to remove entries
	 *
	 * @param entries the entries to remove
	 * @param operand_sizes the operand sizes of the to be removed from cartesian product
	 * @return the to be removed cartesian operands
	 * TODO find better name
	 */
	template<size_t depth, HypertrieTrait htt_t>
		requires (depth > 1)
	std::array<CartesianOperand<1, htt_t>, depth> inverse_cartesian_product2(std::vector<SingleEntry<depth, htt_t>> const &entries,
																			 std::array<size_t, depth> const &operand_sizes) noexcept {
		auto const cartesian_size_without_pos = [&](size_t exclude_pos) noexcept {
			size_t acc = 1;

			for (size_t pos = 0; pos < exclude_pos; ++pos) {
				acc *= operand_sizes[pos];
			}

			for (size_t pos = exclude_pos + 1; pos < depth; ++pos) {
				acc *= operand_sizes[pos];
			}

			return acc;
		};

		// count the number of occurrences of key parts for all dimensions
		std::array<Map<typename htt_t::key_part_type, size_t>, depth> key_part_counts;
		for (auto const &e : entries) {
			for (size_t key_ix = 0; key_ix < depth; ++key_ix) {
				if (auto it = key_part_counts[key_ix].find(e.key()[key_ix]); it != key_part_counts[key_ix].end()) {
					it->second += 1;
				} else {
					key_part_counts[key_ix].emplace(e.key()[key_ix], 1);
				}
			}
		}

		std::array<CartesianOperand<1, htt_t>, depth> operands;
		for (size_t pos = 0; pos < depth; ++pos) {
			for (auto const &[key_part, count] : key_part_counts[pos]) {
				if (count == cartesian_size_without_pos(pos)) {
					// operand element will get removed from cartesian product iff this condition holds
					operands[pos].emplace_back(SingleEntry<1, htt_t>{{key_part}, typename htt_t::value_type{1}});
				}
			}
		}

		return operands;
	}

	/**
	 * Calculates the size of the cartesian product resulting from operands of the given sizes
	 */
	template<size_t depth>
		requires (depth > 1)
	inline size_t cartesian_size(std::array<size_t, depth> const &operand_sizes) noexcept {
		return std::accumulate(operand_sizes.begin(), operand_sizes.end(), size_t{1}, std::multiplies<>{});
	}

	/**
	 * Calculates the size of the cartesian product resulting from its operands
	 */
	template<size_t depth, HypertrieTrait htt_t>
		requires (depth > 1)
	inline size_t cartesian_size(std::array<CartesianOperand<1, htt_t>, depth> const &operands) noexcept {
		return std::accumulate(operands.begin(), operands.end(), size_t{1}, [](auto acc, auto const &operand) noexcept {
			return acc * operand.size();
		});
	}

	/**
	 * Check if the cartesian operands of sizes operand_sizes result in a cartesian product of size actual_size
	 */
	template<HypertrieTrait htt_t, size_t depth>
		requires (depth > 1 && HypertrieTrait_bool_valued<htt_t>)
	inline bool is_general_cartesian(std::array<size_t, depth> const &operand_sizes, size_t const actual_size) noexcept {
		return cartesian_size(operand_sizes) == actual_size;
	}

	/**
	 * Return if the given cartesian operands expand into a node of size new_node_size
	 */
	template<HypertrieTrait htt_t, size_t depth>
		requires (depth > 1 && HypertrieTrait_bool_valued<htt_t>)
	inline bool is_general_cartesian(std::array<CartesianOperand<1, htt_t>, depth> const &operands, size_t const new_node_size) noexcept {
		return cartesian_size(operands) == new_node_size;
	}

	/**
	 * The immediately observable properties of an xfix-cartesian
	 */
	struct XFixCartesianProperties {
		size_t prefix_len;
		size_t postfix_len;

		template<size_t upper_bound_depth, typename F>
		decltype(auto) visit(F &&f) const noexcept {
			using ret_type = decltype(std::forward<F>(f).template operator()<size_t{1}, size_t{0}>());

			assert(prefix_len < upper_bound_depth);
			assert(postfix_len < upper_bound_depth);

			return template_library::switch_cases<0, upper_bound_depth>(prefix_len, [&](auto pre) noexcept -> ret_type {
				return template_library::switch_cases<0, upper_bound_depth>(postfix_len, [&](auto post) noexcept -> ret_type {
					if constexpr (pre + post < upper_bound_depth && (pre > 0 || post > 0)) {
						return std::forward<F>(f).template operator()<size_t{pre}, size_t{post}>();
					} else {
						HYPERTRIE_UNREACHABLE;
					}
				});
			});
		}

		constexpr bool operator==(XFixCartesianProperties const &) const noexcept = default;
	};

	/**
	 * Try to calculate the xfix cartesian properties from the given operand sizes.
	 * Return nullopt if operand sizes do not form an xfix cartesian,
	 * otherwise return the prefix and postfix lengths of the xfix cartesian
	 */
	template<size_t depth>
		requires (depth > 1)
	inline std::optional<XFixCartesianProperties> try_get_xfix_cartesian_properties(std::array<size_t, depth> const &operand_sizes) noexcept {
		size_t prefix_len = 0;
		for (; prefix_len < depth; ++prefix_len) {
			if (operand_sizes[prefix_len] != 1) {
				break;
			}
		}

		size_t postfix_len = 0;
		for (; postfix_len < depth; ++postfix_len) {
			if (operand_sizes[operand_sizes.size() - postfix_len - 1] != 1) {
				break;
			}
		}

		// explicitly _not_ checking for general cartesian case here
		// because it should be ruled out before calling this function
		if (prefix_len == 0 && postfix_len == 0) {
			return std::nullopt;
		}

		return XFixCartesianProperties{.prefix_len = prefix_len,
									   .postfix_len = postfix_len};
	}

	/**
	 * Try to calculate the xfix cartesian properties from the given operands.
	 * Return nullopt if operands do not form an xfix cartesian,
	 * otherwise return the prefix and postfix lengths of the xfix cartesian expanded by operands.
	 */
	template<size_t depth, HypertrieTrait htt_t>
		requires (depth > 1)
	std::optional<XFixCartesianProperties> try_get_xfix_cartesian_properties(std::array<CartesianOperand<1, htt_t>, depth> const &operands) noexcept {
		auto const pred = [](auto const &operand) noexcept {
			return operand.size() == 1;
		};

		auto const prefix_end = std::find_if_not(operands.begin(), operands.end(), pred);
		auto const postfix_end = std::find_if_not(operands.rbegin(), operands.rend(), pred);

		// explicitly _not_ checking for general cartesian case here
		// because it should be ruled out before calling this function
		if ((prefix_end == operands.begin() && postfix_end == operands.rbegin())) {
			return std::nullopt;
		}

		auto const prefix_len = std::distance(operands.begin(), prefix_end);
		auto const postfix_len = std::distance(operands.rbegin(), postfix_end);

		return XFixCartesianProperties{.prefix_len = static_cast<size_t>(prefix_len),
									   .postfix_len = static_cast<size_t>(postfix_len)};
	}

	/**
	 * Extract the operand sizes from the given operands
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	std::array<size_t, depth> get_operand_sizes(ExtractedOperands<depth, htt_t, allocator_type> const &existing_operands) noexcept {
		auto const calc_operand_size = [](auto const &operand) noexcept {
			return std::visit(util::Overloaded{
									  [&](typename htt_t::key_part_type) noexcept -> size_t {
										  return 1;
									  },
									  [&](auto const &edges) noexcept {
										  return edges.visit([&](auto const edges_ptr) noexcept {
											  return edges_ptr->size();
										  });
									  }},
							  operand);
		};

		std::array<size_t, depth> ret;
		for (size_t ix = 0; ix < depth; ++ix) {
			ret[ix] = calc_operand_size(existing_operands[ix]);
		}

		return ret;
	}

	/**
	 * Calculate the future operand sizes of the cartesian formed by existing operands
	 * after the insertion of newly_inserted operands
	 *
	 * @tparam depth
	 * @tparam htt_t
	 * @tparam allocator_type
	 * @param existing_operands operands extracted from a _cartesian_ node
	 * @param newly_inserted_operands only the new/not yet contained new operand elements (e.g. as obtained by calculate_newly_inserted_operand_members)
	 * @return the future operand sizes the the cartesian
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	std::array<size_t, depth> calculate_future_operand_sizes_after_insertion(ExtractedOperands<depth, htt_t, allocator_type> const &existing_operands,
																			 std::array<CartesianOperand<1, htt_t>, depth> const &newly_inserted_operands) noexcept {
		auto const calc_future_operand_size = [](auto const &operand, auto const &new_operand_members) noexcept {
			return std::visit(util::Overloaded{
									  [&](typename htt_t::key_part_type) noexcept {
										  return 1 + new_operand_members.size();
									  },
									  [&](auto const &edges) noexcept {
										  return edges.visit([&](auto const edges_ptr) noexcept {
											  return edges_ptr->size() + new_operand_members.size();
										  });
									  }},
							  operand);
		};

		std::array<size_t, depth> ret;
		for (size_t ix = 0; ix < depth; ++ix) {
			ret[ix] = calc_future_operand_size(existing_operands[ix], newly_inserted_operands[ix]);
		}

		return ret;
	}

	/**
	 * Calculate the future edge sizes of a given full node after the insertion of
	 * newly_inserted_children (as obtained by a call to `CommonChangeImpl::entry_subsets`)
	 *
	 * @param fn the original full node before insertion
	 * @param newly_inserted_children the entry subsets to be inserted
	 * @return fn's future edge sizes
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	std::array<size_t, depth> calculate_future_edge_counts_after_insertion(FullNode<depth, htt_t, allocator_type> const &fn,
																		   std::array<Map<typename htt_t::key_part_type,
																						  std::vector<SingleEntry<depth - 1, htt_t>>>, depth> const &newly_inserted_children) noexcept {
		auto const calc_future_edges_count = [](auto const &edges, auto const &newly_inserted_children) noexcept {
			auto const new_edges_count = std::accumulate(newly_inserted_children.begin(), newly_inserted_children.end(), size_t{0}, [&](auto const acc, auto const &elem) noexcept {
				auto const key_part = elem.first;
				return acc + (edges.contains(key_part) ? 0 : 1);
			});

			return edges.size() + new_edges_count;
		};

		std::array<size_t, depth> ret;
		for (size_t pos = 0; pos < depth; ++pos) {
			ret[pos] = calc_future_edges_count(fn.edges(pos), newly_inserted_children[pos]);
		}

		return ret;
	}

	/**
	 * Calculate the future edge sizes of a given full node after the removal of
	 * removed_entries (as obtained by a call to `CommonChangeImpl::entry_subsets`)
	 *
	 * @param fn the original full node before removal
	 * @param removed_entries the entry subsets to be removed
	 * @return fn's future edge sizes
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	std::array<size_t, depth> calculate_future_edge_counts_after_removal(FullNode<depth, htt_t, allocator_type> const &fn,
																		 std::array<Map<typename htt_t::key_part_type,
																						std::vector<SingleEntry<depth - 1, htt_t>>>, depth> const &removed_entries) noexcept {
		auto const calc_future_edges_count = [&](auto const &edges, auto const &removed_entries) noexcept {
			auto const removed_edges_count = std::accumulate(removed_entries.begin(), removed_entries.end(), size_t{0}, [&](auto const acc, auto const &elem) noexcept {
				auto const key_part = elem.first;
				auto const &to_remove = elem.second;

				if constexpr (depth == 1) {
					return acc + 1;
				} else {
					return acc + (edges.find(key_part)->second.size() - to_remove.size() == 0);
				}
			});

			return edges.size() - removed_edges_count;
		};

		std::array<size_t, depth> ret;
		for (size_t pos = 0; pos < depth; ++pos) {
			ret[pos] = calc_future_edges_count(fn.edges(pos), removed_entries[pos]);
		}

		return ret;
	}

	/**
	 * Extract the cartesian operands from a given full node
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	ExtractedOperands<depth, htt_t, allocator_type> extract_operands(FullNode<depth, htt_t, allocator_type> const &fn) noexcept {
		ExtractedOperands<depth, htt_t, allocator_type> operands;
		for (size_t pos = 0; pos < depth; ++pos) {
			operands[pos] = typename AnyFullNodeEdgesPtr<depth, htt_t, allocator_type>::type{std::in_place_type<FullNodeEdgesPtr<depth, htt_t, allocator_type>>,
																							 &fn.edges(pos)};
		}
		return operands;
	}

	/**
	 * Extract the cartesian operands from a given cartesian node
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	ExtractedOperands<depth, htt_t, allocator_type> extract_operands(CartesianNode<depth, htt_t, allocator_type> const &xn) noexcept {
		ExtractedOperands<depth, htt_t, allocator_type> operands;

		xn.for_each_operand([&, write_ix = size_t{0}]<size_t ix, size_t operand_depth>(NodePtr<operand_depth, htt_t, allocator_type> const operand) mutable noexcept {
			if constexpr (operand_depth > 0) {
				switch (operand.tag()) {
					case IdentifierTag::FN: {
						auto const fn_ptr = operand.template specific_ptr<FullNode>();
						for (size_t pos = 0; pos < operand_depth; ++pos) {
							operands[write_ix + pos] = typename AnyFullNodeEdgesPtr<depth, htt_t, allocator_type>::type{std::in_place_type<FullNodeEdgesPtr<operand_depth, htt_t, allocator_type>>,
																														&fn_ptr->edges(pos)};
						}

						write_ix += operand_depth;
						break;
					}
					case IdentifierTag::SEN: {
						if constexpr (operand_depth == 1) {
							if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
								operands[write_ix] = operand.decode_key_part();
							} else {
								auto const sen_ptr = operand.template specific_ptr<SingleEntryNode>();
								operands[write_ix] = sen_ptr->key()[0];
							}
							write_ix += 1;
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
		});

		return operands;
	}

	/**
	 * Given a node from which extracted_operands are extracted
	 * and new_operands which were obtained from a insert set fed into inverse_cartesian_product
	 * calculate the new/not yet contained operand elements that need to be inserted into the nodes operands.
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	std::array<CartesianOperand<1, htt_t>, depth> calculate_newly_inserted_operand_members(ExtractedOperands<depth, htt_t, allocator_type> const &existing_operands,
																						   std::array<CartesianOperand<1, htt_t>, depth> &&new_operands) noexcept {
		auto const calc_newly_inserted_operand_members = [](auto const &operand, CartesianOperand<1, htt_t> &&new_operand_members, CartesianOperand<1, htt_t> &out) noexcept {
			std::visit(util::Overloaded{
							   [&](typename htt_t::key_part_type const key_part) noexcept {
								   auto it = std::find_if(new_operand_members.begin(), new_operand_members.end(), [&](auto const &entry) noexcept {
									   return entry.key()[0] == key_part;
								   });

								   if (it != new_operand_members.end()) {
									   new_operand_members.erase(it);
								   }

								   out = std::move(new_operand_members);
							   },
							   [&](auto const &edges) noexcept {
								   edges.visit([&](auto const edges_ptr) noexcept {
									   out.reserve(new_operand_members.size());// definitely upper bound
									   std::copy_if(std::move_iterator(new_operand_members.begin()), std::move_iterator(new_operand_members.end()), std::back_inserter(out), [&](auto const &entry) noexcept {
										   return !edges_ptr->contains(entry.key()[0]);
									   });
								   });
							   }},
					   operand);
		};

		std::array<CartesianOperand<1, htt_t>, depth> ret;
		for (size_t ix = 0; ix < depth; ++ix) {
			calc_newly_inserted_operand_members(existing_operands[ix], std::move(new_operands[ix]), ret[ix]);
		}

		return ret;
	}

	/**
	 * Merge the existing operands from a node with the to be inserted/not yet contained/new operands
	 * obtained by a call to calculate_newly_inserted_operand_members
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	std::array<CartesianOperand<1, htt_t>, depth> merge_operands(ExtractedOperands<depth, htt_t, allocator_type> const &existing_operands,
																 std::array<CartesianOperand<1, htt_t>, depth> &&newly_inserted_operands) noexcept {
		std::array<CartesianOperand<1, htt_t>, depth> result_operands;

		for (size_t ix = 0; ix < depth; ++ix) {
			auto &out = result_operands[ix];
			auto &&new_operand_members = newly_inserted_operands[ix];

			std::visit(util::Overloaded{
							   [&](typename htt_t::key_part_type const key_part) noexcept {
								   auto it = std::lower_bound(new_operand_members.begin(), new_operand_members.end(), key_part, [](auto const &entry, auto const key_part) noexcept {
									   return entry.key()[0] < key_part;
								   });

								   // we know key_part is new we just had to find its location
								   new_operand_members.insert(it, SingleEntry<1, htt_t>{{key_part}, typename htt_t::value_type{1}});
								   out = std::move(new_operand_members);
							   },
							   [&](auto const &edges) noexcept {
								   edges.visit([&](auto const edges_ptr) noexcept {
									   out = std::move(new_operand_members);
									   out.reserve(edges_ptr->size() + new_operand_members.size());

									   std::transform(edges_ptr->begin(), edges_ptr->end(), std::back_inserter(out),
													  util::Overloaded{
															  [](typename htt_t::key_part_type const key_part) noexcept {
																  return SingleEntry<1, htt_t>{{key_part}, typename htt_t::value_type{1}};
															  },
															  [](auto const &edge) noexcept {
																  return SingleEntry<1, htt_t>{{edge.first}, typename htt_t::value_type{1}};
															  }});
								   });
							   }
					   },
					   existing_operands[ix]);
		}

		return result_operands;
	}

	/**
	 * Kind of the opposite of merge_operands.
	 * Calculates the future operands of an existing full-node after the removal of the given entry subsets.
	 */
	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires (depth > 1)
	std::array<CartesianOperand<1, htt_t>, depth> unmerge_operands(ExtractedOperands<depth, htt_t, allocator_type> const &existing_operands,
																	std::array<Map<typename htt_t::key_part_type,
																				   std::vector<SingleEntry<depth - 1, htt_t>>>, depth> const &change_operands) noexcept {
		std::array<CartesianOperand<1, htt_t>, depth> result_operands;

		for (size_t ix = 0; ix < depth; ++ix) {
			auto &out = result_operands[ix];
			auto const &removed_operand_members = change_operands[ix];

			std::visit(util::Overloaded{
							   [&](typename htt_t::key_part_type const key_part) noexcept {
								   assert(removed_operand_members.size() < 2);

								   if (removed_operand_members.size() == 1) {
									   assert(removed_operand_members.contains(key_part));
								   } else {
									   out = CartesianOperand<1, htt_t>{SingleEntry<1, htt_t>{{key_part}, typename htt_t::value_type{1}}};
								   }
							   },
							   [&](auto const &edges) noexcept {
								   edges.visit([&](auto const edges_ptr) noexcept {
									   for (auto const &edge : *edges_ptr) {
										   if constexpr (std::is_same_v<std::remove_cvref_t<decltype(edge)>, typename htt_t::key_part_type>) {
											   if (!removed_operand_members.contains(edge)) {
												   out.emplace_back(SingleEntry<1, htt_t>{{edge}, typename htt_t::value_type{1}});
											   }
										   } else {
											   auto const remove_size = [&]() -> size_t {
												   if (auto it = removed_operand_members.find(edge.first); it != removed_operand_members.end()) {
													   return it->second.size();
												   }

												   return 0;
											   }();

											   if (edge.second.size() - remove_size > 0) {
												   out.emplace_back(SingleEntry<1, htt_t>{{edge.first}, typename htt_t::value_type{1}});
											   }
										   }
									   }
								   });
							   }
					   },
					   existing_operands[ix]);
		}

		return result_operands;
	}

}  //namespace dice::hypertrie::internal::raw::node_context::common_detail

#endif//HYPERTRIE_RAWNODECONTEXT_CARTESIANDETAIL_CARTESIANUTIL_HPP
