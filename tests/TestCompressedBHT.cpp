
#include <catch2/catch.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <set>
#include <unordered_set>

#include <Dice/hypertrie/boolhypertrie.hpp>

#include "utils/GenerateTriples.hpp"


namespace hypertrie::tests::compressedboolhypertrie {
    namespace {
        using namespace fmt::literals;
        using namespace hypertrie;
        using BH = hypertrie::boolhypertrie<unsigned long, internal::container::boost_flat_map, internal::container::tsl_sparse_set>::CompressedBoolHypertrie;
        using const_BH = hypertrie::boolhypertrie<unsigned long, internal::container::boost_flat_map, internal::container::tsl_sparse_set>::const_CompressedBoolHypertrie;
        using Key = BH::Key;
        using SliceKey = BH::SliceKey;
    }

    template<int depth>
    void test_single_write() {
        std::array<std::size_t, depth> ranges{};

        BH t{depth};

        BH t1{2};
        BH::Key k = {8, 64};
        t1.set(k, true);
        REQUIRE(t1[k]);
        ranges.fill(123412);
        auto tuples = utils::generateNTuples<unsigned long, 500, depth, Key>(ranges, 8);
        SECTION("CompressedBoolHypertrie depth {}"_format(depth)) {
            for (const auto &key: tuples) {
                BH t{depth};
                t.set(key, true);
                REQUIRE(t[key]);
            }
        }
    }

    TEST_CASE("test_single write/read", "[CompressedBoolHypertrie]") {
        utils::resetDefaultRandomNumberGenerator();
        test_single_write<3>();
    }

    template<int depth, int range_ = 12342, int count = depth * range_ * 2>
    void test_multi_write_read() {
        std::array<std::size_t, depth> ranges{};
        ranges.fill(range_);
        auto write_entries = utils::generateNTuples<unsigned long, count, depth, Key>(ranges, 8);

        SECTION("Boolhypertrie depth {}"_format(depth)) {
            BH t{depth};
            for (const auto &key: write_entries)
                t.set(key, true);
            for (const auto &key: write_entries)
                REQUIRE(t[key] == true);

        }
    }

    TEST_CASE(" multi_write/read_delete", "[CompressedBoolHypertrie]") {
        utils::resetDefaultRandomNumberGenerator();
        test_multi_write_read<3>();
    }

    void print_key(const Key &key) {
        std::cout << "[" << key[0] << ", " << key[1] << ", " << key[2] << "]\n";
    }

    TEST_CASE("test set/get", "[CompressedBoolHypertrie]") {
        int min = 0;
        int max = 100000;
        BH x(3);
        using keys_vector = std::vector<Key>;
        keys_vector generated_keys{};
        using key_part_type_t = unsigned long;
        key_part_type_t last_key_part = 8;
        for (size_t i = 0; i < 100000; i++) {
            key_part_type_t randNumForKeyPart1 = rand() % (max - min + 1) + min;
            last_key_part = randNumForKeyPart1;
            key_part_type_t randNumForKeyPart2 = rand() % (max - min + 1) + min;
            key_part_type_t randNumForKeyPart3 = rand() % (max - min + 1) + min;
            Key key{randNumForKeyPart1 * 8, randNumForKeyPart2 * 8, randNumForKeyPart3 * 8};
            std::cout << "Inserting Key: ";
            print_key(key);
            generated_keys.push_back(key);
            x.set(key, true);
        }

        Key key11{last_key_part, last_key_part, last_key_part};
        x.set(key11, true);
        REQUIRE(x[key11] == true);

        for (keys_vector::iterator iter = generated_keys.begin(); iter != generated_keys.end(); iter++) {
            REQUIRE(x[*iter] == true);
        }
    }

    template<int depth, int range_ = 15, int count = depth * range_ * 2>
    void test_slicing() {
        std::array<std::size_t, depth> ranges{};
        ranges.fill(range_);
        auto entries = utils::generateNTuples<unsigned long, count, depth, Key>(ranges, 8);

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

                    std::variant<std::optional<const_BH>, bool> variant = t[key];
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

    TEST_CASE("Compressed Slicing", "[BoolHypertrie]") {
        utils::resetDefaultRandomNumberGenerator();
        test_slicing<3>();
    }
}