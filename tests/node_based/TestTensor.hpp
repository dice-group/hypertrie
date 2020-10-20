#ifndef HYPERTRIE_TESTTENSOR_HPP
#define HYPERTRIE_TESTTENSOR_HPP
#include <Dice/hypertrie/internal/raw/storage/NodeContext.hpp>
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <tsl/sparse_map.h>
#include <cppitertools/itertools.hpp>

namespace hypertrie::tests::raw::node_context {


	using namespace fmt::literals;

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::raw;

	template<size_t depth, HypertrieInternalTrait tri_t, typename = std::enable_if_t<(depth >= 0)>>
	class TestTensor {
		using tri = tri_t;

	public:
		template <size_t node_depth>
		using NodeRepr_t = std::conditional_t<(not (node_depth == 1 and tri_t::is_bool_valued and tri_t::is_lsb_unused)), TensorHash, TaggedTensorHash<tri_t>>;
		using NodeRepr = NodeRepr_t<depth>;
		template <size_t node_depth>
		using Hash2Instance_t = tsl::sparse_map<NodeRepr_t<node_depth>, std::shared_ptr<TestTensor<node_depth, tri>>>;
		using Hash2Instance = Hash2Instance_t<depth>;


	private:
		template<size_t key_depth>
		using RawKey = typename tri::template RawKey<key_depth>;
		using value_type = typename tri::value_type;
		using key_part_type = typename tri::key_part_type;

		tsl::sparse_map<RawKey<depth>, value_type, absl::Hash<RawKey<depth>>> entries{};

		size_t ref_count_;

		NodeRepr hash_;


	public:
		size_t ref_count() const {
			return ref_count_;
		}

	public:
		explicit TestTensor(size_t ref_count = 0, tsl::sparse_map<RawKey<depth>, value_type, absl::Hash<RawKey<depth>>> entries = {}) : entries(entries), ref_count_(ref_count) {
			hash_ = this->calcHash(this->entries);
		}

		static auto getPrimary() {
			return TestTensor(1);
		}

		template<size_t key_depth>
		static auto subkey(const RawKey<key_depth> &key, size_t remove_pos) -> RawKey<key_depth - 1> {
			RawKey<key_depth - 1> sub_key;
			for (size_t i = 0, j = 0; i < key_depth; ++i)
				if (i != remove_pos) sub_key[j++] = key[i];
			return sub_key;
		}

		const tsl::sparse_map<RawKey<depth>, value_type, absl::Hash<RawKey<depth>>> &getEntries() const {
			return entries;
		}

		void set(RawKey<depth> key, value_type value) {
			if (value != value_type{})
				entries[key] = value;
			else
				entries.erase(key);
			hash_ = calcHash(this->entries);
		}

		void incRefCount() {
			ref_count_ += 1;
		}

		template<size_t context_depth>
		void checkContext(NodeContext<context_depth, tri> &context) {
			if (entries.empty()) {
				validateEmpty<context_depth,depth>(context);
			} else {
				typename TestTensor<depth, tri>::Hash2Instance nodes{};
				nodes[this->hash()] = std::make_shared<TestTensor<depth, tri>>(*this);
				 checkContext<context_depth, depth>(context, nodes);
			}
		}

		template<size_t context_depth, size_t node_depth>
		void static validateEmpty(NodeContext<context_depth, tri> &context) {
			auto & storage = context.storage;
			REQUIRE(storage.template getNodeStorage<node_depth, NodeCompression::uncompressed>().empty());
			if constexpr (not (node_depth == 1 and tri::is_bool_valued and tri::is_lsb_unused))
				REQUIRE(storage.template getNodeStorage<node_depth, NodeCompression::compressed>().empty());
			if constexpr (node_depth > 1)
				validateEmpty<context_depth,node_depth -1>(context);
		}

