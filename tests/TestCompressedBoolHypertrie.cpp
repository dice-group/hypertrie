//
// Created by burhan on 22.01.20.
//

#ifndef HYPERTRIE_TESTRAWCOMPRESSEDBOOLHYPERTRIE_HPP
#define HYPERTRIE_TESTRAWCOMPRESSEDBOOLHYPERTRIE_HPP

#include <catch2/catch.hpp>

#include <iostream>
#include <memory>
#include "utils/GenerateTriples.hpp"
#include <set>
#include <Dice/hypertrie/internal/compressed/RawCompressedBoolHypertrie_impl.hpp>


namespace hypertrie::tests::raw_compressedboolhypertrie {
    namespace {
        using namespace hypertrie::internal;
        template<pos_type depth, bool compressed>
        using RCompressedBH = hypertrie::internal::compressed::RawCompressedBoolHypertrie<depth, unsigned long, container::tsl_sparse_map, container::boost_flat_set, compressed>;
        template<pos_type depth>
        using Node = RCompressedBH<depth, false>;
        template<pos_type depth>
        using CompressedNode = RCompressedBH<depth, true>;

        template<pos_type depth>
        using NodePointer = util::CompressedBoolHyperTrieTaggedPointer<CompressedNode<depth> *, Node<depth> *, 8>;
    }

    TEST_CASE("pointer tagging basic") {
        using leaf_node = RCompressedBH<1, false>;
        using key_part_type_t = unsigned long;

        using TaggedPointerType = util::KeyPartTaggedPointer<leaf_node, unsigned long, 8>;

        size_t index = 0;

        while (index < 1000) {
            leaf_node *child_ptr = new leaf_node{};

            TaggedPointerType compressed_child_in_depth_2_node{child_ptr};

            REQUIRE(compressed_child_in_depth_2_node.getTag() == TaggedPointerType::POINTER_TAG);
            REQUIRE(compressed_child_in_depth_2_node.getPointer() == child_ptr);
            index++;
        }

        key_part_type_t key_part = 8 * 31251235;
        TaggedPointerType compressed_child_in_depth_2_node{key_part};

        REQUIRE(compressed_child_in_depth_2_node.getTag() == TaggedPointerType::INT_TAG);
        REQUIRE(compressed_child_in_depth_2_node.getInt() == key_part);

        int min = 0;
        int max = 1000;
        for (size_t i = 0; i < 1000; i++) {
            int randNum = rand() % (max - min + 1) + min;
            key_part_type_t key_part = randNum * 8;
            TaggedPointerType compressed_child_in_depth_2_node_to_store_INT{key_part};

            REQUIRE(compressed_child_in_depth_2_node_to_store_INT.getTag() == TaggedPointerType::INT_TAG);
            REQUIRE(compressed_child_in_depth_2_node_to_store_INT.getInt() == key_part);
        }

        // Resetting the value of tagged pointer

        using TaggedPointerIntType = util::KeyPartTaggedPointer<key_part_type_t, key_part_type_t, 2>;
        key_part_type_t key_part_new = 146 * 8;
        TaggedPointerIntType t{key_part_new};
        REQUIRE(t.getTag() == TaggedPointerType::INT_TAG);
        REQUIRE(t.getInt() == key_part_new);
        key_part_type_t *key_part_new_ptr = new key_part_type_t{146 * 8};
        t.setPointer(key_part_new_ptr);
        REQUIRE(t.getTag() == TaggedPointerType::POINTER_TAG);
        REQUIRE(t.getPointer() == key_part_new_ptr);
    }

    void printKey(const RCompressedBH<3, false>::Key &key) {
        std::cout << "[" << key[0] << ", " << key[1] << ", " << key[2] << "]\n";
    }

