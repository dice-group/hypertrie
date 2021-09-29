#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include <AssetGenerator.hpp>
#include <Dice/hypertrie/internal/util/name_of_type.hpp>
#include <Node_test_configs.hpp>
#include <Dice/hypertrie/internal/raw/RawKey.hpp>

#include <Dice/hypertrie/internal/raw/node/Identifier.hpp>


namespace hypertrie::tests::core::node {

	TEST_SUITE("Hash_or_InplaceNode") {
		using namespace hypertrie::internal::raw;
		TEST_CASE_TEMPLATE("Hash_or_InplaceNode", T,
						   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>) {

			using tri = typename T::tri;
			constexpr size_t depth = T::depth;
			using RawKey_t = RawKey<depth, tri>;
			using Identifier_t = Identifier<depth, tri>;
			using Entry = SingleEntry<depth, tri>;

			SUBCASE("From SEN depth-1 SEN"){
				RawKey_t key{};
				Identifier_t compressed_hash(Entry{key});
				REQUIRE(compressed_hash.is_sen());
				REQUIRE(not compressed_hash.empty());
				REQUIRE(compressed_hash);// inverse of above
			}
		}
//
//		using key_part_type = float;
//
//		TEST_CASE("default construct") {
//			REQUIRE(Hash_or_InplaceNode().hash() == size_t(0));
//		}
//
//
//		template<size_t depth>
//		void singleEntryCompressed() {
//			auto key = Key<depth>{};
//			auto compressed_hash = TNS::getCompressedNodeHash(key, 1.3);
//			REQUIRE(compressed_hash.isCompressed());
//			REQUIRE(not compressed_hash.empty());
//			REQUIRE(compressed_hash);// inverse of above
//		}
//
//		TEST_CASE("construct hash for single entry compressed node", "[TensorHash]") {
//			singleEntryCompressed<1>();
//			singleEntryCompressed<2>();
//			singleEntryCompressed<3>();
//			singleEntryCompressed<4>();
//
//			// key and value must be hashed together
//			using IntKey = hypertrie::internal::RawKey<1, int>;
//			REQUIRE(TNS::getCompressedNodeHash(IntKey{1}, int(2)) != TNS::getCompressedNodeHash(IntKey{2}, int(1)));
//			// different order of the key_parts in a key must lead to different hash
//			REQUIRE(TNS::getCompressedNodeHash<2>(Key<2>{1, 2}, 1.3) != TNS::getCompressedNodeHash<2>(Key<2>{2, 1}, 1.3));
//		}
//
//		TEST_CASE("add and remove starting with compressed", "[TensorHash]") {
//			constexpr auto depth = 2;
//			using value_type = double;
//			const auto keys = std::vector<Key<depth>>{{4.3, 7.1},
//													  {13, 2},
//													  {11, 2},
//													  {-3, 0}};
//			const auto values = std::vector<value_type>{4.2, 7.0, -100, 15.75};
//
//			const TensorHash hash = TNS::getCompressedNodeHash(keys[0], values[0]);
//			REQUIRE(hash.isCompressed());
//
//			TensorHash hash1 = hash;
//			hash1.addEntry(keys[1], values[1]);
//			REQUIRE(hash1.isUncompressed());
//			REQUIRE(TensorHash(hash1).removeEntry(keys[1], values[1], true) == hash);
//
//			TensorHash hash2 = hash1;
//			hash2.addEntry(keys[2], values[2]);
//			REQUIRE(hash2.isUncompressed());
//			REQUIRE(TensorHash(hash2).removeEntry(keys[2], values[2], false) == hash1);
//
//			TensorHash hash3 = hash2;
//			hash3.addEntry(keys[3], values[3]);
//			REQUIRE(hash3.isUncompressed());
//			REQUIRE(TensorHash(hash3).removeEntry(keys[3], values[3], false) == hash2);
//
//			REQUIRE(TensorHash(hash3)
//							.removeEntry(keys[2], values[2], false)
//							.removeEntry(keys[1], values[1], false)
//							.removeEntry(keys[3], values[3], true) == hash);
//		}
//
//		TEST_CASE("add and remove starting with uncompressed", "[TensorHash]") {
//			constexpr auto depth = 2;
//			using value_type = double;
//			const auto keys = std::vector<Key<depth>>{{4.3, 7.1},
//													  {13, 2},
//													  {11, 2},
//													  {-3, 0}};
//			const auto values = std::vector<value_type>{4.2, 7.0, -100, 15.75};
//
//			const TensorHash hash = TensorHash{}.addEntry(keys[0], values[0]);
//			REQUIRE(hash.isUncompressed());
//
//			TensorHash hash1 = hash;
//			hash1.addEntry(keys[1], values[1]);
//			REQUIRE(hash1.isUncompressed());
//			REQUIRE(TensorHash(hash1).removeEntry(keys[1], values[1], false) == hash);
//
//			TensorHash hash2 = hash1;
//			hash2.addEntry(keys[2], values[2]);
//			REQUIRE(hash2.isUncompressed());
//			REQUIRE(TensorHash(hash2).removeEntry(keys[2], values[2], false) == hash1);
//
//			TensorHash hash3 = hash2;
//			hash3.addEntry(keys[3], values[3]);
//			REQUIRE(hash3.isUncompressed());
//			REQUIRE(TensorHash(hash3).removeEntry(keys[3], values[3], false) == hash2);
//
//			REQUIRE(TensorHash(hash3)
//							.removeEntry(keys[2], values[2], false)
//							.removeEntry(keys[1], values[1], false)
//							.removeEntry(keys[3], values[3], false) == hash);
//		}
//
//		TEST_CASE("use default sorting", "[TensorHash]") {
//			constexpr auto depth = 2;
//			using value_type = double;
//			const auto keys = std::vector<Key<depth>>{{4.3, 7.1},
//													  {13, 2},
//													  {11, 2},
//													  {-3, 0}};
//			const auto values = std::vector<value_type>{4.2, 7.0, -100, 15.75};
//
//			const TensorHash hash = TNS::getCompressedNodeHash(keys[0], values[0]);
//			const TensorHash hash1 = TensorHash{hash}.addEntry(keys[1], values[1]);
//			const TensorHash hash2 = TensorHash{hash1}.addEntry(keys[2], values[2]);
//			const TensorHash hash3 = TensorHash{hash2}.addEntry(keys[3], values[3]);
//
//			std::vector<TensorHash> hashes{hash, hash1, hash2, hash3};
//
//			std::sort(hashes.begin(), hashes.end());
//		}
	}
}// namespace hypertrie::tests::core::node