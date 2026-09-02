#ifndef HYPERTRIE_VALIDATIONRAWNODECONTEXT_HPP
#define HYPERTRIE_VALIDATIONRAWNODECONTEXT_HPP

#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>

#include <fmt/format.h>
#include <cppitertools/itertools.hpp>
#include <dice/sparse-map/sparse_map.hpp>

#include <dice/hash/DiceHash.hpp>
#include <dice/hypertrie/internal/commons/PosType.hpp>

#include <dice/template-library/for.hpp>

namespace dice::hypertrie::tests::core::node {
	using namespace fmt::literals;

	using namespace ::dice::hypertrie::internal::raw;
	using namespace ::dice::hypertrie::internal;

	/* The Equal struct needs a way to know whether a type is a set, map or something completely different.
	 * This namespace collects different concepts to help with this problem.
	 */
	namespace EqualTraits {
		template <typename T>
		concept ProbablyMap = requires (T t) {
			typename T::key_type;
			typename T::mapped_type;
			{*t.begin()} -> std::convertible_to<std::pair<typename T::key_type const, typename T::mapped_type>>;
		};

		template <typename T>
		concept ProbablySet = requires (T t) {
			typename T::key_type;
			{*t.begin()} -> std::convertible_to<typename T::key_type const>;
		};

		template <typename, typename>
		struct EqualCheckPossible : std::false_type{};
		template <typename T, typename V> requires requires (T t, V v) {t == v;}
		struct EqualCheckPossible<T,V> : std::true_type{};
	}

	struct Equal {
		template <typename Set1, typename Set2>
		static bool set_equal(Set1 const& lhs, Set2 const& rhs) {
			if(lhs.size() != rhs.size()) {return false;}
			for (auto const & key : rhs) {
				if (lhs.find(key) == lhs.end()) {return false;}
			}
			return true;
		}

		template <typename Map1, typename Map2>
		static bool map_equal(Map1 const& lhs, Map2 const& rhs) {
			if(lhs.size() != rhs.size()) {return false;}
			for (auto const & [key, value_rhs] : rhs) {
				auto iter = lhs.find(key);
				if (iter == lhs.end()) {return false;}
				auto value_lhs = iter->second;
				if(!equal(value_lhs, value_rhs)) {
					return false;
				}
			}
			return true;
		}

		template <typename T, typename V> requires EqualTraits::EqualCheckPossible<T,V>::value
		static bool equal(T const& lhs, V const& rhs) {
			return lhs == rhs;
		}

		template <EqualTraits::ProbablySet Set1, EqualTraits::ProbablySet Set2> requires (!EqualTraits::EqualCheckPossible<Set1, Set2>::value)
		static bool equal(Set1 const& lhs, Set2 const& rhs) {
			return set_equal(lhs, rhs);
		}

		template <EqualTraits::ProbablyMap Map1, EqualTraits::ProbablyMap Map2> requires (!EqualTraits::EqualCheckPossible<Map1, Map2>::value)
		static bool equal(Map1 const& lhs, Map2 const& rhs) {
			return map_equal(lhs, rhs);
		}

		template <size_t depth, HypertrieTrait htt_t>
		static bool equal(RawIdentifier<depth, htt_t> const& lhs, RawIdentifier<depth, htt_t> const& rhs) {
			return lhs.hash() == rhs.hash();
		}

		template <size_t depth, typename Type1, typename Type2>
		static bool equal(std::array<Type1, depth> const& lhs, std::array<Type2, depth> const& rhs) {
			for (size_t ix = 0; ix < depth; ++ix) {
				if (!equal(lhs[ix], rhs[ix])) {
					return false;
				}
			}

			return true;
		}

