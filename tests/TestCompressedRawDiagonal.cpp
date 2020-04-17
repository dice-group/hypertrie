//
// Created by botour-pc on 4/17/20.
//

#include "Dice/hypertrie/internal/compressed/RawCompressedBoolHypertrie_Hash_Diagonal_impl.hpp"
#include <Dice/hypertrie/internal/container/AllContainer.hpp>

namespace hypertrie::tests::compressedboolhypertrie {
    namespace {
        using pos_type = hypertrie::internal::pos_type;
        template<pos_type depth, bool compressed>
        using RCompressedBH = hypertrie::internal::compressed::RawCompressedBoolHypertrie<depth, unsigned long, hypertrie::internal::container::tsl_sparse_map, hypertrie::internal::container::boost_flat_set, compressed>;

        template<pos_type depth>
        using Node = RCompressedBH<depth, false>;

        template<pos_type depth>
        using CompressedNode = RCompressedBH<depth, true>;

        template<pos_type depth>
        using NodePointer = hypertrie::internal::util::CompressedBoolHyperTrieTaggedPointer<CompressedNode<depth> *, Node<depth> *, 8>;

        template<pos_type diag_depth, pos_type depth, bool compressed>
        using RawDiagonal = hypertrie::internal::compressed::RawCompressedBHTHashDiagonal<diag_depth, depth, compressed, unsigned long, hypertrie::internal::container::tsl_sparse_map, hypertrie::internal::container::boost_flat_set>;
    }

    TEST_CASE("At least compiles") {
        using RD = RawDiagonal<2, 2, true>;

        using RD2 = RawDiagonal<1, 2, true>;
        using RD5 = RawDiagonal<3, 3, false>;
        using Key = Node<3>::Key;
        Node<3> x{};
        Key key1 = {8, 8, 8};
        Key key2 = {8, 24, 16};
        Key key3 = {8, 16, 32};
        x.set(key1);
        x.set(key2);
        x.set(key3);
        RD5 d{&x};
        RD5::init(&d);
        REQUIRE(!RD5::empty(&d));
        REQUIRE(RD5::currentKeyPart(&d) == 8);
    }
}