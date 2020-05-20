#ifndef HYPERTRIE_TESTTAGGEDNODEHASH_HPP
#define HYPERTRIE_TESTTAGGEDNODEHASH_HPP

#include <Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp>

#include <Dice/hypertrie/internal/util/RawKey.hpp>

namespace hypertrie::tests::node_based::tagged_node_hash {
	using namespace hypertrie::internal::node_based;

	using key_part_type = float;
	template<pos_type depth>
	using Key = hypertrie::internal::RawKey<depth, key_part_type>;

	TEST_CASE("default construct", "[TaggedNodeHash]") {
		REQUIRE(TaggedNodeHash().hash() == size_t(0));
	}

	TEST_CASE("construct hash for empty nodes", "[TaggedNodeHash]") {
		// make sure template and parameter functions return the same value
		REQUIRE(TaggedNodeHash::getEmptyNodeHash<1>() == TaggedNodeHash::getEmptyNodeHash(pos_type(1)));
		REQUIRE(TaggedNodeHash::getEmptyNodeHash<2>() == TaggedNodeHash::getEmptyNodeHash(pos_type(2)));
		REQUIRE(TaggedNodeHash::getEmptyNodeHash<3>() == TaggedNodeHash::getEmptyNodeHash(pos_type(3)));
		REQUIRE(TaggedNodeHash::getEmptyNodeHash<4>() == TaggedNodeHash::getEmptyNodeHash(pos_type(4)));
		// make sure the hashes are pairwise different
		REQUIRE(TaggedNodeHash::getEmptyNodeHash<1>() != TaggedNodeHash::getEmptyNodeHash<2>());
		REQUIRE(TaggedNodeHash::getEmptyNodeHash<1>() != TaggedNodeHash::getEmptyNodeHash<3>());
		REQUIRE(TaggedNodeHash::getEmptyNodeHash<1>() != TaggedNodeHash::getEmptyNodeHash<4>());
		REQUIRE(TaggedNodeHash::getEmptyNodeHash<2>() != TaggedNodeHash::getEmptyNodeHash<3>());
		REQUIRE(TaggedNodeHash::getEmptyNodeHash<2>() != TaggedNodeHash::getEmptyNodeHash<4>());
		REQUIRE(TaggedNodeHash::getEmptyNodeHash<3>() != TaggedNodeHash::getEmptyNodeHash<4>());
	}

	TEST_CASE("construct hash for single entry compressed Node", "[TaggedNodeHash]") {

		auto key = Key<1>{1};
		TaggedNodeHash::getCompressedNodeHash<1>(key, 1.0);
	}

};

#endif //HYPERTRIE_TESTTAGGEDNODEHASH_HPP
