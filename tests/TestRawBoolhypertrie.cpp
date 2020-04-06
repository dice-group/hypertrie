#ifndef HYPERTRIE_TESTRAWBOOLHYPERTRIE_HPP
#define HYPERTRIE_TESTRAWBOOLHYPERTRIE_HPP

#include <catch2/catch.hpp>

#include <iostream>
#include <memory>
#include <Dice/hypertrie/internal/RawBoolHypertrie.hpp>
#include "utils/GenerateTriples.hpp"
#include <set>

namespace hypertrie::tests::raw_boolhypertrie {
    namespace {
        using namespace hypertrie::internal;
        template<pos_type depth>
        using RBH = hypertrie::internal::RawBoolHypertrie<depth, unsigned long, container::tsl_sparse_map, container::boost_flat_set>;
    }

    TEST_CASE("test  basic features of RawBoolHypertrie", "[RawBoolHypertrie]") {

        RBH<3> x{};
        RBH<3>::Key key{1, 2, 3};
        RBH<3>::Key key2{1, 2, 5};
        x.set(key, true);
        x.set(key2, true);
        REQUIRE(x[key] == true);
        REQUIRE(x.size() == 2);
        std::shared_ptr<RBH<2> const> x1 = x.get(0, 1);
        REQUIRE(bool(x1));

        const RBH<3> &y = x;
        auto y1 = y.get(0, 1);
        REQUIRE(bool(y1));
        REQUIRE(y1->size() == 2);
        REQUIRE(x1 == y1);

        auto y12 = y1->get(0, 2);
        REQUIRE(y12);
        REQUIRE(y12->size() == 2);

        REQUIRE(y12->get(0, 0) == false);
        REQUIRE(y12->get(0, 1) == false);
        REQUIRE(y12->get(0, 2) == false);
        REQUIRE(y12->get(0, 3) == true);
        REQUIRE(y12->get(0, 4) == false);
        REQUIRE(y12->get(0, 5) == true);

        x.set(key2, false);
        REQUIRE(y12->get(0, 5) == false);
        REQUIRE(y12->size() == 1);
        REQUIRE(x.size() == 1);
        REQUIRE(x[key2] == false);
        REQUIRE(x[key] == true);
    }

    TEST_CASE("set and read values", "[RawBoolHypertrie]") {
        RBH<3> x{};
        x.set({1, 2, 3}, true);
        REQUIRE(x[{1, 2, 3}] == true);
        x.set({1, 2, 3}, false);
        REQUIRE(x[{1, 2, 3}] == false);
    }

    TEST_CASE("test  slicing depth 5 BHT", "[RawBoolHypertrie]") {

        utils::resetDefaultRandomNumberGenerator();
        auto tuples = utils::generateNTuples<unsigned long, 500, 5>({15, 15, 15, 15, 15}, 1);

        auto x = std::make_shared<RBH<5>>();
        for (auto tuple : tuples) {
            x->set(tuple, true);
        }

        SECTION("slice by 1 position") {
            std::set<RBH<4>::Key> slice_entries{};
            for (auto tuple : tuples) {
                if (tuple[1] == 5)
                    slice_entries.insert({tuple[0], tuple[2], tuple[3], tuple[4]});
            }
            RBH<5>::SliceKey slice_key = {std::nullopt, 5, std::nullopt, std::nullopt, std::nullopt};
            auto slice = x->operator[]<4>(slice_key);
            REQUIRE(slice);
            for (const auto &entry : slice_entries) {
                REQUIRE((*slice)[entry] == true);
            }
            REQUIRE(slice->size() == slice_entries.size());
        }

        SECTION("slice by 2 position") {
            std::set<RBH<3>::Key> slice_entries{};
            for (auto tuple : tuples) {
                if (tuple[1] == 5 and tuple[4] == 5)
                    slice_entries.insert({tuple[0], tuple[2], tuple[3]});
            }
            RBH<5>::SliceKey slice_key = {std::nullopt, 5, std::nullopt, std::nullopt, 5};
            auto slice = x->operator[]<3>(slice_key);
            REQUIRE(slice);
            for (const auto &entry : slice_entries) {
                REQUIRE((*slice)[entry] == true);
            }
            REQUIRE(slice->size() == slice_entries.size());
        }

        SECTION("slice by 5 position") {
            std::set<RBH<5>::Key> slice_entries{};
            RBH<5>::SliceKey slice_key = {1, 2, 3, 4, 5};
            auto slice = x->template operator[]<0>(slice_key);
            REQUIRE(slice == (*x)[{1, 2, 3, 4, 5}]);
        }

    }
}

#endif //HYPERTRIE_TESTRAWBOOLHYPERTRIE_HPP
