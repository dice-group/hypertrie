#ifndef HYPERTRIE_TESTNODE_HPP
#define HYPERTRIE_TESTNODE_HPP

#include <Dice/hypertrie/internal/node_based/Node.hpp>

namespace hypertrie::tests::node_based::node {

	using namespace hypertrie::internal::node_based;

	template<size_t depth, typename key_part_type>
	using Key = hypertrie::internal::RawKey<depth, key_part_type>;

	template<HypertrieInternalTrait tr>
	void createCompressedNode() {
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		const pos_type depth = 1;
		using Key = Key<depth, key_part_type>;

		Key key{3};

		value_type value;
		if constexpr (not std::is_same_v<value_type, bool>)
			value = [&]() {
				if constexpr(std::is_same_v<value_type, long>) return 3;
				else return 3.6; //double
			}();

		Node<1, true, tr> node = [&]() {
			if constexpr (std::is_same_v<value_type, bool>) return Node<1, true, tr>{key};
			else return Node<1, true, tr>{key, value};
		}();

		REQUIRE(node.key_ == key);
		if constexpr (not std::is_same_v<value_type, bool>)
			REQUIRE(node.value_ == value);
	}

	TEST_CASE("create compressed node", "[Node]") {
		createCompressedNode<default_bool_Hypertrie_internal_t>();
		createCompressedNode<default_double_Hypertrie_internal_t>();
	}

};

#endif //HYPERTRIE_TESTNODE_HPP