    TEST_CASE("compressed hypertrie: basic features") {
        using key_part_type_t = unsigned long;
        using BHT3Node = Node<3>;
        using BHT2Node = Node<2>;
        using BHT1Node = Node<1>;

        using BHT3CompressedNode = CompressedNode<3>;
        using BHT2CompressedNode = CompressedNode<2>;
        using BHT1CompressedNode = CompressedNode<1>;

        using BHT3NodePointer = NodePointer<3>;
        using BHT2NodePointer = NodePointer<2>;
        using BHT1NodePointer = NodePointer<1>;
        BHT3Node x{};
        BHT3Node::Key key1{16, 24, 48};

        BHT3Node::Key key2{16, 16, 48};
        x.set(key1);
        x.set(key2);

        REQUIRE(x[key1] == true);
        REQUIRE(x.size() == 2);

        const BHT3Node &x1 = x;
        auto y1 = x1.get(0, 16);

        BHT2NodePointer y = x.get(0, 16);
        REQUIRE(!y.isEmpty());
        REQUIRE(y.getTag() == BHT2NodePointer::NON_COMPRESSED_TAG);
        REQUIRE(y.getNode() == y1.getNode());
        BHT2NodePointer const compressed_depth3_child = x.get(1, 24);
        REQUIRE(compressed_depth3_child.getCompressedNode()->get(0) == 16);
        REQUIRE(compressed_depth3_child.getCompressedNode()->get(1) == 48);

        BHT2Node const *depth2BHTInstanceOfY = y.getNode();
        BHT1NodePointer bht1_ptr_first = depth2BHTInstanceOfY->get(0, 24);
        BHT1NodePointer bht1_ptr_second = depth2BHTInstanceOfY->get(0, 16);
        REQUIRE(bht1_ptr_first.getTag() == BHT1NodePointer::COMPRESSED_TAG);
        REQUIRE(bht1_ptr_second.getTag() == BHT1NodePointer::COMPRESSED_TAG);
        BHT1CompressedNode *z1 = bht1_ptr_first.getCompressedNode();
        BHT1CompressedNode *z2 = bht1_ptr_second.getCompressedNode();
        REQUIRE(z1->get(0, 48));
        REQUIRE(z2->get(0, 48));
        BHT1NodePointer anotherPointer = y.getNode()->get(1, 48);
        REQUIRE(anotherPointer.getTag() == BHT1NodePointer::NON_COMPRESSED_TAG);

        BHT1Node *anotherZ = anotherPointer.getNode();
        REQUIRE(anotherZ->size() == 2);
        REQUIRE(anotherZ->get(0, 24));
        REQUIRE(anotherZ->get(0, 16));

    }

    TEST_CASE("compressed hyptertrie: diagonal") {
        using Depth3CompressedBoolHypertrie = RCompressedBH<3, false>;
        Depth3CompressedBoolHypertrie x{};
        Depth3CompressedBoolHypertrie::Key key1{16, 16, 16};

        REQUIRE(x.empty());
        x.set(key1);
        REQUIRE(not x.empty());
        // x.set(key2);
        REQUIRE(x.diagonal(16) == true);
        REQUIRE(x.diagonal(8) == false);
        Depth3CompressedBoolHypertrie::Key key2{8, 8, 8};
        x.set(key2);
        REQUIRE(x.diagonal(16) == true);
        REQUIRE(x.diagonal(8) == true);

        Depth3CompressedBoolHypertrie::Key key3{16, 24, 16};
        x.set(key3);

        Depth3CompressedBoolHypertrie::Key key4{16, 32, 32};
        x.set(key4);
        REQUIRE(x.diagonal(16) == true);
        REQUIRE(x.diagonal(8) == true);
    }

    TEST_CASE("compressed hypertrie: test API") {
        using BHT3Node = RCompressedBH<3, false>;
        using Key = BHT3Node::Key;

        Key key1 = {8, 8, 16};
        Key key2 = {8, 24, 16};
        Key key3 = {8, 16, 32};
        BHT3Node *x = new BHT3Node{};
        std::vector<pos_type> positions3 = {1, 2};
        x->set(key1);
        x->set(key2);
        x->set(key3);
        auto dim3 = x->minCardPos();
        REQUIRE(dim3 == 0);

        auto dim33 = x->minCardPos(positions3);
        REQUIRE(*dim33 == 2);

        std::vector<pos_type> positions33 = {1};
        auto dim333 = x->minCardPos(positions33);
        REQUIRE(*dim333 == 1);

        auto y = x->get(0, 8);
        auto dim2 = y.getNode()->minCardPos();
        REQUIRE(dim2 == 1);
        std::vector<pos_type> positions2 = {0};
        auto dim22 = y.getNode()->minCardPos(positions2);
        REQUIRE(*dim22 == 0);
    }