		template <size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type1, ByteAllocator allocator_type2>
		static void check_equal(FullNode<depth, htt_t, allocator_type1> const& lhs, FullNode<depth, htt_t, allocator_type2> const& rhs) {
			CHECK(lhs.identifier() == rhs.identifier());
			CHECK(lhs.size() == rhs.size());
			CHECK(lhs.ref_count() == rhs.ref_count());

			if constexpr (depth == 1) {
				CHECK(lhs.edges().size() == rhs.edges().size());

				if constexpr (HypertrieTrait_bool_valued<htt_t>) {
					for (auto const &key_part : lhs.edges()) {
						CHECK(rhs.edges().contains(key_part));
					}
				} else {
					for (auto const &[key_part, lvalue] : lhs.edges()) {
						auto rit = rhs.edges().find(key_part);
						CHECK(rit != rhs.edges().end());
						CHECK(lvalue == rit->second);
					}
				}
			} else {
				for (size_t pos = 0; pos < depth; ++pos) {
					CHECK(lhs.edges(pos).size() == rhs.edges(pos).size());
					for (auto const &[key_part, lptr] : lhs.edges(pos)) {
						auto rit = rhs.edges(pos).find(key_part);
						CHECK(rit != rhs.edges(pos).end());

						auto rptr = rit->second;
						CHECK(lptr.tag() == rptr.tag());

						if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
							if (lptr.is_sen()) {
								auto lkey_part = lptr.decode_key_part();
								auto rkey_part = rptr.decode_key_part();
								CHECK(lkey_part == rkey_part);
								return;
							}
						}

						lptr.visit_ptr(util::Overloaded{
								[&](FNPtr<depth - 1, htt_t, allocator_type1> lfn) noexcept {
									auto rfn = rptr.template specific_ptr<FullNode>();
									Equal::check_equal(*lfn, *rfn);
								},
								[&](SENPtr<depth - 1, htt_t, allocator_type1> lsen) noexcept  {
									auto rsen = rptr.template specific_ptr<SingleEntryNode>();
									Equal::check_equal(*lsen, *rsen);
								},
								[&](XNPtr<depth - 1, htt_t, allocator_type1> lxn) noexcept {
									if constexpr (depth - 1 == 1) {
										CHECK_FALSE("found depth 1 cartesian node");
									} else {
										auto rxn = rptr.template specific_ptr<CartesianNode>();
										Equal::check_equal(*lxn, *rxn);
									}
								}});
					}
				}
			}
		}

		template <size_t depth, HypertrieTrait htt_t>
		static void check_equal(SingleEntryNode<depth, htt_t> const& lhs, SingleEntryNode<depth, htt_t> const& rhs) {
			CHECK(lhs.key() == rhs.key());
			CHECK(lhs.value() == rhs.value());
		}

