#ifndef HYPERTRIE_TESTTAGGEDNODEHASH_HPP
#define HYPERTRIE_TESTTAGGEDNODEHASH_HPP

#include <Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp>

#include <Dice/hypertrie/internal/util/RawKey.hpp>

namespace hypertrie::tests::node_based::tagged_node_hash {
	using namespace hypertrie::internal::node_based;

	using key_part_type = float;
	template<pos_type depth>
	using Key = hypertrie::internal::RawKey<depth, key_part_type>;

	using TNS = TaggedNodeHash;

	TEST_CASE("default construct", "[TaggedNodeHash]") {
		REQUIRE(TNS().hash() == size_t(0));
	}

	TEST_CASE("construct hash for empty nodes", "[TaggedNodeHash]") {
		// make sure the hashes are pairwise different
		REQUIRE(TNS::getCompressedEmptyNodeHash<1>() != TNS::getCompressedEmptyNodeHash<2>());
		REQUIRE(TNS::getCompressedEmptyNodeHash<1>() != TNS::getCompressedEmptyNodeHash<3>());
		REQUIRE(TNS::getCompressedEmptyNodeHash<1>() != TNS::getCompressedEmptyNodeHash<4>());
		REQUIRE(TNS::getCompressedEmptyNodeHash<2>() != TNS::getCompressedEmptyNodeHash<3>());
		REQUIRE(TNS::getCompressedEmptyNodeHash<2>() != TNS::getCompressedEmptyNodeHash<4>());
		REQUIRE(TNS::getCompressedEmptyNodeHash<3>() != TNS::getCompressedEmptyNodeHash<4>());
	}

	template<pos_type depth>
	void singleEntryCompressed() {
		auto key = Key<depth>{};
		auto compressed_hash = TNS::getCompressedNodeHash(key, 1.3);
		REQUIRE(compressed_hash.isCompressed());
		REQUIRE(not compressed_hash.empty());
		REQUIRE(compressed_hash);// inverse of above
	}

	TEST_CASE("construct hash for single entry compressed node", "[TaggedNodeHash]") {
		singleEntryCompressed<1>();
		singleEntryCompressed<2>();
		singleEntryCompressed<3>();
		singleEntryCompressed<4>();

		// key and value must be hashed together
		using IntKey = hypertrie::internal::RawKey<1, int>;
		REQUIRE(TNS::getCompressedNodeHash(IntKey{1}, int(2)) != TNS::getCompressedNodeHash(IntKey{2}, int(1)));
		// different order of the key_parts in a key must lead to different hash
		REQUIRE(TNS::getCompressedNodeHash<2>(Key<2>{1, 2}, 1.3) != TNS::getCompressedNodeHash<2>(Key<2>{2, 1}, 1.3));
	}

	TEST_CASE("construct hash for two entries uncompressed node", "[TaggedNodeHash]") {
		constexpr auto depth = 2;
		using value_type = double;
		auto key1 = Key<depth>{4.3, 7.1};
		value_type value1 = 2.3;
		auto key2 = Key<depth>{4.2, 7.0};
		value_type value2 = 21.1;
		auto hash = TNS::getTwoEntriesNodeHash(key1, value1, key2, value2);
		// must be uncompressd
		REQUIRE(hash.isUncompressed());
		// order must not matter
		REQUIRE(hash == TNS::getTwoEntriesNodeHash(key2, value2, key1, value1));

		// creating it manually must be equal
		TaggedNodeHash hash2 = TNS::getCompressedNodeHash(key1, value1);
		REQUIRE(hash2.isCompressed());
		hash2.addEntry(key2, value2);
		REQUIRE(hash2.isUncompressed());
		REQUIRE(hash == hash2);

		// key and value must be hashed together
		REQUIRE(hash != TNS::getTwoEntriesNodeHash(key1, value2, key2, value1));
	}

