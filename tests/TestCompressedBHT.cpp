
#include <catch2/catch.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <set>
#include <unordered_set>

#include <Dice/hypertrie/boolhypertrie.hpp>

#include "utils/GenerateTriples.hpp"


namespace hypertrie::tests::compressedboolhypertrie{
    namespace {
        using namespace fmt::literals;
        using namespace hypertrie;
        using BH = hypertrie::boolhypertrie<unsigned long, internal::container::boost_flat_map, internal::container::tsl_sparse_set>::CompressedBoolHypertrie ;
        using const_BH = hypertrie::boolhypertrie<unsigned long, internal::container::boost_flat_map, internal::container::tsl_sparse_set>::const_CompressedBoolHypertrie ;
        using Key = BH::Key;
        using SliceKey = BH::SliceKey;
    }

}