		template <size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type1, ByteAllocator allocator_type2>
		static void check_equal(CartesianNode<depth, htt_t, allocator_type1> const& lhs, CartesianNode<depth, htt_t, allocator_type2> const& rhs) {
			CHECK(lhs.identifier() == rhs.identifier());
			CHECK(lhs.size() == rhs.size());
			CHECK(lhs.ref_count() == rhs.ref_count());
			CHECK(lhs.discriminant() == rhs.discriminant());

			lhs.for_each_operand([&]<size_t ix, size_t operand_depth>(NodePtr<operand_depth, htt_t, allocator_type1> lptr) noexcept {
				auto rptr = static_cast<NodePtr<operand_depth, htt_t, allocator_type2>>(rhs.operand(ix));

				if constexpr (operand_depth > 0) {
					CHECK(lptr.tag() == rptr.tag());

					if constexpr (operand_depth == 1 && HypertrieTrait_taggable_key_part<htt_t>) {
						if (lptr.is_sen()) {
							auto lkey_part = lptr.decode_key_part();
							auto rkey_part = rptr.decode_key_part();
							CHECK(lkey_part == rkey_part);
							return;
						}
					}

					lptr.visit_ptr(util::Overloaded{
							[&](FNPtr<operand_depth, htt_t, allocator_type1> lfn) noexcept {
								auto rfn = rptr.template specific_ptr<FullNode>();
								Equal::check_equal(*lfn, *rfn);
							},
							[&](SENPtr<operand_depth, htt_t, allocator_type1> lsen) noexcept {
								auto rsen = rptr.template specific_ptr<SingleEntryNode>();
								Equal::check_equal(*lsen, *rsen);
							},
							[&](XNPtr<operand_depth, htt_t, allocator_type1>) noexcept {
								CHECK_FALSE("found cartesian child in cartesian");
							}});
				} else {
					CHECK(std::to_address(lptr.ptr()) == std::to_address(rptr.ptr()));
					CHECK(lptr.tag() == rptr.tag());
				}
			});
		}
	};

	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct ValidationRawNodeContext : public RawHypertrieContext<max_depth, htt_t, allocator_type> {
		using tri = htt_t;
		using key_part_type = typename htt_t::key_part_type;

		template<size_t depth>
		using SingleEntry_t = SingleEntry<depth, htt_t>;

		template<size_t depth>
		using RawIterator_t = RawIterator<depth, true, htt_t, allocator_type>;

	public:
		template<size_t depth>
		ValidationRawNodeContext(allocator_type const &alloc,
								 std::vector<SingleEntry_t<depth>> const &entries) : RawHypertrieContext<max_depth, htt_t, allocator_type>(alloc) {
			if (entries.empty()) {
				return;
			}

			if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				if (entries.size() == 1) {
					return;
				}
			}

			auto const ptr = init<depth>(entries);
			//dump_context(*this);
			validate(ptr, entries);
		}

		template<size_t depth>
		ValidationRawNodeContext(allocator_type const &alloc,
								 NodePtr<depth, htt_t, allocator_type> &node,
								 std::vector<SingleEntry_t<depth>> const &entries) noexcept : RawHypertrieContext<max_depth, htt_t, allocator_type>(alloc) {
			if (entries.empty()) {
				node = NodePtr<depth, htt_t, allocator_type>{};
				return;
			}

			if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				if (entries.size() == 1) {
					node = NodePtr<1, htt_t, allocator_type>::encode_key_part(entries[0].key()[0]);
					return;
				}
			}

			node = init<depth>(entries);
			validate(node, entries);
		}

		template<size_t depth>
		void validate(NodePtr<depth, htt_t, allocator_type> const &node,
					  std::vector<SingleEntry_t<depth>> const &entries) const {

			node_context::common_detail::Set<SingleEntry<depth, htt_t>> validation_entries; {
				for (RawIterator_t<depth> iter{node}; iter; ++iter) {
					validation_entries.emplace(*iter);
				}
			}

			CHECK(entries.size() == validation_entries.size());
			for (auto const &e : entries) {
				CHECK(validation_entries.contains(e));
			}
		}

		template<size_t depth>
		NodePtr<depth, htt_t, allocator_type> init(std::vector<SingleEntry_t<depth>> const &entries) noexcept {
			assert(entries.size() > 0);

			if (entries.size() == 1) {
				if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					CHECK_MESSAGE(false, "There must be no depth-1 SEN. They are stored in the identifier.");
				} else {
					RawIdentifier<depth, htt_t> const id{entries[0]};

					if (auto existing_sen = this->node_storage_.template lookup<depth, SingleEntryNode>(id); existing_sen) {
						existing_sen->ref_count() += 1;
						return existing_sen;
					}

					auto new_sen = this->node_storage_.template nodes<depth, SingleEntryNode>().node_lifecycle().new_(entries[0], 1UL);
					this->node_storage_.template nodes<depth, SingleEntryNode>().nodes().insert(new_sen);
					return new_sen;
				}
			}

			RawIdentifier<depth, htt_t> id{entries};

			if constexpr (depth > 1) {
				id = id.retag_as_xn();

				if (auto existing_xn = this->node_storage_.template lookup<depth, CartesianNode>(id); existing_xn) {
					existing_xn->ref_count() += 1;
					return existing_xn;
				}

				using node_context::common_detail::CartesianOperand;
				using node_context::common_detail::XFixCartesianProperties;
				using node_context::common_detail::inverse_cartesian_product;
				using node_context::common_detail::try_get_xfix_cartesian_properties;
				using node_context::common_detail::is_general_cartesian;

				auto operands = inverse_cartesian_product(entries);

				if constexpr (HypertrieTrait_bool_valued<htt_t>) {
					if (is_general_cartesian<htt_t>(operands, entries.size())) {
						auto new_xn = this->node_storage_.template nodes<depth, CartesianNode>().node_lifecycle().new_();
						new_xn->ref_count() = 1;
						assert(id.size() == entries.size());
						new_xn->size() = entries.size();
						new_xn->hash() = id.hash();
						new_xn->discriminant() = CartesianDiscriminant<depth>::for_general_cartesian();
						new_xn->for_each_operand([&]<size_t ix, size_t operand_depth>(NodePtr<operand_depth, htt_t, allocator_type> &operand) noexcept {
							if constexpr (operand_depth == 1) {
								if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
									if (operands[ix].size() == 1) {
										operand = NodePtr<1, htt_t, allocator_type>::encode_key_part(operands[ix][0].key()[0]);
										return;
									}
								}

								operand = init<1>(operands[ix]);
							}
						});

						auto [it, _] = this->node_storage_.template nodes<depth, CartesianNode>().nodes().insert(new_xn);
						return *it;
					}
				}

				if (auto const xfix_props = try_get_xfix_cartesian_properties(operands); xfix_props.has_value()) {
					CHECK(xfix_props->prefix_len + xfix_props->postfix_len < depth);
					CHECK((xfix_props->prefix_len > 0 || xfix_props->postfix_len > 0));

					return dice::template_library::switch_cases<0, depth>(xfix_props->prefix_len, [&](auto prefix_len) noexcept -> NodePtr<depth, htt_t, allocator_type> {
						return dice::template_library::switch_cases<0, depth>(xfix_props->postfix_len, [&](auto postfix_len) noexcept -> NodePtr<depth, htt_t, allocator_type> {
							if constexpr (prefix_len + postfix_len < depth) {
								auto new_xn = this->node_storage_.template nodes<depth, CartesianNode>().node_lifecycle().new_(id, 1UL);
								new_xn->size() = entries.size();

								if constexpr (prefix_len > 0) {
									std::array<CartesianOperand<1, htt_t>, prefix_len> prefix; {
										std::copy_n(operands.begin(), prefix_len, prefix.begin());
									}

									for (size_t ix = 0; ix < prefix_len; ++ix) {
										new_xn->discriminant().set(ix, 1);
										CHECK(prefix[ix].size() == 1);

										if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
											new_xn->operand(ix) = NodePtr<1, htt_t, allocator_type>::encode_key_part(prefix[ix][0].key()[0]);
										} else {
											new_xn->operand(ix) = init<1>(prefix[ix]);
										}
									}
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

								new_xn->discriminant().set(prefix_len, depth - prefix_len - postfix_len);
								new_xn->operand(prefix_len) = init<depth - prefix_len - postfix_len>(high_order_operand);

								if constexpr (postfix_len > 0) {
									std::array<CartesianOperand<1, htt_t>, postfix_len> postfix; {
										std::copy_n(operands.end() - postfix_len, postfix_len, postfix.begin());
									}

									for (size_t ix = 0; ix < postfix_len; ++ix) {
										auto const jx = ix + prefix_len + 1;

										new_xn->discriminant().set(jx, 1);
										CHECK(postfix[ix].size() == 1);

										if constexpr (HypertrieTrait_taggable_key_part<htt_t>) {
											new_xn->operand(jx) = NodePtr<1, htt_t, allocator_type>::encode_key_part(postfix[ix][0].key()[0]);
										} else {
											new_xn->operand(jx) = init<1>(postfix[ix]);
										}
									}
								}

								this->node_storage_.template nodes<depth, CartesianNode>().nodes().insert(new_xn);
								return new_xn;
							} else {
								CHECK_MESSAGE(false, "accidentally create xfix XN that does not fullfil xfix properties");
								HYPERTRIE_UNREACHABLE;
							}
						});
					});
				}
			}

			id = id.retag_as_fn();
			if (auto existing_fn = this->node_storage_.template lookup<depth, FullNode>(id); existing_fn) {
				existing_fn->ref_count() += 1;
				return existing_fn;
			}

			auto new_node = this->node_storage_.template nodes<depth, FullNode>().node_lifecycle().new_with_alloc(id, 1UL);//ref_count = 1

			// populate node
			if constexpr (depth == 1) {
				for (const auto &entry : entries) {
					if constexpr (HypertrieTrait_bool_valued<htt_t>) {
						new_node->edges().insert(entry.key()[0]);
					} else {
						new_node->edges().emplace(entry.key()[0], entry.value());
					}
				}
			} else {
				new_node->size() = entries.size();
				for (pos_type pos : iter::range(depth)) {
					node_context::common_detail::Map<key_part_type, std::vector<SingleEntry_t<depth - 1>>> childs_entries; {
						for (const auto &entry : entries) {
							childs_entries[entry.key()[pos]].emplace_back(entry.key().subkey(pos), entry.value());
						}
					}

					for (const auto &[key_part, child_entries] : childs_entries) {
						if constexpr (depth - 1 == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
							if (child_entries.size() == 1) {
								new_node->edges(pos)[key_part] = NodePtr<1, htt_t, allocator_type>::encode_key_part(child_entries[0].key()[0]);
								continue;
							}
						}
						// recursively populate subnode
						new_node->edges(pos)[key_part] = init<depth - 1>(child_entries);
					}
				}
			}

			auto [it, _] = this->node_storage_.template nodes<depth, FullNode>().nodes().insert(new_node);
			return *it;
		}

		template <ByteAllocator allocator_type2>
		bool operator==(RawHypertrieContext<max_depth, htt_t, allocator_type2> const &other) const {
			dice::template_library::for_range<1, max_depth>([this, &other](auto depth) {
				auto const &this_FNs = this->node_storage_.template nodes<depth, FullNode>().nodes();
				auto const &other_FNs = other.node_storage_.template nodes<depth, FullNode>().nodes();

				CHECK(this_FNs.size() == other_FNs.size());
				for (auto const &node : this_FNs) {
					CHECK(node->identifier().is_fn());
					CHECK(other_FNs.contains(node->identifier()));

					if (auto other_it = other_FNs.find(node->identifier()); other_it != other_FNs.end()) {
						auto const other_node = *other_it;
						Equal::check_equal(*node, *other_node);
					}
				}

				if constexpr (depth > 1) {
					auto const &this_XNs = this->node_storage_.template nodes<depth, CartesianNode>().nodes();
					auto const &other_XNs = other.node_storage_.template nodes<depth, CartesianNode>().nodes();
					CHECK(this_XNs.size() == other_XNs.size());

					for (auto const &node : this_XNs) {
						CHECK(other_XNs.contains(node->identifier()));
						CHECK(node->identifier().is_xn());

						if (auto other_it = other_XNs.find(node->identifier()); other_it != other_XNs.end()) {
							auto const other_node = *other_it;
							Equal::check_equal(*node, *other_node);
						}
					}
				}

				if constexpr (depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					auto const &this_SENs = this->node_storage_.template nodes<depth, SingleEntryNode>().nodes();
					auto const &other_SENs = other.node_storage_.template nodes<depth, SingleEntryNode>().nodes();
					CHECK(this_SENs.size() == other_SENs.size());

					for (const auto &node : this_SENs) {
						CHECK(other_SENs.contains(node->identifier()));
						CHECK(node->identifier().is_sen());
						if (auto other_it = other_SENs.find(node->identifier()); other_it != other_SENs.end()) {
							auto const other_node = *other_it;
							Equal::check_equal(*node, *other_node);
						}
					}
				}
			});
			return true;
		}
	};

}// namespace dice::hypertrie::tests::core::node

#endif//HYPERTRIE_VALIDATIONRAWNODECONTEXT_HPP
