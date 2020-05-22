#ifndef HYPERTRIE_TESTNODE_HPP
#define HYPERTRIE_TESTNODE_HPP

#include <Dice/hypertrie/internal/node_based/Node.hpp>

#include "../utils/AssetGenerator.hpp"

namespace hypertrie::tests::node_based::node {

	using namespace hypertrie::internal::node_based;

	template<size_t depth, typename key_part_type>
	using Key = hypertrie::internal::RawKey<depth, key_part_type>;

	template<HypertrieInternalTrait tr, pos_type depth>
	void createCompressedNode() {
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = Key<depth, key_part_type>;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		Key key = gen.key();

		// unused for bool
		value_type value = gen.value();

		Node<depth, true, tr> node = [&]() {
			if constexpr (tr::is_bool_valued) return Node<depth, true, tr>{key};
			else return Node<depth, true, tr>{key, value};
		}();

		REQUIRE(node.key() == key);
		if constexpr (not std::is_same_v<value_type, bool>)
			REQUIRE(node.value() == value);
	}

	TEST_CASE("create compressed node", "[Node]") {
		for ([[maybe_unused]] auto _ : iter::range(50)) {
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
	}

	template<HypertrieInternalTrait tr, pos_type depth>
	void createEmptyUncompressed() {

		Node<depth, false, tr> node{};

		for (size_t pos : iter::range(depth))
			REQUIRE(node.edges(pos).size() == 0);
		REQUIRE(node.size() == 0);
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

	template<HypertrieInternalTrait tr, pos_type depth>
	void fillEmptyUncompressed() {
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = Key<depth, key_part_type>;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		// TODO: implement
	}


	TEST_CASE("fill empty uncompressed node", "[Node]") {
		fillEmptyUncompressed<default_bool_Hypertrie_internal_t, 1>();
		fillEmptyUncompressed<default_long_Hypertrie_internal_t, 1>();
		fillEmptyUncompressed<default_double_Hypertrie_internal_t, 1>();
		fillEmptyUncompressed<default_bool_Hypertrie_internal_t, 2>();
		fillEmptyUncompressed<default_long_Hypertrie_internal_t, 2>();
		fillEmptyUncompressed<default_double_Hypertrie_internal_t, 2>();
		fillEmptyUncompressed<default_bool_Hypertrie_internal_t, 3>();
		fillEmptyUncompressed<default_long_Hypertrie_internal_t, 3>();
		fillEmptyUncompressed<default_double_Hypertrie_internal_t, 3>();
	}

};

#endif //HYPERTRIE_TESTNODE_HPP