    TEST_CASE("compressed hypertrie: diagonal APIs") {
        using key_part_type_t = unsigned long;
        using BHT3Node = Node<3>;
        using BHT2Node = Node<2>;
        using BHT1Node = Node<1>;
        using Key = BHT3Node::Key;

        using BHT3CompressedNode = CompressedNode<3>;
        using BHT2CompressedNode = CompressedNode<2>;
        using BHT1CompressedNode = CompressedNode<1>;

        using BHT3NodePointer = NodePointer<3>;
        using BHT2NodePointer = NodePointer<2>;
        using BHT1NodePointer = NodePointer<1>;

        BHT3Node *x = new BHT3Node{};
        // Setup the keys
        Key key1{16, 24, 48};
        Key key2{16, 16, 48};
        Key key3{32, 16, 80};
        Key key4{16, 16, 80};
        Key key5{16, 80, 80};
        Key key6{80, 80, 80};

        REQUIRE(x->empty());
        x->set(key1);
        x->set(key2);
        x->set(key3);
        x->set(key4);
        x->set(key5);
        x->set(key6);
        REQUIRE(not x->empty());

        REQUIRE(x->diagonal(80));
        std::vector<pos_type> positions = {0};
        NodePointer<2> pointer1 = x->diagonal<1>(positions, 80);
        REQUIRE(not pointer1.isEmpty());
        REQUIRE(pointer1.getTag() == NodePointer<2>::COMPRESSED_TAG);
        BHT2CompressedNode *child1 = pointer1.getCompressedNode();
        REQUIRE(not child1->empty());
        REQUIRE(!child1->diagonal(81));
        REQUIRE(child1->diagonal(80));

        NodePointer<2> pointer2 = x->diagonal<1>(positions, 32);
        REQUIRE(!pointer2.isEmpty());
        REQUIRE(pointer2.getTag() == NodePointer<2>::COMPRESSED_TAG);
        BHT2CompressedNode *child2 = pointer2.getCompressedNode();
        NodePointer<1> pointer3 = child2->diagonal<1>({0}, 16);
        BHT1CompressedNode *child3 = pointer3.getCompressedNode();
        REQUIRE(child3->diagonal(80));

        NodePointer<1> pointer4 = child2->diagonal<1>({1}, 80);
        BHT1CompressedNode *child4 = pointer4.getCompressedNode();
        REQUIRE(child4->diagonal(16));

        REQUIRE(child2->diagonal<1>({0}, 80).isEmpty());

        std::vector<pos_type> positions1 = {0, 2};
        NodePointer<1> pointer5 = x->diagonal<2>(positions1, 80);
        REQUIRE(!pointer5.isEmpty());
        BHT1CompressedNode *child5 = pointer5.getCompressedNode();
        REQUIRE(!child5->diagonal(81));
        REQUIRE(child5->diagonal(80));

        std::vector<pos_type> positions2 = {1};
        BHT2NodePointer pointer6 = x->diagonal<1>(positions2, 24);
        REQUIRE(!pointer6.isEmpty());
        BHT2CompressedNode *child6 = pointer6.getCompressedNode();
        REQUIRE(child6->get(0, 16).getCompressedNode()->get(0, 48));
        REQUIRE(child6->get(1, 48).getCompressedNode()->get(0, 16));

        BHT1CompressedNode *child7 = child6->diagonal<1>(positions, 16).getCompressedNode();
        REQUIRE(child7->diagonal(48));

        BHT2NodePointer pointer7 = x->diagonal<1>(positions2, 16);
        REQUIRE(!pointer7.isEmpty());
        REQUIRE(pointer7.getTag() == BHT2NodePointer::NON_COMPRESSED_TAG);
        Node<2> *child8 = pointer7.getNode();
        REQUIRE(child8->size() == 3);

        NodePointer<1> pointer8 = child8->diagonal<1>({1}, 48);
        REQUIRE(pointer8.getTag() == NodePointer<1>::COMPRESSED_TAG);
        REQUIRE(pointer8.getCompressedNode()->diagonal(16));
        NodePointer<1> pointer9 = child8->diagonal<1>({0}, 32);
        REQUIRE(pointer9.getTag() == NodePointer<1>::COMPRESSED_TAG);
        REQUIRE(pointer9.getCompressedNode()->diagonal(80));

        NodePointer<1> pointer10 = child8->diagonal<1>({0}, 16);
        REQUIRE(pointer10.getNode()->diagonal(80));
        REQUIRE(pointer10.getNode()->diagonal(48));
        REQUIRE(not pointer10.getNode()->empty());
        NodePointer<1> pointer11 = x->get(0, 16).getNode()->get(0, 16);
        REQUIRE(pointer10.getNode() == pointer11.getNode());
    }

