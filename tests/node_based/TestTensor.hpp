#ifndef HYPERTRIE_TESTTENSOR_HPP
#define HYPERTRIE_TESTTENSOR_HPP
#include <Dice/hypertrie/internal/node_based/NodeContext.hpp>
#include <fmt/format.h>
#include <itertools.hpp>

namespace hypertrie::tests::node_based::node_context {


	using namespace fmt::literals;

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::node_based;

	template<size_t depth, HypertrieInternalTrait tri_t>
	class TestTensor {
		using tri = tri_t;

		template<size_t key_depth>
		using RawKey = typename tri::template RawKey<key_depth>;
		using value_type = typename tri::value_type;
		using key_part_type = typename tri::key_part_type;

		std::map<RawKey<depth>, value_type> entries{};

		bool is_primary_node;

		size_t ref_count_;


	public:
		bool isPrimaryNode() const {
			return is_primary_node;
		}
		size_t ref_count() const {
			return ref_count_;
		}

	public:
		explicit TestTensor(bool primary = false, size_t ref_count = 0, std::map<RawKey<depth>, value_type> entries = {}) : entries(entries), is_primary_node(primary), ref_count_(ref_count) {}

		static auto getPrimary() {
			return TestTensor(true, 1);
		}

		template<size_t key_depth>
		static auto subkey(const RawKey<key_depth> &key, size_t remove_pos) -> RawKey<key_depth - 1> {
			RawKey<key_depth - 1> sub_key;
			for (size_t i = 0, j = 0; i < key_depth; ++i)
				if (i != remove_pos) sub_key[j++] = key[i];
			return sub_key;
		}

		void set(RawKey<depth> key, value_type value) {
			if (value != value_type{})
				entries[key] = value;
			else
				entries.erase(key);
		}

		void incRefCount() {
			ref_count_ += 1;
		}

		template<size_t context_depth>
		void checkContext(NodeContext<context_depth, tri> &context) {
			TaggedNodeHash hash = this->hash();
			auto & storage = context.storage;
			const UncompressedNodeContainer<depth, tri> &nc = storage.template getUncompressedNode<depth>(hash);
			INFO("Storage doesn't contain the entry");
			REQUIRE(not nc.null());
			INFO("Storage doesn't contain the entry");
			REQUIRE(not nc.empty());

			REQUIRE(storage.template getNodeStorage<context_depth, NodeCompression::uncompressed>().size() == 1);
			REQUIRE(storage.template getNodeStorage<context_depth, NodeCompression::compressed>().size() == 0);
			const UncompressedNode<depth, tri> *node = nc.node();
			INFO("ref count is not correct");
			REQUIRE(node->ref_count() == this->ref_count());
			std::array<std::map<key_part_type, std::shared_ptr<TestTensor<depth - 1, tri>>>, depth> children_by_pos = getChildrenNodes();

			for (size_t pos : iter::range(depth)) {
				auto &actual_edges = node->edges(pos);
				auto &expected_edges = children_by_pos[pos];
				INFO("Wrong amount of children for depth {} and pos {}"_format(depth, pos));
				REQUIRE(actual_edges.size() == expected_edges.size());
				for (const auto &[key_part, child] : expected_edges) {
					INFO("key_part {} missing for depth {} and pos {}"_format(key_part, depth, pos));
					REQUIRE(actual_edges.count(key_part));
					INFO("Wrong hash for key_part {} missing for depth {} and pos {}"_format(key_part, depth, pos));
					REQUIRE(actual_edges.at(key_part) == child->hash());
				}
			}
		}

		auto getChildrenNodes() {
			std::array<std::map<key_part_type, std::map<RawKey<depth - 1>, value_type>>, depth> children_entries_by_pos{};
			for (size_t pos : iter::range(depth))
				for (const auto &[key, value] : entries)
					children_entries_by_pos[pos][key[pos]].insert({subkey(key, pos), value});


			std::map<TaggedNodeHash, std::shared_ptr<TestTensor<depth - 1, tri>>> existing_children{};
			std::array<std::map<key_part_type, std::shared_ptr<TestTensor<depth - 1, tri>>>, depth> children_by_pos{};

			for (size_t pos : iter::range(depth))
				for (const auto &[key_part, child_entries] : children_entries_by_pos[pos]) {
					TaggedNodeHash child_hash = calcHash(child_entries);
					if (not existing_children.contains(child_hash))
						existing_children[child_hash] = std::make_shared<TestTensor<depth - 1, tri>>(false, 0, child_entries);
					std::shared_ptr<TestTensor<depth - 1, tri>> child = existing_children[child_hash];
					child->incRefCount();
					children_by_pos[pos][key_part] = child;
				}
			return children_by_pos;
		}

		TaggedNodeHash
		hash() const {
			return calcHash(this->entries, this->is_primary_node);
		}

		template<size_t key_depth>
		static TaggedNodeHash calcHash(std::map<RawKey<key_depth>, value_type> entries, bool is_primary_node = false) {
			TaggedNodeHash hash = (is_primary_node) ? TaggedNodeHash::getUncompressedEmptyNodeHash<depth>() : TaggedNodeHash::getCompressedEmptyNodeHash<depth>();
			bool first = true;
			for (const auto &[key, value] : entries) {
				if (not is_primary_node and first) {
					first = false;
					hash.addFirstEntry(key, value);
				} else
					hash.addEntry(key, value);
			}
			return hash;
		}
	};

}// namespace hypertrie::tests::node_based::node_context

#endif//HYPERTRIE_TESTTENSOR_HPP
