#ifndef HYPERTRIE_TESTTENSOR_HPP
#define HYPERTRIE_TESTTENSOR_HPP
#include <Dice/hypertrie/internal/node_based/NodeContext.hpp>
#include <catch2/catch.hpp>
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

		TaggedNodeHash hash_;


	public:
		bool isPrimaryNode() const {
			return is_primary_node;
		}
		size_t ref_count() const {
			return ref_count_;
		}

	public:
		explicit TestTensor(bool primary = false, size_t ref_count = 0, std::map<RawKey<depth>, value_type> entries = {}) : entries(entries), is_primary_node(primary), ref_count_(ref_count) {
			hash_ = calcHash(this->entries, this->is_primary_node);
		}

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

		const std::map<RawKey<depth>, value_type> &getEntries() const {
			return entries;
		}

		void set(RawKey<depth> key, value_type value) {
			if (value != value_type{})
				entries[key] = value;
			else
				entries.erase(key);
			hash_ = calcHash(this->entries, this->is_primary_node);
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

			REQUIRE(storage.template getNodeStorage<depth, NodeCompression::uncompressed>().size() == 1);
			REQUIRE(storage.template getNodeStorage<depth, NodeCompression::compressed>().size() == 0);
			const UncompressedNode<depth, tri> *node = nc.node();
			INFO("ref count is not correct");
			REQUIRE(node->ref_count() == this->ref_count());

			std::map<TaggedNodeHash, std::shared_ptr<TestTensor<depth - 1, tri>>> existing_children{};
			auto children_by_pos = getChildrenNodes(existing_children);

			for (size_t pos : iter::range(depth)) {
				auto &actual_edges = node->edges(pos);
				auto &expected_edges = children_by_pos[pos];
				INFO("Wrong amount of children for depth {} and pos {}"_format(depth, pos));
				REQUIRE(actual_edges.size() == expected_edges.size());
				for (const auto &[key_part, child] : expected_edges) {
					INFO("key_part {} missing for depth {} and pos {}"_format(key_part, depth, pos));
					REQUIRE(actual_edges.count(key_part));
					INFO("Wrong hash for key_part {} for depth {} and pos {}"_format(key_part, depth, pos));
					INFO("actual_edges hash {}, expected hash {}"_format(actual_edges.at(key_part), child->hash()));
					REQUIRE(actual_edges.at(key_part) == child->hash());
				}
			}

			if constexpr (depth > 1)
				checkContext(context, existing_children);
		}

		template<size_t context_depth, size_t node_depth>
		void static checkContext(NodeContext<context_depth, tri> &context, std::map<TaggedNodeHash, std::shared_ptr<TestTensor<node_depth, tri>>> nodes) {
			auto & storage = context.storage;
			std::map<TaggedNodeHash, std::shared_ptr<TestTensor<node_depth - 1, tri>>> existing_children{};

			size_t uncompressed_count = 0;
			size_t compressed_count = 0;



			for (auto &[hash, test_node] : nodes) {
				assert(hash == test_node->hash());
				assert(not hash.empty());
				INFO(fmt::format("hash: {}", hash));
				if (hash.isUncompressed()) {
					uncompressed_count++;

					const UncompressedNodeContainer<node_depth, tri> &nc = storage.template getUncompressedNode<node_depth>(hash);

					INFO("Storage doesn't contain the entry");
					REQUIRE(not nc.null());

					INFO("Storage doesn't contain the entry");
					REQUIRE(not nc.empty());

					const UncompressedNode<node_depth, tri> *node = nc.node();

					INFO("ref count is not correct");
					REQUIRE(node->ref_count() == test_node->ref_count());

					INFO("size is not correct");
					REQUIRE(node->size() == test_node->size());
					auto children_by_pos= test_node->getChildrenNodes(existing_children);

					for (size_t pos : iter::range(node_depth)) {
						auto &actual_edges = node->edges(pos);
						auto &expected_edges = children_by_pos[pos];
						INFO("Wrong amount of children for depth {} and pos {}"_format(node_depth, pos));
						REQUIRE(actual_edges.size() == expected_edges.size());
						for (const auto &[key_part, child] : expected_edges) {
							INFO("key_part {} missing for depth {} and pos {}"_format(key_part, node_depth, pos));
							REQUIRE(actual_edges.count(key_part));
							if constexpr (node_depth == 1 and not tri::is_bool_valued) {
								INFO("Wrong hash for key_part {} for depth {} and pos {}"_format(key_part, node_depth, pos));
								INFO("actual_edges hash {}, expected hash {}"_format(actual_edges.at(key_part), child->hash()));
								REQUIRE(actual_edges.at(key_part) == child->getEntries().at({}));
							}
						}
					}
				} else {
					compressed_count++;

					const CompressedNodeContainer<node_depth, tri> &nc = storage.template getCompressedNode<node_depth>(hash);

					INFO("Storage doesn't contain the entry");
					REQUIRE(not nc.null());

					INFO("Storage doesn't contain the entry");
					REQUIRE(not nc.empty());

					const CompressedNode<node_depth, tri> *node = nc.node();

					INFO("ref count is not correct");
					REQUIRE(node->ref_count() == test_node->ref_count());

					INFO("size is not correct");
					REQUIRE(node->size() == test_node->size());

					INFO("Key is not correct");
					REQUIRE(test_node->getEntries().count(node->key()));
					INFO("Value is not correct");
					REQUIRE(test_node->getEntries().at(node->key()) == node->value());
				}
			}

			REQUIRE(storage.template getNodeStorage<node_depth, NodeCompression::uncompressed>().size() == uncompressed_count);
			REQUIRE(storage.template getNodeStorage<node_depth, NodeCompression::compressed>().size() == compressed_count);

			if constexpr (node_depth > 1)
				checkContext(context, existing_children);
		}

		auto getChildrenNodes(std::map<TaggedNodeHash, std::shared_ptr<TestTensor<depth - 1, tri>>> &existing_children) {
			std::array<std::map<key_part_type, std::map<RawKey<depth - 1>, value_type>>, depth> children_entries_by_pos{};
			for (size_t pos : iter::range(depth))
				for (const auto &[key, value] : entries)
					children_entries_by_pos[pos][key[pos]].insert({subkey(key, pos), value});


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
			return  this->hash_;
		}

		size_t size() const {
			return this->entries.size();
		}

		template<size_t key_depth>
		static TaggedNodeHash calcHash(std::map<RawKey<key_depth>, value_type> entries, bool is_primary_node = false) {
			TaggedNodeHash hash = TaggedNodeHash::getUncompressedEmptyNodeHash<depth>();
			bool first = true;
			for (const auto &[key, value] : entries) {
				if (not is_primary_node and first) {
					first = false;
					hash = TaggedNodeHash::getCompressedNodeHash(key, value);
				} else
					hash.addEntry(key, value);
			}
			return hash;
		}
	};

}// namespace hypertrie::tests::node_based::node_context

#endif//HYPERTRIE_TESTTENSOR_HPP