    TEST_CASE("compressed hypertrie: basic set/ get") {
        /**
         * Hint, these keys are invalid as they don't represent the actual values of key_part (multiple of 8). Those keys are for testing on depth=3 nodes only.
         *
         */
        using key_part_type_t = unsigned long;
        RCompressedBH<3, false> x{};
        RCompressedBH<3, false>::Key key1{16, 24, 48};

        RCompressedBH<3, false>::Key key2{16, 16, 48};

        RCompressedBH<3, false>::Key key3{32, 16, 80};

        RCompressedBH<3, false>::Key key4{16, 16, 80};

        RCompressedBH<3, false>::Key key5{16, 80, 80};

        RCompressedBH<3, false>::Key key6{80, 80, 80};

        RCompressedBH<3, false>::Key key7{80, 16, 80};

        RCompressedBH<3, false>::Key key8{80, 16, 80};

        RCompressedBH<3, false>::Key key9{88, 32, 80};

        RCompressedBH<3, false>::Key key10{88, 32, 88};

        // REQUIRE(x[key1] == false);
        x.set(key1);


        // REQUIRE(x[key1] == true);
        // Verify the storage of compressed children on depth=3 nodes
        std::array<unsigned long, 2> compressed_value_edge0 = (*x.compressed_edges[0].begin()).second;
        REQUIRE(compressed_value_edge0[0] == 24);
        REQUIRE(compressed_value_edge0[1] == 48);

        std::array<unsigned long, 2> compressed_value_edge1 = (*x.compressed_edges[1].begin()).second;
        REQUIRE(compressed_value_edge1[0] == 16);
        REQUIRE(compressed_value_edge1[1] == 48);

        std::array<unsigned long, 2> compressed_value_edge2 = (*x.compressed_edges[2].begin()).second;
        REQUIRE(compressed_value_edge2[0] == 16);
        REQUIRE(compressed_value_edge2[1] == 24);
        // REQUIRE(x[key3] == false);

        x.set(key2);
        REQUIRE(x[key3] == false);
        REQUIRE(x[key2] == true);
        REQUIRE(x[key1] == true);
        x.set(key3);
        REQUIRE(x[key2] == true);
        REQUIRE(x[key3] == true);
        REQUIRE(x[key1] == true);

        x.set(key4);
        REQUIRE(x[key2] == true);
        REQUIRE(x[key3] == true);
        REQUIRE(x[key1] == true);
        REQUIRE(x[key4] == true);

        x.set(key5);
        REQUIRE(x[key2] == true);
        REQUIRE(x[key3] == true);
        REQUIRE(x[key1] == true);
        REQUIRE(x[key4] == true);
        REQUIRE(x[key5] == true);

        x.set(key6);
        REQUIRE(x[key2] == true);
        REQUIRE(x[key3] == true);
        REQUIRE(x[key1] == true);
        REQUIRE(x[key4] == true);
        REQUIRE(x[key5] == true);
        REQUIRE(x[key6] == true);

        x.set(key7);
        REQUIRE(x[key2] == true);
        REQUIRE(x[key3] == true);
        REQUIRE(x[key1] == true);
        REQUIRE(x[key4] == true);
        REQUIRE(x[key5] == true);
        REQUIRE(x[key6] == true);
        REQUIRE(x[key7] == true);

        x.set(key8);
        REQUIRE(x[key2] == true);
        REQUIRE(x[key3] == true);
        REQUIRE(x[key1] == true);
        REQUIRE(x[key4] == true);
        REQUIRE(x[key5] == true);
        REQUIRE(x[key6] == true);
        REQUIRE(x[key7] == true);
        REQUIRE(x[key8] == true);

        x.set(key9);
        REQUIRE(x[key2] == true);
        REQUIRE(x[key3] == true);
        REQUIRE(x[key1] == true);
        REQUIRE(x[key4] == true);
        REQUIRE(x[key5] == true);
        REQUIRE(x[key6] == true);
        REQUIRE(x[key7] == true);
        REQUIRE(x[key8] == true);
        REQUIRE(x[key9] == true);

        x.set(key10);
        REQUIRE(x[key2] == true);
        REQUIRE(x[key3] == true);
        REQUIRE(x[key1] == true);
        REQUIRE(x[key4] == true);
        REQUIRE(x[key5] == true);
        REQUIRE(x[key6] == true);
        REQUIRE(x[key7] == true);
        REQUIRE(x[key8] == true);
        REQUIRE(x[key9] == true);
        REQUIRE(x[key10] == true);

        int min = 0;
        int max = 100000;

        using key_vector = std::vector<RCompressedBH<3, false>::Key>;
        key_vector generated_keys{};
        key_part_type_t last_key_part = 8;
        for (size_t i = 0; i < 100000; i++) {
            key_part_type_t randNumForKeyPart1 = rand() % (max - min + 1) + min;
            last_key_part = randNumForKeyPart1;
            key_part_type_t randNumForKeyPart2 = rand() % (max - min + 1) + min;
            key_part_type_t randNumForKeyPart3 = rand() % (max - min + 1) + min;
            RCompressedBH<3, false>::Key key{randNumForKeyPart1 * 8, randNumForKeyPart2 * 8, randNumForKeyPart3 * 8};
            std::cout << "Inserting Key: ";
            printKey(key);
            generated_keys.push_back(key);
            x.set(key);
        }

        RCompressedBH<3, false>::Key key11{last_key_part, last_key_part, last_key_part};
        x.set(key11);
        REQUIRE(x[key11] == true);
        REQUIRE(x.diagonal(last_key_part) == true);
        for (key_vector::iterator iter = generated_keys.begin(); iter != generated_keys.end(); iter++) {
            REQUIRE(x[*iter] == true);
        }
    }

