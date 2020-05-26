#ifndef HYPERTRIE_TESTNODE_HPP
#define HYPERTRIE_TESTNODE_HPP

#include <boost/type_index.hpp>
#include <fmt/format.h>
#include <enumerate.hpp>

#include <Dice/hypertrie/internal/node_based/Node.hpp>

#include "../utils/AssetGenerator.hpp"

namespace hypertrie::tests::node_based::node {

    template<typename T>
    auto name_of_type() {
        return std::string{boost::typeindex::type_id_with_cvr<T>().pretty_name()};
    }

    using namespace hypertrie::internal::node_based;

    template<size_t depth, typename key_part_type>
    using Key = hypertrie::internal::RawKey<depth, key_part_type>;

    template<HypertrieInternalTrait tr, pos_type depth>
    void createCompressedNode() {
        using key_part_type = typename tr::key_part_type;
        using value_type = typename tr::value_type;
        using Key = Key<depth, key_part_type>;

        SECTION(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
                            depth, name_of_type<key_part_type>(), name_of_type<value_type>())) {
            utils::RawGenerator<depth, key_part_type, value_type> gen{};

            Key key = gen.key();

            // unused for bool
            value_type value = gen.value();

            CompressedNode<depth, tr> node = [&]() {
                if constexpr (tr::is_bool_valued) return CompressedNode<depth, tr>{key};
                else return CompressedNode<depth, tr>{key, value};
            }();

            REQUIRE(node.key() == key);
            if constexpr (not std::is_same_v<value_type, bool>)
                REQUIRE(node.value() == value);
        }
    }

    TEST_CASE("create compressed node", "[Node]") {
        createCompressedNode<default_bool_Hypertrie_internal_t, 1>();
        createCompressedNode<default_long_Hypertrie_internal_t, 1>();
        createCompressedNode<default_double_Hypertrie_internal_t, 1>();
        createCompressedNode<default_bool_Hypertrie_internal_t, 2>();
        createCompressedNode<default_long_Hypertrie_internal_t, 2>();
        createCompressedNode<default_double_Hypertrie_internal_t, 2>();
        createCompressedNode<default_bool_Hypertrie_internal_t, 3>();
        createCompressedNode<default_long_Hypertrie_internal_t, 3>();
        createCompressedNode<default_double_Hypertrie_internal_t, 3>();
    }

    template<HypertrieInternalTrait tr, pos_type depth>
    void createEmptyUncompressed() {
        using key_part_type = typename tr::key_part_type;
        using value_type = typename tr::value_type;
        SECTION(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
                            depth, name_of_type<key_part_type>(), name_of_type<value_type>())) {

            UncompressedNode<depth, tr> node{};

            for (size_t pos : iter::range(depth))
                REQUIRE(node.edges(pos).size() == 0);
            REQUIRE(node.size() == 0);
        }
    }

    TEST_CASE("create empty uncompressed node", "[Node]") {
        createEmptyUncompressed<default_bool_Hypertrie_internal_t, 1>();
        createEmptyUncompressed<default_long_Hypertrie_internal_t, 1>();
        createEmptyUncompressed<default_double_Hypertrie_internal_t, 1>();
        createEmptyUncompressed<default_bool_Hypertrie_internal_t, 2>();
        createEmptyUncompressed<default_long_Hypertrie_internal_t, 2>();
        createEmptyUncompressed<default_double_Hypertrie_internal_t, 2>();
        createEmptyUncompressed<default_bool_Hypertrie_internal_t, 3>();
        createEmptyUncompressed<default_long_Hypertrie_internal_t, 3>();
        createEmptyUncompressed<default_double_Hypertrie_internal_t, 3>();
    }

    template<HypertrieInternalTrait tr, pos_type depth>
    void insertSingleEntryIntoEmptyUncompressed() {
        using key_part_type = typename tr::key_part_type;
        using value_type = typename tr::value_type;
        using Key = Key<depth, key_part_type>;
        SECTION(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
                            depth, name_of_type<key_part_type>(), name_of_type<value_type>())) {

            utils::RawGenerator<depth, key_part_type, value_type> gen{};

            UncompressedNode<depth, tr> node{};

            auto[key, value] = gen.entry();

            // insert a single entry
            {
                node.insertEntry(key, value);

                if constexpr(depth == 1) {
                    REQUIRE(node.child(0, key[0]) == value);
                } else {
                    for (auto pos : iter::range(depth)) {
                        auto expected_hash = TaggedNodeHash::getCompressedNodeHash(tr::subkey(key, pos), value);
                        REQUIRE(node.edges(pos)[key[pos]] == expected_hash);
                    }
                }
            }
            // insert another value with a common position
            if constexpr (depth > 1) {
                for (auto unchanged_pos : iter::range(depth)) {
                    auto node2 = node;
                    auto second_value = gen.value();
                    auto second_key = key;
                    // increase all but one entries of the second key by 1
                    for (auto[pos, key_part] : iter::enumerate(second_key))
                        if (pos != unchanged_pos)
                            key_part += key_part_type(1);

                    // insert the second entry
                    node2.insertEntry(second_key, second_value);

                    for (auto pos : iter::range(depth)) {
                        if (pos == unchanged_pos) { // there is a sub node with two entries for unchanged_pos
                            auto expected_hash = TaggedNodeHash::getCompressedNodeHash(tr::subkey(key, pos), value)
                                    .addEntry(tr::subkey(second_key, pos), second_value);
                            REQUIRE(node2.edges(pos)[key[pos]] == expected_hash);
                        } else { // there are single entries for all other positions
                            { // entry for key/value
                                auto expected_hash = TaggedNodeHash::getCompressedNodeHash(tr::subkey(key, pos), value);
                                REQUIRE(node2.edges(pos)[key[pos]] == expected_hash);
                            }
                            { // entry for second_key/value
                                auto expected_hash = TaggedNodeHash::getCompressedNodeHash(tr::subkey(second_key, pos),
                                                                                           second_value);
                                REQUIRE(node2.edges(pos)[second_key[pos]] == expected_hash);
                            }
                        }
                    }
                }
            }
        }
    }


    TEST_CASE("insert single entry into uncompressed node", "[Node]") {
        insertSingleEntryIntoEmptyUncompressed<default_bool_Hypertrie_internal_t, 1>();
        insertSingleEntryIntoEmptyUncompressed<default_long_Hypertrie_internal_t, 1>();
        insertSingleEntryIntoEmptyUncompressed<default_double_Hypertrie_internal_t, 1>();
        insertSingleEntryIntoEmptyUncompressed<default_bool_Hypertrie_internal_t, 2>();
        insertSingleEntryIntoEmptyUncompressed<default_long_Hypertrie_internal_t, 2>();
        insertSingleEntryIntoEmptyUncompressed<default_double_Hypertrie_internal_t, 2>();
        insertSingleEntryIntoEmptyUncompressed<default_bool_Hypertrie_internal_t, 3>();
        insertSingleEntryIntoEmptyUncompressed<default_long_Hypertrie_internal_t, 3>();
        insertSingleEntryIntoEmptyUncompressed<default_double_Hypertrie_internal_t, 3>();
    }

};

#endif //HYPERTRIE_TESTNODE_HPP