		template<size_t context_depth, size_t node_depth>
		void static checkContext(NodeContext<context_depth, tri> &context, Hash2Instance_t<node_depth> nodes) {
			static_assert(depth > 0);
			auto & storage = context.storage;

			// count hwo much uncompressed and compressed children are there
			size_t uncompressed_count = 0;
			size_t compressed_count = 0;

			// check if the nodes passed via argument are fine and in context cext
			for (auto &[hash, test_node] : nodes) {
				REQUIRE(hash == test_node->hash());
				REQUIRE(not hash.empty());
				INFO(fmt::format("hash: {}", hash));
				if (hash.isUncompressed()) {
					uncompressed_count++;

					const UncompressedNodeContainer<node_depth, tri> &nc = storage.template getUncompressedNode<node_depth>((TensorHash)hash);

					INFO("Storage doesn't contain the entry");
					REQUIRE(not nc.null());

					INFO("Storage doesn't contain the entry");
					REQUIRE(not nc.empty());

					const UncompressedNode<node_depth, tri> *node = nc.uncompressed_node();

					INFO("ref count is not correct");
					REQUIRE(node->ref_count() == test_node->ref_count());

					INFO("size is not correct");
					REQUIRE(node->size() == test_node->size());

					if constexpr (node_depth > 1)
						for (size_t pos : iter::range(node_depth)) {
							auto &actual_edges = node->edges(pos);
							const auto &expected_edges = test_node->getEdgesByPos(pos);
							INFO("Wrong amount of children for depth {} and pos {}"_format(node_depth, pos));
							REQUIRE(actual_edges.size() == expected_edges.size());
							for (const auto &[expected_key_part, expected_child_hash] : expected_edges) {
								INFO("key_part {} missing for depth {} and pos {}"_format(expected_key_part, node_depth, pos));
								REQUIRE(actual_edges.count(expected_key_part));
								if constexpr (not (node_depth == 1 and tri::is_bool_valued)) {
									INFO("child_hash_or_value {}  is wrong for depth {} and pos {}"_format(expected_child_hash, node_depth, pos));
									REQUIRE(actual_edges.at(expected_key_part) == expected_child_hash);
								}
							}
						}
					else {
						auto &actual_edges = node->edges(0);
						INFO("Wrong amount of children for depth {}"_format(node_depth));
						REQUIRE(actual_edges.size() == test_node->getEntries().size());
						for (const auto &[expected_key, expected_value] : test_node->getEntries()) {
							auto expected_key_part = expected_key[0];
							INFO("key_part {} missing for depth {}"_format(expected_key_part, node_depth));
							REQUIRE(actual_edges.count(expected_key_part));
							INFO("child_value {}  is wrong for depth {}"_format(expected_value, node_depth));
							if constexpr (not tri::is_bool_valued)
							REQUIRE(actual_edges.at(expected_key_part) == expected_value);
						}
					}
				} else {
					if constexpr (not (node_depth == 1 and tri::is_bool_valued and tri::is_lsb_unused)) {
						compressed_count++;
						const CompressedNodeContainer<node_depth, tri> &nc = storage.template getCompressedNode<node_depth>(hash);

						INFO("Storage doesn't contain the entry");
						REQUIRE(not nc.null());

						INFO("Storage doesn't contain the entry");
						REQUIRE(not nc.empty());

						const CompressedNode<node_depth, tri> *node = nc.compressed_node();

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
			}
			REQUIRE(storage.template getNodeStorage<node_depth, NodeCompression::uncompressed>().size() == uncompressed_count);
			if constexpr (not (node_depth == 1 and tri::is_bool_valued and tri::is_lsb_unused))
				REQUIRE(storage.template getNodeStorage<node_depth, NodeCompression::compressed>().size() == compressed_count);



			// calculate sub-nodes
			if constexpr (node_depth > 1) {

				Hash2Instance_t<node_depth -1> existing_children{};

				for (auto &[hash, test_node] : nodes)
					if (hash.isUncompressed())
						for (size_t pos : iter::range(node_depth))
							test_node->populateExistingChildren(pos, existing_children);

				// check the sub-nodes
				checkContext<context_depth, node_depth -1>(context, existing_children);
			}
		}

		auto getSubEntriesByPos(size_t pos) {
			if constexpr(depth > 1){
				tsl::sparse_map<key_part_type,
						tsl::sparse_map<RawKey<depth - 1>, value_type, absl::Hash<RawKey<depth -1>>>
				> mapped_entries{};

				// find all entries of sub-tensors for that position
				for (const auto &[key, value] : this->entries)
					mapped_entries[key[pos]].insert({subkey(key, pos), value});

				return mapped_entries;
			}
		}

		auto getEdgesByPos(size_t pos) {
			if constexpr(depth > 1) {
				auto sub_entries = getSubEntriesByPos(pos);

				// hash them
				tsl::sparse_map<key_part_type, NodeRepr_t<depth -1>> edges{};
				for (const auto &[key_part, child_entries] : sub_entries)
					edges[key_part] = this->calcHash<depth - 1>(child_entries);

				return edges;
			}
		}

		void populateExistingChildren(size_t pos, Hash2Instance_t<depth -1> &existing_children){
			if constexpr (depth > 1){
			auto sub_entries = getSubEntriesByPos(pos);

			for (const auto &[key_part, child_entries] : sub_entries) {
				if constexpr (depth == 2 and tri::is_bool_valued and tri::is_lsb_unused)
					if (child_entries.size() == 1)
						continue;
				auto child_hash = calcHash<depth -1>(child_entries);
				if (not existing_children.contains(child_hash))
					existing_children[child_hash] = std::make_shared<TestTensor<depth - 1, tri>>(0, child_entries); //<TestTensor<depth - 1, tri>>
				auto child = existing_children[child_hash];
				child->incRefCount();
			}
			}
		}

		NodeRepr
		hash() const {
			return  this->hash_;
		}

		size_t size() const {
			return this->entries.size();
		}

		template<size_t key_depth>
		static NodeRepr_t<key_depth> calcHash(tsl::sparse_map<RawKey<key_depth>, value_type, absl::Hash<RawKey<key_depth>>> entries) {
			if constexpr (tri::is_bool_valued and tri::is_lsb_unused and key_depth == 1) {
				if (entries.size() == 1) {
					return {entries.begin()->first[0]};
				}
			}

			TensorHash hash{};
			bool first = true;
			for (const auto &[key, value] : entries) {
				if (first) {
					first = false;
					hash = TensorHash::getCompressedNodeHash(key, value);
				} else
					hash.addEntry(key, value);
			}
			return {hash};
		}
	};

}// namespace hypertrie::tests::node_context

#endif//HYPERTRIE_TESTTENSOR_HPP