    TEST_CASE("test custom slicing CompressedBHT", "[RawCompressedBoolHypertrie]") {
        using key_part_type_t = unsigned long;
        using BHT3Node = Node<3>;
        using BHT2Node = Node<2>;
        using BHT1Node = Node<1>;
        using Key = BHT3Node::Key;

        using BHT3CompressedNode = CompressedNode<3>;
        using BHT2CompressedNode = CompressedNode<2>;
        using BHT1CompressedNode = CompressedNode<1>;

        using BHT3NodePointer = NodePointer<3>;
        using BHT2NodePointer = NodePointer<2>;
        using BHT1NodePointer = NodePointer<1>;

        BHT3Node *x = new BHT3Node{};
        // Setup the keys
        Key key1{16, 24, 48};
        Key key2{16, 16, 48};
        Key key3{32, 16, 80};
        Key key4{16, 16, 80};
        Key key5{16, 80, 80};
        Key key6{80, 80, 80};

        x->set(key1);
        x->set(key2);
        x->set(key3);
        x->set(key4);
        x->set(key5);
        x->set(key6);

        REQUIRE((*x)[key1]);
        REQUIRE((*x)[key2]);
        REQUIRE((*x)[key3]);
        REQUIRE((*x)[key4]);
        REQUIRE((*x)[key5]);
        REQUIRE((*x)[key6]);

        SECTION("Test NodePointer reinterpret_cast") {
            BHT3Node::SliceKey slice_key_0_32 = {32, std::nullopt, std::nullopt};
            BHT2NodePointer slice_0_32 = x->operator[]<2>(slice_key_0_32);

            BHT2NodePointer new_ptr{slice_0_32.getPointer()};

            REQUIRE(new_ptr.getTag() == BHT2NodePointer::COMPRESSED_TAG);
            REQUIRE(new_ptr.getCompressedNode()->size() == 1);
        }
        // Slicing
        BHT3Node::SliceKey slice_key_0_32 = {32, std::nullopt, std::nullopt};
        BHT2NodePointer slice_0_32 = x->operator[]<2>(slice_key_0_32);
        REQUIRE(!slice_0_32.isEmpty());
        REQUIRE(slice_0_32.getTag() == BHT2NodePointer::COMPRESSED_TAG);

        BHT2CompressedNode *compressedNode1 = slice_0_32.getCompressedNode();
        REQUIRE(compressedNode1->get(0) == 16);
        REQUIRE(compressedNode1->get(1) == 80);
        REQUIRE(compressedNode1->size() == 1);

        BHT2CompressedNode::SliceKey slice_key_0_32_16 = {16, std::nullopt};
        BHT1NodePointer slice_0_32_16 = compressedNode1->operator[]<1>(slice_key_0_32_16);
        REQUIRE(slice_0_32_16.getTag() == BHT2NodePointer::COMPRESSED_TAG);
        //REQUIRE(!slice_0_32_16.getCompressedNode()->diagonal(81));
        //REQUIRE(slice_0_32_16.getCompressedNode()->diagonal(80));
        REQUIRE(slice_0_32_16.getCompressedNode()->size() == 1);

        BHT2CompressedNode::SliceKey slice_key_0_32_80 = {std::nullopt, 80};
        BHT1NodePointer slice_0_32_80 = compressedNode1->operator[]<1>(slice_key_0_32_80);
        REQUIRE(slice_0_32_80.getTag() == BHT2NodePointer::COMPRESSED_TAG);
        //REQUIRE(!slice_0_32_80.getCompressedNode()->diagonal(81));
        // REQUIRE(slice_0_32_80.getCompressedNode()->diagonal(16));
        REQUIRE(slice_0_32_80.getCompressedNode()->size() == 1);

        BHT3Node::SliceKey slice_key_0_80 = {80, std::nullopt, std::nullopt};
        BHT2NodePointer slice_0_80 = x->operator[]<2>(slice_key_0_80);
        REQUIRE(!slice_0_80.isEmpty());
        REQUIRE(slice_0_80.getTag() == BHT2NodePointer::COMPRESSED_TAG);
        //REQUIRE(slice_0_80.getCompressedNode()->diagonal(80));

        BHT3Node::SliceKey slice_key_1_24 = {std::nullopt, 24, std::nullopt};
        BHT2NodePointer compressedNode2 = x->operator[]<2>(slice_key_1_24);
        REQUIRE(compressedNode2.getTag() == BHT2NodePointer::COMPRESSED_TAG);
        REQUIRE(compressedNode2.getCompressedNode()->get(0) == 16);
        REQUIRE(compressedNode2.getCompressedNode()->get(1) == 48);

        BHT2NodePointer emptyNode = x->operator[]<2>({std::nullopt, std::nullopt, 24});
        REQUIRE(emptyNode.isEmpty());

        BHT3Node::SliceKey slice_key2_0_32_80 = {32, std::nullopt, 80};
        BHT1NodePointer slice2_0_32_80 = x->operator[]<1>(slice_key2_0_32_80);
        REQUIRE(slice2_0_32_80.getCompressedNode()->diagonal(16));
        REQUIRE(!slice2_0_32_80.getCompressedNode()->diagonal(80));

        BHT3Node::SliceKey slice_key2_0_80_80 = {80, std::nullopt, 80};
        BHT1NodePointer slice2_0_80_80 = x->operator[]<1>(slice_key2_0_80_80);
        REQUIRE(!slice2_0_80_80.getCompressedNode()->diagonal(16));
        REQUIRE(slice2_0_80_80.getCompressedNode()->diagonal(80));

        BHT3Node::SliceKey slice_key_0_16 = {16, std::nullopt, std::nullopt};
        BHT2NodePointer slice_0_16 = x->operator[]<2>(slice_key_0_16);
        REQUIRE(slice_0_16.getTag() == BHT2NodePointer::NON_COMPRESSED_TAG);
        REQUIRE(slice_0_16.getNode()->size() == 4);
        REQUIRE(slice_0_16.getNode()->diagonal(80));

        BHT2Node *node2 = slice_0_16.getNode();
        BHT2Node::SliceKey slice_key_0_16_24 = {24, std::nullopt};
        BHT1NodePointer slice_0_16_24 = node2->operator[]<1>(slice_key_0_16_24);
        REQUIRE(slice_0_16_24.getTag() == BHT2NodePointer::COMPRESSED_TAG);
        REQUIRE(!slice_0_16_24.getCompressedNode()->diagonal(24));
        REQUIRE(slice_0_16_24.getCompressedNode()->diagonal(48));
        REQUIRE(slice_0_16_24.getCompressedNode()->get(0, 48));

        BHT2Node::SliceKey slice_key_0_16_80 = {80, std::nullopt};
        BHT1NodePointer slice_0_16_80 = node2->operator[]<1>(slice_key_0_16_80);
        REQUIRE(slice_0_16_80.getTag() == BHT2NodePointer::COMPRESSED_TAG);
        REQUIRE(!slice_0_16_80.getCompressedNode()->diagonal(24));
        REQUIRE(slice_0_16_80.getCompressedNode()->diagonal(80));
        REQUIRE(slice_0_16_80.getCompressedNode()->get(0, 80));

        BHT1NodePointer slice_0_16_16 = node2->operator[]<1>({16, std::nullopt});
        REQUIRE(slice_0_16_16.getTag() == BHT2NodePointer::NON_COMPRESSED_TAG);


        BHT3Node::SliceKey slice_key_1_16 = {std::nullopt, 16, std::nullopt};
        BHT2NodePointer slice_1_16 = x->operator[]<2>(slice_key_1_16);
        REQUIRE(slice_1_16.getTag() == BHT2NodePointer::NON_COMPRESSED_TAG);
        BHT1NodePointer slice_1_16_16 = slice_1_16.getNode()->operator[]<1>({16, std::nullopt});
        REQUIRE(slice_1_16_16.getNode() == slice_0_16_16.getNode());
    }

