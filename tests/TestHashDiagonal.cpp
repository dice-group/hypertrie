#ifndef HYPERTRIE_TESTHASHDIAGONAL_HPP
#define HYPERTRIE_TESTHASHDIAGONAL_HPP

#include <catch2/catch.hpp>

#include <iostream>
#include <memory>
#include <Dice/hypertrie/boolhypertrie.hpp>
#include "utils/GenerateTriples.hpp"
#include <set>
#include "diagonal/DiagonalTestData.hpp"

namespace hypertrie::tests::hash_diagonal {
	namespace {
		using namespace fmt::literals;
		using namespace hypertrie;
		using BH = ::hypertrie::boolhypertrie<>::BoolHypertrie;
		using const_BH = ::hypertrie::boolhypertrie<>::const_BoolHypertrie;
		using Diagonal = ::hypertrie::boolhypertrie<>::HashDiagonal;
		using Key = BH::Key;
		using SliceKey = BH::SliceKey;
		using pos_type = ::hypertrie::boolhypertrie<>::pos_type;
	}

	// deactivated for now
//	template<typename key_part_type = std::size_t>
//	void test_diagonal(pos_type depth) {
//		const key_part_type max = 15;
//		const std::size_t count = depth * max * 2;
//
//		std::set<uint8_t> all_positions;
//		for (auto i : range(depth))
//			all_positions.insert(i);
//		for (auto &&pos_it : powerset(all_positions)) {
//
//			// generate slice key
//			Diagonal::poss_type positions{pos_it.begin(), pos_it.end()};
//			if (positions.empty())
//				continue;
//			std::sort(positions.begin(), positions.end());
//
//			DiagonalTestData diag_test_data{positions, count, depth, max};
//
//			BH t{depth};
//
//			for (const auto &key : diag_test_data.entries)
//				t.set(key, true);
//
//			SECTION("Boolhypertrie depth {} - diagonal positions ({})"_format(
//					depth, fmt::join(positions.begin(), positions.end(), ", "))) {
//
//				Diagonal iter_diag{t, positions};
//
//				iter_diag.init();
//
//				while (not iter_diag.empty()) {
//					unsigned long key_part = iter_diag.currentKeyPart();
//					REQUIRE(diag_test_data.expected_entries.count(key_part));
//					if (positions.size() == depth) {
//
//					} else {
//						const_BH boolHypertrie = iter_diag.currentValue();
//						auto expected_entries_by_kp = diag_test_data.expected_entries[key_part];
//						REQUIRE(boolHypertrie.size() == expected_entries_by_kp.size());
//						for (const auto &key : boolHypertrie) {
//							REQUIRE(expected_entries_by_kp.count(key));
//						}
//					}
//					++iter_diag;
//				}
//
//				Diagonal probe_diag{t, positions};
//
//			}
//		}
//	}
//
//	TEST_CASE("diagonal", "[HashDiagonal]") {
//		utils::resetDefaultRandomNumberGenerator();
//		test_diagonal(1);
//		test_diagonal(2);
//		test_diagonal(3);
//		test_diagonal(4);
//		test_diagonal(5);
//	}
}

#endif //HYPERTRIE_TESTHASHDIAGONAL_HPP

