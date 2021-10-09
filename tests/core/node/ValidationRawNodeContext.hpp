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
			Identifier<depth, tri> id{entries};
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
								Identifier<depth - 1, tri> child_id{child_entries};
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

		bool operator==(RawHypertrieContext<max_depth, tri_t> const &other) const {
			namespace hana = boost::hana;

			hana::for_each(hana::range_c<size_t, 1, max_depth>, [this, &other](auto depth) {
				const auto &this_FNs = this->node_storage_.template nodes<depth, FullNode>().nodes();
				const auto &other_FNs = other.node_storage_.template nodes<depth, FullNode>().nodes();

				CHECK(this_FNs.size() == other_FNs.size());
				for (const auto &[id, node] : this_FNs) {
					CHECK(other_FNs.contains(id));
					if (other_FNs.contains(id)) {
						auto other_node = other_FNs.find(id).value();
						CHECK(*node == *other_node);
						CHECK(node->ref_count() == other_node->ref_count());
					}
				}


				if constexpr (not(depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>)) {
					const auto &this_SENs = this->node_storage_.template nodes<depth, SingleEntryNode>().nodes();
					const auto &other_SENs = other.node_storage_.template nodes<depth, SingleEntryNode>().nodes();
					CHECK(this_SENs.size() == other_SENs.size());

					for (const auto &[id, node] : this_SENs) {
						CHECK(other_SENs.contains(id));
						if (other_SENs.contains(id)) {
							auto other_node = other_SENs.find(id).value();
							CHECK(*node == *other_node);
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