	TEST_CASE("add and remove starting with compressed", "[TaggedNodeHash]") {
		constexpr auto depth = 2;
		using value_type = double;
		const auto keys = std::vector<Key<depth>>{{4.3, 7.1},
												  {13, 2},
												  {11, 2},
												  {-3, 0}};
		const auto values = std::vector<value_type>{4.2, 7.0, -100, 15.75};

		const TaggedNodeHash hash = TNS::getCompressedNodeHash(keys[0], values[0]);
		REQUIRE(hash.isCompressed());

		TaggedNodeHash hash1 = hash;
		hash1.addEntry(keys[1], values[1]);
		REQUIRE(hash1.isUncompressed());
		REQUIRE(TaggedNodeHash(hash1).removeEntry(keys[1], values[1], true) == hash);

		TaggedNodeHash hash2 = hash1;
		hash2.addEntry(keys[2], values[2]);
		REQUIRE(hash2.isUncompressed());
		REQUIRE(TaggedNodeHash(hash2).removeEntry(keys[2], values[2], false) == hash1);

		TaggedNodeHash hash3 = hash2;
		hash3.addEntry(keys[3], values[3]);
		REQUIRE(hash3.isUncompressed());
		REQUIRE(TaggedNodeHash(hash3).removeEntry(keys[3], values[3], false) == hash2);

		REQUIRE(TaggedNodeHash(hash3)
						.removeEntry(keys[2], values[2], false)
						.removeEntry(keys[1], values[1], false)
						.removeEntry(keys[3], values[3], true) == hash);
	}

	TEST_CASE("add and remove starting with uncompressed", "[TaggedNodeHash]") {
		constexpr auto depth = 2;
		using value_type = double;
		const auto keys = std::vector<Key<depth>>{{4.3, 7.1},
												  {13, 2},
												  {11, 2},
												  {-3, 0}};
		const auto values = std::vector<value_type>{4.2, 7.0, -100, 15.75};

		const TaggedNodeHash hash = TNS::getUncompressedEmptyNodeHash<depth>().addEntry(keys[0], values[0]);
		REQUIRE(hash.isUncompressed());

		TaggedNodeHash hash1 = hash;
		hash1.addEntry(keys[1], values[1]);
		REQUIRE(hash1.isUncompressed());
		REQUIRE(TaggedNodeHash(hash1).removeEntry(keys[1], values[1], false) == hash);

		TaggedNodeHash hash2 = hash1;
		hash2.addEntry(keys[2], values[2]);
		REQUIRE(hash2.isUncompressed());
		REQUIRE(TaggedNodeHash(hash2).removeEntry(keys[2], values[2], false) == hash1);

		TaggedNodeHash hash3 = hash2;
		hash3.addEntry(keys[3], values[3]);
		REQUIRE(hash3.isUncompressed());
		REQUIRE(TaggedNodeHash(hash3).removeEntry(keys[3], values[3], false) == hash2);

		REQUIRE(TaggedNodeHash(hash3)
						.removeEntry(keys[2], values[2], false)
						.removeEntry(keys[1], values[1], false)
						.removeEntry(keys[3], values[3], false) == hash);
	}

	TEST_CASE("use default sorting", "[TaggedNodeHash]") {
		constexpr auto depth = 2;
		using value_type = double;
		const auto keys = std::vector<Key<depth>>{{4.3, 7.1},
												  {13, 2},
												  {11, 2},
												  {-3, 0}};
		const auto values = std::vector<value_type>{4.2, 7.0, -100, 15.75};

		const TaggedNodeHash hash = TNS::getCompressedNodeHash(keys[0], values[0]);
		const TaggedNodeHash hash1 = TaggedNodeHash{hash}.addEntry(keys[1], values[1]);
		const TaggedNodeHash hash2 = TaggedNodeHash{hash1}.addEntry(keys[2], values[2]);
		const TaggedNodeHash hash3 = TaggedNodeHash{hash2}.addEntry(keys[3], values[3]);

		std::vector<TaggedNodeHash> hashes{hash, hash1, hash2, hash3};

		std::sort(hashes.begin(), hashes.end());
	}

};// namespace hypertrie::tests::node_based::tagged_node_hash

#endif//HYPERTRIE_TESTTAGGEDNODEHASH_HPP
