#ifndef HYPERTRIE_TESTRAWDIAGONAL_HPP
#define HYPERTRIE_TESTRAWDIAGONAL_HPP

#include <catch2/catch.hpp>

#include <iostream>
#include <memory>
#include <set>
#include <map>
#include <Dice/hypertrie/internal/RawBoolHypertrie_Hash_Diagonal_impl.hpp>
#include "utils/GenerateTriples.hpp"

namespace hypertrie::tests::raw_diagonal {
	namespace {
		using namespace hypertrie::internal;
		template<pos_type depth>
		using RBH = hypertrie::internal::RawBoolHypertrie<depth, unsigned long, container::tsl_sparse_map, container::boost_flat_set>;

		template<pos_type depth, pos_type diag_depth>
		using RD = typename hypertrie::internal::RawHashDiagonal<diag_depth, depth, unsigned long, container::tsl_sparse_map, container::boost_flat_set>;

	}

	TEST_CASE("basic tests", "RawDiagonal") {
		RBH<1> x{};

		auto data = utils::generateNTuples<unsigned long, 50, 1>({50}, 1);
		std::set<unsigned long> key_parts;
		for (const auto &entry : data) {
			key_parts.insert(entry[0]);
			x.set(entry[0], true);
		}
		RD<1, 1> d{x};
		RD<1, 1>::init(&d);
		unsigned long key_part = RD<1, 1>::currentKeyPart(&d);
		REQUIRE(key_part == *key_parts.begin());
		for (const auto &correct_key_part : key_parts) {
			key_part = RD<1, 1>::currentKeyPart(&d);
			REQUIRE(key_part == correct_key_part);
			bool empty = RD<1, 1>::empty(&d);
			REQUIRE(not empty);
			RD<1, 1>::inc(&d);
		}

		bool empty = RD<1, 1>::empty(&d);
		REQUIRE(empty);
	}

	// disabled for now
//	TEST_CASE("5 BHT, iterate over diagonals", "[RawDiagonal]") {
//
//		utils::resetDefaultRandomNumberGenerator();
//		auto tuples = utils::generateNTuples<unsigned long, 500, 5>({15, 15, 15, 15, 15});
//		tuples.push_back({1, 1, 1, 1, 1});
//		tuples.push_back({7, 7, 7, 7, 7});
//
//		auto x = std::make_shared<RBH<5>>();
//		for (auto tuple : tuples) {
//			x->set(tuple, true);
//		}
//
//		SECTION("diagonal by 1 position") {
//			std::map<unsigned long, std::set<RBH<4>::Key>> diagonal_entries{};
//			for (auto tuple : tuples) {
//				if (not diagonal_entries.count(tuple[1])) {
//					diagonal_entries[tuple[1]] = {};
//				}
//				auto &diagonal_value = diagonal_entries[tuple[1]];
//				diagonal_value.insert({tuple[0], tuple[2], tuple[3], tuple[4]});
//			}
//
//			RD<5, 1> diagonal{x.get(), {1}};
//
//			RD<5, 1>::init(&diagonal);
//			while (not RD<5, 1>::empty(&diagonal)) {
//				auto key_part = RD<5, 1>::currentKeyPart(&diagonal);
//				REQUIRE(diagonal_entries.count(key_part));
//				auto diagonal_value = RD<5, 1>::currentValue(&diagonal);
//				auto expeced_diagonal_value_entries = diagonal_entries[key_part];
//				for (const auto &entry : expeced_diagonal_value_entries) {
//					REQUIRE(diagonal_value->operator[](entry) == true);
//				}
//				REQUIRE(diagonal_value->size() == expeced_diagonal_value_entries.size());
//				RD<5, 1>::inc(&diagonal);
//			}
//		}
//
//		SECTION("diagonal by 2 position") {
//			std::map<unsigned long, std::set<RBH<3>::Key>> diagonal_entries{};
//			for (auto tuple : tuples) {
//				if (tuple[1] == tuple[3] and not diagonal_entries.count(tuple[1])) {
//					diagonal_entries[tuple[1]] = {};
//				}
//				if (tuple[1] == tuple[3]) {
//					auto &diagonal_value = diagonal_entries[tuple[1]];
//					diagonal_value.insert({tuple[0], tuple[2], tuple[4]});
//				}
//			}
//
//			RD<5, 2> diagonal{*x, {1, 3}};
//
//			RD<5, 2>::init(&diagonal);
//			while (not RD<5, 2>::empty(&diagonal)) {
//				auto key_part = RD<5, 2>::currentKeyPart(&diagonal);
//				REQUIRE(diagonal_entries.count(key_part));
//				auto diagonal_value = RD<5, 2>::currentValue(&diagonal);
//				auto expeced_diagonal_value_entries = diagonal_entries[key_part];
//				for (const auto &entry : expeced_diagonal_value_entries) {
//					REQUIRE(diagonal_value->operator[](entry) == true);
//				}
//				REQUIRE(diagonal_value->size() == expeced_diagonal_value_entries.size());
//				RD<5, 2>::inc(&diagonal);
//			}
//		}
//
//		SECTION("diagonal by 4 position") {
//			std::map<unsigned long, std::set<RBH<1>::Key>> diagonal_entries{};
//			for (auto tuple : tuples) {
//				if (tuple[1] == tuple[3] and tuple[1] == tuple[2] and tuple[1] == tuple[4] and
//				    not diagonal_entries.count(tuple[1])) {
//					diagonal_entries[tuple[1]] = {};
//				}
//				if (tuple[1] == tuple[3] and tuple[1] == tuple[2] and tuple[1] == tuple[4]) {
//					auto &diagonal_value = diagonal_entries[tuple[1]];
//					diagonal_value.insert({tuple[0]});
//				}
//			}
//
//			RD<5, 4> diagonal{*x, {1, 2, 3, 4}};
//
//			RD<5, 4>::init(&diagonal);
//			while (not RD<5, 4>::empty(&diagonal)) {
//				auto key_part = RD<5, 4>::currentKeyPart(&diagonal);
//				REQUIRE(diagonal_entries.count(key_part));
//				auto diagonal_value = RD<5, 4>::currentValue(&diagonal);
//				auto expeced_diagonal_value_entries = diagonal_entries[key_part];
//				for (const auto &entry : expeced_diagonal_value_entries) {
//					REQUIRE(diagonal_value->operator[](entry) == true);
//				}
//				REQUIRE(diagonal_value->size() == expeced_diagonal_value_entries.size());
//				RD<5, 4>::inc(&diagonal);
//			}
//		}
//
//		SECTION("diagonal by 5 position") {
//			std::set<unsigned long> full_diag_keys{};
//			for (auto tuple : tuples) {
//				if (tuple[1] == tuple[0] and tuple[1] == tuple[3] and tuple[1] == tuple[2] and tuple[1] == tuple[4])
//					full_diag_keys.insert(tuple[0]);
//			}
//
//			RD<5, 5> diagonal{*x};
//
//			RD<5, 5>::init(&diagonal);
//			while (not RD<5, 5>::empty(&diagonal)) {
//				auto key_part = RD<5, 5>::currentKeyPart(&diagonal);
//				REQUIRE(full_diag_keys.count(key_part));
//				RD<5, 5>::inc(&diagonal);
//			}
//		}
//
//	}
}

#endif //HYPERTRIE_TESTRAWDIAGONAL_HPP
