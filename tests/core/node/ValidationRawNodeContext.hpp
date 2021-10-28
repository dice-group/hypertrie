#ifndef HYPERTRIE_VALIDATIONRAWNODECONTEXT_HPP
#define HYPERTRIE_VALIDATIONRAWNODECONTEXT_HPP
#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <fmt/format.h>
#include <itertools.hpp>
#include <tsl/sparse_map.h>

#include <Dice/hash/DiceHash.hpp>
#include <Dice/hypertrie/internal/commons/PosType.hpp>

#include <boost/hana/for_each.hpp>
#include <boost/hana/range.hpp>

namespace hypertrie::tests::core::node {
	using namespace fmt::literals;

	using namespace ::hypertrie::internal::raw;
	using namespace ::hypertrie::internal;

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


		template <size_t depth, HypertrieCoreTrait tri_t1, HypertrieCoreTrait tri_t2>
		static bool equal(RawIdentifier<depth, tri_t1> const& lhs, RawIdentifier<depth, tri_t2> const& rhs) {
			return lhs.hash() == rhs.hash();
		}

		template <size_t depth, typename Type1, typename Type2>
		static bool equal(std::array<Type1, depth> const& lhs, std::array<Type2, depth> const& rhs) {
			bool result = true;
			boost::hana::for_each(boost::hana::range_c<size_t, 0, depth>, [&lhs, &rhs, &result](size_t index){
				if(!equal(lhs[index], rhs[index])){
					result = false;
				}
			});
			return result;
		}

		template <size_t depth, HypertrieCoreTrait tri_t1, HypertrieCoreTrait tri_t2>
		static bool equal(FullNode<depth, tri_t1> const& lhs, FullNode<depth, tri_t2> const& rhs) {
			return equal(lhs.edges(), rhs.edges());
		}

		template <size_t depth, HypertrieCoreTrait tri_t1, HypertrieCoreTrait tri_t2>
		static bool equal(SingleEntryNode<depth, tri_t1> const& lhs, SingleEntryNode<depth, tri_t2> const& rhs) {
			return lhs.key() == rhs.key() && lhs.value() == rhs.value();
		}
	};

	template<size_t max_depth, HypertrieCoreTrait tri_t>
	struct ValidationRawNodeContext : public RawHypertrieContext<max_depth, tri_t> {
		using tri = tri_t;
		template<size_t depth>
		using EntriesType = std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>>;
		using key_part_type = typename tri::key_part_type;

	public:
		template<size_t depth>
		ValidationRawNodeContext(const typename tri::allocator_type &alloc,
								 EntriesType<depth> const &entries) noexcept : RawHypertrieContext<max_depth, tri>(alloc) {
			if constexpr (depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>)
				if (entries.size() == 1)
					return;
			if (not entries.empty())
				init<depth>(entries);
		}

		template<size_t depth>
		void init(EntriesType<depth> const &entries) noexcept {
			RawIdentifier<depth, tri> id{entries};
			if (entries.size() == 1) {// SEN
				if constexpr (depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>)
					CHECK_MESSAGE(false, "There must be no depth-1 SEN. They are stored in the identifier.");
				else {
					SingleEntryNode<depth, tri_with_stl_alloc<tri>> new_node{entries[0], 1};

					auto existing_node = this->node_storage_.template lookup<depth, SingleEntryNode>(id);
					if (existing_node) {
						REQUIRE(SingleEntryNode<depth, tri_with_stl_alloc<tri>>{*existing_node} == new_node);
						existing_node->ref_count() += 1;
					} else {
						this->node_storage_.template nodes<depth, SingleEntryNode>().nodes().insert(
								{id,
								 this->node_storage_.template nodes<depth, SingleEntryNode>().node_lifecycle().new_(new_node)});
					}
				}
			} else {

				auto existing_node = this->node_storage_.template lookup<depth, FullNode>(id);
				if (existing_node) {
					// CHECK(*new_node == *existing_node); // TODO: validate, that the nodes are actually equal (checking for cache collisions
					existing_node->ref_count() += 1;
				} else {
					// FN
					auto new_node = this->node_storage_.template nodes<depth, FullNode>().node_lifecycle().new_with_alloc(1);//ref_count = 1

					// populate node
					if constexpr (depth == 1) {
						for (const auto &entry : entries)
							new_node->insert_or_assign(entry.key(), entry.value());
					} else {
						new_node->size() = entries.size();
						for (pos_type pos : iter::range(depth)) {
							tsl::sparse_map<key_part_type, EntriesType<depth - 1>> childs_entries;
							for (const auto &entry : entries)
								childs_entries[entry.key()[pos]].emplace_back(entry.key().subkey(pos), entry.value());

							for (const auto &[key_part, child_entries] : childs_entries) {
								RawIdentifier<depth - 1, tri> child_id{child_entries};
								new_node->edges(pos)[key_part] = child_id;
								if constexpr (depth > 1)
									if constexpr (depth == 2 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>)
										if (child_entries.size() == 1)
											continue;
								// recursively populate subnode
								init<depth - 1>(child_entries);
							}
						}
					}

					this->node_storage_.template nodes<depth, FullNode>().nodes()[id] = new_node;
					auto x = this->node_storage_.template nodes<depth, FullNode>().nodes().find(id);
					CHECK(x != this->node_storage_.template nodes<depth, FullNode>().nodes().end());
				}
			}
		}

		template <HypertrieCoreTrait tri_t2>
		bool operator==(RawHypertrieContext<max_depth, tri_t2> const &other) const {
			namespace hana = boost::hana;

			hana::for_each(hana::range_c<size_t, 1, max_depth>, [this, &other](auto depth) {
				const auto &this_FNs = this->node_storage_.template nodes<depth, FullNode>().nodes();
				const auto &other_FNs = other.node_storage_.template nodes<depth, FullNode>().nodes();

				CHECK(this_FNs.size() == other_FNs.size());
				/* Usage of different allocators create different types even if the types are bitwise equal.
			     * converter simply swaps the allocator type to the one needed.
			     * DOES NOT WORK FOR ALL TYPES!
			    */
				auto ident_converter = [depth](RawIdentifier<depth, tri_t> const& rawIdent) {
					return reinterpret_cast<RawIdentifier<depth, tri_t2> const&>(rawIdent);
				};

				for (const auto &[raw_id, node] : this_FNs) {
					auto id = ident_converter(raw_id);
					CHECK(other_FNs.contains(id));
					if (other_FNs.contains(id)) {
						auto other_node = other_FNs.find(id).value();
						CHECK(Equal::equal(*node, *other_node));
						CHECK(node->ref_count() == other_node->ref_count());
					}
				}


				if constexpr (not(depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>)) {
					const auto &this_SENs = this->node_storage_.template nodes<depth, SingleEntryNode>().nodes();
					const auto &other_SENs = other.node_storage_.template nodes<depth, SingleEntryNode>().nodes();
					CHECK(this_SENs.size() == other_SENs.size());

					for (const auto &[raw_id, node] : this_SENs) {
						auto id = ident_converter(raw_id);
						CHECK(other_SENs.contains(id));
						if (other_SENs.contains(id)) {
							auto other_node = other_SENs.find(id).value();
							CHECK(Equal::equal(*node, *other_node));
							CHECK(node->ref_count() == other_node->ref_count());
						}
					}
				}
			});
			return true;
		}
	};

}// namespace hypertrie::tests::core::node

#endif//HYPERTRIE_VALIDATIONRAWNODECONTEXT_HPP