    TEST_CASE("test  slicing depth 3 CompressedBHT", "[RawCompressedBoolHypertrie]") {
        using key_part_type_t = unsigned long;
        using BHT3Node = Node<3>;
        using BHT2Node = Node<2>;
        using BHT1Node = Node<1>;

        using BHT3CompressedNode = CompressedNode<3>;
        using BHT2CompressedNode = CompressedNode<2>;
        using BHT1CompressedNode = CompressedNode<1>;

        using BHT3NodePointer = NodePointer<3>;
        using BHT2NodePointer = NodePointer<2>;
        using BHT1NodePointer = NodePointer<1>;

        utils::resetDefaultRandomNumberGenerator();
        auto tuples = utils::generateNTuples<BHT3Node::key_part_type, 4000, 3>({70, 14, 140}, 8);

        auto x = new BHT3Node{};
        for (auto tuple : tuples) {
            x->set(tuple);
        }

        SECTION("slice by 1 position") {
            std::set<BHT2Node::Key> slice_entries_1_16{};
            std::set<BHT2Node::Key> slice_entries_0_24{};
            std::set<BHT2Node::Key> slice_entries_2_56{};
            for (auto tuple : tuples) {
                if (tuple[1] == 16) {
                    slice_entries_1_16.insert({tuple[0], tuple[2]});
                }
            }
            for (auto tuple : tuples) {
                if (tuple[0] == 24) {
                    slice_entries_0_24.insert({tuple[1], tuple[2]});
                }
            }
            for (auto tuple : tuples) {
                if (tuple[2] == 56) {
                    slice_entries_2_56.insert({tuple[0], tuple[1]});
                }
            }
            BHT3Node::SliceKey slice_key_1_16 = {std::nullopt, 16, std::nullopt};
            BHT2NodePointer slice_1_16 = x->operator[]<2>(slice_key_1_16);

            BHT3Node::SliceKey slice_key_0_24 = {24, std::nullopt, std::nullopt};
            BHT2NodePointer slice_0_24 = x->operator[]<2>(slice_key_0_24);

            BHT3Node::SliceKey slice_key_2_56 = {std::nullopt, std::nullopt, 56};
            BHT2NodePointer slice_2_56 = x->operator[]<2>(slice_key_2_56);

            REQUIRE(!slice_1_16.isEmpty());
            for (const auto &entry : slice_entries_1_16) {
                if (slice_1_16.getTag() == BHT2NodePointer::COMPRESSED_TAG) {
                    REQUIRE(slice_1_16.getCompressedNode()->operator[](entry));
                } else {
                    REQUIRE(slice_1_16.getNode()->operator[](entry));
                }

                if (slice_1_16.getTag() == BHT2NodePointer::COMPRESSED_TAG) {
                    REQUIRE(slice_1_16.getCompressedNode()->size() == slice_entries_1_16.size());
                } else {
                    REQUIRE(slice_1_16.getNode()->size() == slice_entries_1_16.size());
                }
            }
            REQUIRE(!slice_0_24.isEmpty());
            for (const auto &entry : slice_entries_0_24) {
                if (slice_0_24.getTag() == BHT2NodePointer::COMPRESSED_TAG) {
                    REQUIRE(slice_0_24.getCompressedNode()->operator[](entry));
                } else {
                    REQUIRE(slice_0_24.getNode()->operator[](entry));
                }
                if (slice_0_24.getTag() == BHT2NodePointer::COMPRESSED_TAG) {
                    REQUIRE(slice_0_24.getCompressedNode()->size() == slice_entries_0_24.size());
                } else {
                    REQUIRE(slice_0_24.getNode()->size() == slice_entries_0_24.size());
                }
            }

            REQUIRE(!slice_2_56.isEmpty());
            for (const auto &entry : slice_entries_2_56) {
                if (slice_2_56.getTag() == BHT2NodePointer::COMPRESSED_TAG) {
                    REQUIRE(slice_2_56.getCompressedNode()->operator[](entry));
                } else {
                    REQUIRE(slice_2_56.getNode()->operator[](entry));
                }
                if (slice_2_56.getTag() == BHT2NodePointer::COMPRESSED_TAG) {
                    REQUIRE(slice_2_56.getCompressedNode()->size() == slice_entries_2_56.size());
                } else {
                    REQUIRE(slice_2_56.getNode()->size() == slice_entries_2_56.size());
                }
            }
        }

        SECTION("slice by 2 positionss") {
            std::set<BHT1Node::Key> slice_entries_1_2_88_360{};
            for (auto tuple : tuples) {
                if (tuple[1] == 88 and tuple[2] == 360) {
                    slice_entries_1_2_88_360.insert({tuple[0]});
                }
            }

            BHT3Node::SliceKey slice_key_1_2_88_36 = {std::nullopt, 88, 360};
            BHT1NodePointer const slice_1_2_88_36 = x->operator[]<1>(slice_key_1_2_88_36);

            REQUIRE(!slice_1_2_88_36.isEmpty());
            for (const auto &entry : slice_entries_1_2_88_360) {
                if (slice_1_2_88_36.getTag() == BHT2NodePointer::COMPRESSED_TAG) {
                    REQUIRE(slice_1_2_88_36.getCompressedNode()->operator[](entry));
                } else {
                    REQUIRE(slice_1_2_88_36.getNode()->operator[](entry));
                }

                if (slice_1_2_88_36.getTag() == BHT2NodePointer::COMPRESSED_TAG) {
                    REQUIRE(slice_1_2_88_36.getCompressedNode()->size() == slice_entries_1_2_88_360.size());
                } else {
                    REQUIRE(slice_1_2_88_36.getNode()->size() == slice_entries_1_2_88_360.size());
                }
            }
        }
    }
}

#endif //HYPERTRIE_TESTRAWCOMPRESSEDBOOLHYPERTRIE_HPP