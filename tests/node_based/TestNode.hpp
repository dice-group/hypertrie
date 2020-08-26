#ifndef HYPERTRIE_TESTNODE_HPP
#define HYPERTRIE_TESTNODE_HPP

#include <boost/type_index.hpp>
#include <enumerate.hpp>
#include <fmt/format.h>

#include <Dice/hypertrie/internal/raw/node/Node.hpp>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"

namespace hypertrie::tests::node_based::raw::node {

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::node_based::raw;

	template<size_t depth, typename key_part_type>
	using Key = hypertrie::internal::RawKey<depth, key_part_type>;

	template<HypertrieInternalTrait tr, pos_type depth>
	void createCompressedNode() {
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = Key<depth, key_part_type>;

		SECTION(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
							depth, nameOfType<key_part_type>(), nameOfType<value_type>())) {
			utils::RawGenerator<depth, key_part_type, value_type> gen{};

			Key key = gen.key();

			// unused for bool
			value_type value = gen.value();

			CompressedNode<depth, tr> node = [&]() {
				if constexpr (tr::is_bool_valued) return CompressedNode<depth, tr>{key};
				else
					return CompressedNode<depth, tr>{key, value};
			}();

			REQUIRE(node.key() == key);
			if constexpr (not std::is_same_v<value_type, bool>)
				REQUIRE(node.value() == value);
			REQUIRE(node.size() == 1);
		}
	}

	TEST_CASE("create compressed node", "[Node]") {
		createCompressedNode<default_bool_Hypertrie_internal_t, 1>();
		createCompressedNode<default_long_Hypertrie_internal_t, 1>();
		createCompressedNode<default_double_Hypertrie_internal_t, 1>();
		createCompressedNode<default_bool_Hypertrie_internal_t, 2>();
		createCompressedNode<default_long_Hypertrie_internal_t, 2>();
		createCompressedNode<default_double_Hypertrie_internal_t, 2>();
		createCompressedNode<default_bool_Hypertrie_internal_t, 3>();
		createCompressedNode<default_long_Hypertrie_internal_t, 3>();
		createCompressedNode<default_double_Hypertrie_internal_t, 3>();
	}

	template<HypertrieInternalTrait tr, pos_type depth>
	void createEmptyUncompressed() {
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		SECTION(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
							depth, nameOfType<key_part_type>(), nameOfType<value_type>())) {

			UncompressedNode<depth, tr> node{};

			for (size_t pos : iter::range(depth))
				REQUIRE(node.edges(pos).size() == 0);
			REQUIRE(node.size() == 0);
		}
	}

	TEST_CASE("create empty uncompressed node", "[Node]") {
		createEmptyUncompressed<default_bool_Hypertrie_internal_t, 1>();
		createEmptyUncompressed<default_long_Hypertrie_internal_t, 1>();
		createEmptyUncompressed<default_double_Hypertrie_internal_t, 1>();
		createEmptyUncompressed<default_bool_Hypertrie_internal_t, 2>();
		createEmptyUncompressed<default_long_Hypertrie_internal_t, 2>();
		createEmptyUncompressed<default_double_Hypertrie_internal_t, 2>();
		createEmptyUncompressed<default_bool_Hypertrie_internal_t, 3>();
		createEmptyUncompressed<default_long_Hypertrie_internal_t, 3>();
		createEmptyUncompressed<default_double_Hypertrie_internal_t, 3>();
	}

};// namespace hypertrie::tests::node_based::node

#endif//HYPERTRIE_TESTNODE_HPP
