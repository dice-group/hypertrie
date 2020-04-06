#include <catch2/catch.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <set>
#include <unordered_set>

#include <Dice/hypertrie/boolhypertrie.hpp>

#include "utils/GenerateTriples.hpp"


namespace hypertrie::tests::boolhypertrie {
	namespace {
		using namespace fmt::literals;
		using namespace hypertrie;
		using BH = hypertrie::boolhypertrie<>::BoolHypertrie;
		using const_BH = hypertrie::boolhypertrie<>::const_BoolHypertrie;
		using Key = BH::Key;
		using SliceKey = BH::SliceKey;
	}

	template<int depth>
	void test_single_write() {
		std::array<std::size_t, depth> ranges{};
		ranges.fill(15);
		auto tuples = utils::generateNTuples<unsigned long, 500, depth, Key>(ranges, 1);
		SECTION("Boolhypertrie depth {}"_format(depth)) {
			for (const auto &key: tuples) {
				BH t{depth};
				t.set(key, true);
				REQUIRE(t[key]);
			}
		}
	}

	TEST_CASE("test_single_write_read", "[BoolHypertrie]") {
		utils::resetDefaultRandomNumberGenerator();
		test_single_write<1>();
		test_single_write<2>();
		test_single_write<3>();
		test_single_write<4>();
		test_single_write<5>();
	}

	template<int depth, int range_ = 15, int count = depth * range_ * 2>
	void test_multi_write_read_delete() {
		std::array<std::size_t, depth> ranges{};
		ranges.fill(range_);
		auto write_entries = utils::generateNTuples<unsigned long, count, depth, Key>(ranges, 1);
		auto remove_entries = utils::generateNTuples<unsigned long, count, depth, Key>(ranges, 1);
		std::set<Key> retaining_entries{write_entries.begin(), write_entries.end()};
		for (const auto &entry : remove_entries)
			retaining_entries.erase(entry);

		SECTION("Boolhypertrie depth {}"_format(depth)) {
			BH t{depth};
			for (const auto &key: write_entries)
				t.set(key, true);
			for (const auto &key: write_entries)
				REQUIRE(t[key] == true);

			for (const auto &key: remove_entries)
				t.set(key, false);
			for (const auto &key: remove_entries)
				REQUIRE(t[key] == false);

			for (const auto &key: retaining_entries)
				REQUIRE(t[key] == true);
		}
	}

	TEST_CASE(" multi_write_read_delete", "[BoolHypertrie]") {
		utils::resetDefaultRandomNumberGenerator();
		test_multi_write_read_delete<1>();
		test_multi_write_read_delete<2>();
		test_multi_write_read_delete<3>();
		test_multi_write_read_delete<4>();
		test_multi_write_read_delete<5, 500>();
	}

	template<int depth, int range_ = 15, int count = depth * range_ * 2>
	void test_slicing() {
		std::array<std::size_t, depth> ranges{};
		ranges.fill(range_);
		auto entries = utils::generateNTuples<unsigned long, count, depth, Key>(ranges, 1);

		BH t{depth};
		for (const auto &key: entries)
			t.set(key, true);

		std::set<int> all_positions;
		for (auto i : iter::range(depth))
			all_positions.insert(i);
		for (auto &&pos_it : iter::powerset(all_positions)) {
			// generate slice key
			std::set<uint> positions{pos_it.begin(), pos_it.end()};
			SECTION("Boolhypertrie depth {} - positions ({})"_format(depth,
																	 fmt::join(positions.begin(), positions.end(),
																			   ", "))) {
				SliceKey key(depth, std::nullopt);

				for ([[maybe_unused]]auto i : iter::range(count * 5)) {

					Key key_parts{};
					for (auto rand_n : utils::gen_random<unsigned long>(positions.size(), 0, range_ - 1))
						key_parts.push_back(rand_n);

					for (auto[pos, key_part] : iter::zip(positions, key_parts))
						key[pos] = key_part;

					std::set<Key> expected_entries;
					for (const auto &entry : entries) {
						bool in_slice = true;
						for (const auto &[pos, key_part] : iter::zip(positions, key_parts)) {
							if (entry[pos] != key_part) {
								in_slice = false;
							}
						}
						if (not in_slice)
							continue;

						std::set<int> remaining_pos;
						std::set_difference(all_positions.begin(), all_positions.end(), positions.begin(),
											positions.end(),
											std::inserter(remaining_pos, remaining_pos.begin()));
						Key slice_entry;
						for (auto pos : remaining_pos) {
							slice_entry.push_back(entry[pos]);
						}
						expected_entries.insert(slice_entry);
					}

					std::variant<std::optional<const_BH >, bool> variant = t[key];
					if (positions.size() == depth) {
						bool x = std::get<bool>(variant);
						REQUIRE(x == bool(expected_entries.size()));
					} else {
						std::optional<const_BH> t_slice_opt = std::get<std::optional<const_BH >>(variant);
						if (not expected_entries.empty()) {
							REQUIRE(t_slice_opt.has_value());
							const_BH t_slice = t_slice_opt.value();
							REQUIRE(t_slice.depth() == depth - positions.size());
							REQUIRE(t_slice.size() == expected_entries.size());
							for (const auto &expected_entry : expected_entries) {
								REQUIRE(t_slice[expected_entry] == true);
							}
						} else {
							REQUIRE(not t_slice_opt.has_value());
						}
					}

				}
			}
		}

	}

	TEST_CASE(" slicing", "[BoolHypertrie]") {
		utils::resetDefaultRandomNumberGenerator();
		test_slicing<1>();
		test_slicing<2>();
		test_slicing<3>();
		test_slicing<4>();
		test_slicing<5>();
	}

	template<int depth, int range_ = 15, int count = depth * range_ * 2>
	void test_iterator() {
		std::array<std::size_t, depth> ranges{};
		ranges.fill(range_);
		auto entries = utils::generateNTuples<unsigned long, count, depth, Key>(ranges, 1);

		BH t{depth};
		for (const auto &key: entries)
			t.set(key, true);

		std::set<Key> entries_set{entries.begin(), entries.end()};


		for (auto i : iter::range(10)) {
			std::set<Key> entries_set_copy{entries_set};
			SECTION("Boolhypertrie depth {} - run {}"_format(depth, i)) {

				size_t c = 0;
				for (auto key : t) {
					REQUIRE(key.size() == depth);
					auto found = entries_set_copy.find(key);
					REQUIRE(found != entries_set_copy.end());
					entries_set_copy.erase(found);
					++c;
				}
				REQUIRE(c == t.size());
			}
		}

	}

	TEST_CASE("iterator", "[BoolHypertrie]") {
		utils::resetDefaultRandomNumberGenerator();
		test_iterator<1>();
		test_iterator<2>();
		test_iterator<3>();
		test_iterator<4>();
		test_iterator<5>();
	}

}