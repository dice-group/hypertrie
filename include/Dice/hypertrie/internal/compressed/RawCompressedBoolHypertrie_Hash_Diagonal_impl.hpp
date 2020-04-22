//
// Created by burhan on 30.01.20.
//

#ifndef HYPERTRIE_RAWCOMPRESSEDBOOLHYPERTRIE_HASH_DIAGONAL_IMPL_HPP
#define HYPERTRIE_RAWCOMPRESSEDBOOLHYPERTRIE_HASH_DIAGONAL_IMPL_HPP

#include "Dice/hypertrie/internal/compressed/RawCompressedBoolHypertrie_impl.hpp"
#include <numeric>


namespace hypertrie::internal::compressed {

    template<pos_type diag_depth, pos_type depth, typename key_part_type, template<typename, typename> typename map_type,
            template<typename> typename set_type>
    class RawCompressedBHTHashDiagonal<diag_depth, depth, true, key_part_type, map_type, set_type, std::enable_if_t<(
            depth == diag_depth and depth <= 2 and depth >= 1)>> {
        template<pos_type depth_>
        using CompressedNode = RawCompressedBoolHypertrie<depth_, key_part_type, map_type, set_type, true>;

    private:
        CompressedNode<depth> const &rawboolhypertrie;
        bool isEmpty;

    public:
        explicit RawCompressedBHTHashDiagonal(const CompressedNode<depth> *boolhypertrie) :
                rawboolhypertrie{*boolhypertrie} {}

        static void init(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            diag.isEmpty = false;
            if constexpr (depth == 2) {
                diag.isEmpty = !diag.rawboolhypertrie.diagonal(diag.rawboolhypertrie.currentKeyPart(0));
            }
        }

        static key_part_type currentKeyPart(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.rawboolhypertrie.currentKeyPart(0);
        }

        static bool contains(void *diag_ptr, key_part_type key_part) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.rawboolhypertrie.diagonal(key_part);
        }

        static void inc(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            diag.isEmpty = true;
        }

        static bool empty(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.isEmpty;
        }

        static size_t size(void const *diag_ptr) {
            return 1;
        }

    };

    template<pos_type diag_depth, pos_type depth, typename key_part_type, template<typename, typename> typename map_type,
            template<typename> typename set_type>
    class RawCompressedBHTHashDiagonal<diag_depth, depth, true, key_part_type, map_type, set_type, std::enable_if_t<(
            depth == 2 and diag_depth == 1)>> {
        template<pos_type depth_>
        using CompressedNode = RawCompressedBoolHypertrie<depth_, key_part_type, map_type, set_type, true>;
        using child_type = typename CompressedNode<depth>::child_type;

    public:
        using value_type = child_type;

    private:
        mutable CompressedNode<depth> const *rawboolhypertrie;
        std::vector<pos_type> diag_poss;
        value_type value;
        pos_type pos;
        bool isEmpty;

    public:
        RawCompressedBHTHashDiagonal(CompressedNode<depth> const *const boolhypertrie,
                                     std::vector<pos_type> positions)
                : rawboolhypertrie{boolhypertrie}, diag_poss{std::move(positions)} {}

        RawCompressedBHTHashDiagonal(CompressedNode<depth> const &boolhypertrie,
                                     const std::vector<pos_type> &positions)
                : rawboolhypertrie(&boolhypertrie, positions) {}

        /* RawCompressedBHTHashDiagonal(CompressedNode<depth> *const &boolhypertrie,
                                      const std::vector<pos_type> &positions)
                 : RawCompressedBHTHashDiagonal(boolhypertrie, positions) {}
 */
        static void init(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            diag.pos = *diag.diag_poss.begin();
            diag.value = diag.rawboolhypertrie->get(diag.pos,
                                                    diag.rawboolhypertrie->currentKeyPart(diag.pos));
            diag.isEmpty = false;
        }

        static key_part_type currentKeyPart(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.rawboolhypertrie->currentKeyPart(diag.pos);
        }

        static void *currentValue(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.value.getPointer();
        }

        static bool contains(void *diag_ptr, key_part_type key_part) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            diag.value = diag.rawboolhypertrie->template diagonal<diag_depth>(diag.diag_poss, key_part);
            return not diag.value.isEmpty();
        }

        static void inc(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            diag.isEmpty = true;
        }

        static bool empty(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.isEmpty;
        }

        static size_t size(void const *diag_ptr) {
            return 1;
        }
    };

    template<pos_type diag_depth, pos_type depth, typename key_part_type, template<typename, typename> typename map_type,
            template<typename> typename set_type>
    class RawCompressedBHTHashDiagonal<diag_depth, depth, false, key_part_type, map_type, set_type, std::enable_if_t<(
            depth == diag_depth and depth > 2)>> {

        // It happens when for one operand X we have all key part position resolve to the same label (a). Ex. X[a,a]
        template<pos_type depth_>
        using BHTNode = RawCompressedBoolHypertrie<depth_, key_part_type, map_type, set_type, false>;
        using compressed_children_type = typename BHTNode<depth>::compressed_children_type;
        using compressed_child_type = typename BHTNode<depth>::compressed_child_type;

        using children_type = typename BHTNode<depth>::children_type;
        using child_type = typename BHTNode<depth>::ChildNode;


    private:
        BHTNode<depth> const &rawboolhypertrie;
        typename compressed_children_type::const_iterator compressed_children_iter;
        typename compressed_children_type::const_iterator compressed_children_end;
        typename children_type::const_iterator iter;
        typename children_type::const_iterator end;
        bool inCompressedMode;

    public:
        explicit RawCompressedBHTHashDiagonal(const BHTNode<depth> *compressedboolhypertrie) :
                rawboolhypertrie{*compressedboolhypertrie} {}

        static void init(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            // Find the edges with least cardinality
            const auto min_card_pos = diag.rawboolhypertrie.minCardPos();
            const auto &min_dim_compressed_edges = diag.rawboolhypertrie.compressed_edges[min_card_pos];
            // Setup the iterators
            diag.compressed_children_iter = min_dim_compressed_edges.begin();
            diag.compressed_children_end = min_dim_compressed_edges.end();
            diag.iter = diag.rawboolhypertrie.edges[min_card_pos].begin();
            diag.end = diag.rawboolhypertrie.edges[min_card_pos].end();

            // Setup the processing mode flag
            if (diag.compressed_children_iter != diag.compressed_children_end) {
                // We start with the compressed mode
                diag.inCompressedMode = true;
            } else {
                // Even though, the non-compressed children map could also be empty
                diag.inCompressedMode = false;
            }

            if (not empty(diag_ptr) and not diagonal(diag_ptr)) {
                inc(diag_ptr);
            }
        }

        // Get the current key part when diagonal is in valid state only
        static key_part_type currentKeyPart(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if (diag.inCompressedMode) {
                return diag.compressed_children_iter->first;
            } else {
                return diag.iter->first;
            }
        }

        static bool contains(void *diag_ptr, key_part_type key_part) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            return diag.rawboolhypertrie.diagonal(key_part);
        }

        // This will handle the transition from compressed to non-compressed processing mode
        static void inc(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            assert(not empty(diag_ptr));
            do {
                if (diag.inCompressedMode and diag.compressed_children_iter != diag.compressed_children_end) {
                    diag.compressed_children_iter++;
                    if (diag.compressed_children_iter == diag.compressed_children_end) {
                        diag.inCompressedMode = false;
                    }
                } else {
                    diag.iter++;
                }
            } while (not empty(diag_ptr) and not diagonal(diag_ptr));
        }

        // There is nothing to iterate over anymore
        static bool empty(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if (not diag.inCompressedMode) {
                return diag.iter == diag.end;
            } else {
                // Although Diagonal in compressed mode 'empty' predicate applies for all states of diagonal (compressed and non-compressed)
                return diag.iter == diag.end and diag.compressed_children_iter == diag.compressed_children_end;
            }
        }

        static size_t size(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            const auto min_card_pos = diag.rawboolhypertrie.minCardPos();
            return diag.rawboolhypertrie.edges[min_card_pos].size()
                   + diag.rawboolhypertrie.compressed_edges[min_card_pos].size();
        }

    private:
        static bool compressed_diag(void const *diag_ptr) {
            auto const &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            compressed_child_type const &compressed_child = diag.compressed_children_iter->second;
            for (size_t i = 0; i < compressed_child.size(); i++) {
                if (compressed_child[i] != diag.compressed_children_iter->first) {
                    return false;
                }
            }
            return true;
        }

        static bool diagonal(void const *diag_ptr) {
            auto const &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if (diag.inCompressedMode) {
                return compressed_diag(diag_ptr);
            } else {
                return diag.iter->second->diagonal(diag.iter->first);
            }
        }
    };

    template<pos_type diag_depth, pos_type depth, typename key_part_type, template<typename, typename> typename map_type,
            template<typename> typename set_type>
    class RawCompressedBHTHashDiagonal<diag_depth, depth, false, key_part_type, map_type, set_type, std::enable_if_t<(
            depth == diag_depth and depth <= 2 and depth >= 1)>> {
        // It happens when for one operand X we have all key part position resolve to the same label (a). Ex. X[a,a]
        template<pos_type depth_>
        using Node = RawCompressedBoolHypertrie<depth_, key_part_type, map_type, set_type, false>;
        using children_type = typename Node<depth>::children_type;
        using compressed_child_type = typename Node<depth>::compressed_child_type;
        using child_type = typename Node<depth>::child_type;
    private:
        Node<depth> const &rawboolhypertrie;
        typename children_type::const_iterator iter;
        typename children_type::const_iterator end;

    public:
        explicit RawCompressedBHTHashDiagonal(const Node<depth> *boolhypertrie) :
                rawboolhypertrie{*boolhypertrie} {}

        static void init(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            if constexpr (depth > 1) {
                const auto min_card_pos = diag.rawboolhypertrie.minCardPos();
                const auto &min_dim_edges = diag.rawboolhypertrie.edges[min_card_pos];
                diag.iter = min_dim_edges.begin();
                diag.end = min_dim_edges.end();
                if (not empty(diag_ptr)) {
                    if (diag.iter->second.getTag() == compressed_child_type::INT_TAG) {
                        if (diag.iter->second.getInt() != diag.iter->first) {
                            inc(diag_ptr);
                        }
                    } else if (not diag.iter->second.getPointer()->diagonal(diag.iter->first)) {
                        inc(diag_ptr);
                    }
                }
            } else {
                diag.iter = diag.rawboolhypertrie.edges.begin();
                diag.end = diag.rawboolhypertrie.edges.end();
            }
        }

        static key_part_type currentKeyPart(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if constexpr (depth > 1)
                return diag.iter->first;
            else
                return *diag.iter;
        }

        static bool contains(void *diag_ptr, key_part_type key_part) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            return diag.rawboolhypertrie.diagonal(key_part);
        }

        static void inc(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            assert(not empty(diag_ptr));
            if constexpr (depth > 1) {
                do {
                    ++diag.iter;
                } while (not empty(diag_ptr) and not diagonal(diag_ptr));
            } else {
                ++diag.iter;
            }
        }

        static bool empty(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.iter == diag.end;
        }

        static size_t size(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if constexpr (depth > 1) {
                const auto min_card_pos = diag.rawboolhypertrie.minCardPos();
                return diag.rawboolhypertrie.edges[min_card_pos].size();
            } else {
                return diag.rawboolhypertrie.size();
            }
        }

    private:
        static bool diagonal(void const *diag_ptr) {
            auto const &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if (diag.iter->second.getTag() == compressed_child_type::INT_TAG) {
                return diag.iter->second.getInt() == diag.iter->first;
            } else {
                return diag.iter->second.getPointer()->diagonal(diag.iter->first);
            }
        }
    };

    // When depth != diag_depth
    template<pos_type diag_depth, pos_type depth, typename key_part_type, template<typename, typename> typename map_type,
            template<typename> typename set_type>
    class RawCompressedBHTHashDiagonal<diag_depth, depth, false, key_part_type, map_type, set_type, std::enable_if_t<(
            depth > diag_depth and depth > 2)>> {
        // It happens when for one operand X we have all key part position resolve to the same label (a). Ex. X[a,a]
        template<pos_type depth_>
        using Node = RawCompressedBoolHypertrie<depth_, key_part_type, map_type, set_type, false>;
        template<pos_type depth_>
        using CompressedNode = RawCompressedBoolHypertrie<depth_, key_part_type, map_type, set_type, true>;

        template<pos_type depth_t>
        using NodePointer = util::CompressedBoolHyperTrieTaggedPointer<CompressedNode<depth_t> *, Node<depth_t> *, 8>;

        using compressed_children_type = typename Node<depth>::compressed_children_type;
        using compressed_child_type = typename Node<depth>::compressed_child_type;

        using children_type = typename Node<depth>::children_type;
        using child_type = typename Node<depth>::ChildNode;
        using CurrentNodePointer = typename Node<depth>::template NodePointer<depth>;
    public:
        using value_type = NodePointer<depth - diag_depth>;

    private:
        static constexpr bool is_tsl_map = std::is_same_v<map_type<int, int>, container::tsl_sparse_map < int, int>>
        ;
        static constexpr bool is_tsl_set = std::is_same_v<set_type<int>, container::tsl_sparse_set < int>>
        ;
        /*
         * @TODO Do I need to make it "mutable" pointer
         */
        Node<depth> const *rawboolhypertrie;
        std::vector<pos_type> diag_poss;
        typename compressed_children_type::const_iterator compressed_children_iter;
        typename compressed_children_type::const_iterator compressed_children_end;
        typename children_type::const_iterator iter;
        typename children_type::const_iterator end;
        bool inCompressedMode;

        value_type value;

    public:
        RawCompressedBHTHashDiagonal(Node<depth> const *const boolhypertrie, std::vector<pos_type> positions)
                : rawboolhypertrie{boolhypertrie}, diag_poss{std::move(positions)} {}

        RawCompressedBHTHashDiagonal(CurrentNodePointer const &nodePointer, const std::vector<pos_type> &positions)
                : RawCompressedBHTHashDiagonal(nodePointer.getNode(), positions) {}

        static void init(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            // Find the edges with least cardinality
            const auto min_card_pos_it = diag.rawboolhypertrie->minCardPos(diag.diag_poss);
            const auto &min_dim_compressed_children = diag.rawboolhypertrie->compressed_edges[*min_card_pos_it];
            // Setup the iterators
            diag.compressed_children_iter = min_dim_compressed_children.begin();
            diag.compressed_children_end = min_dim_compressed_children.end();
            diag.iter = diag.rawboolhypertrie->edges[*min_card_pos_it].begin();
            diag.end = diag.rawboolhypertrie->edges[*min_card_pos_it].end();
            auto const min_card_pos = *min_card_pos_it;
            if constexpr (diag_depth > 1) {
                diag.diag_poss.erase(min_card_pos_it);
                auto posCalc = util::PosCalc::getInstance(depth)->use(min_card_pos);
                for (auto &pos : diag.diag_poss)
                    pos = posCalc->key_to_subkey_pos(pos);
            }
            if (diag.compressed_children_iter != diag.compressed_children_end) {
                diag.inCompressedMode = true;
            } else {
                diag.inCompressedMode = false;
            }
            if (not empty(diag_ptr)) {
                if constexpr (diag_depth > 1) {
                    if (diag.inCompressedMode) {
                        typename Node<depth>::compressed_child_type const &compressed_child_val = diag.compressed_children_iter->second;
                        CompressedNode<depth - 1> const *compressed_child = new CompressedNode<depth - 1>{
                                compressed_child_val};
                        diag.value = compressed_child->template diagonal<diag_depth - 1>(diag.diag_poss,
                                                                                         diag.compressed_children_iter->first);
                    } else {
                        diag.value = diag.iter->second->template diagonal<diag_depth - 1>(diag.diag_poss,
                                                                                          diag.iter->first);
                    }
                    if (diag.value.isEmpty()) {
                        inc(diag_ptr);
                    }
                } else {
                    if (diag.inCompressedMode) {
                        diag.value = value_type{new CompressedNode<depth - 1>{diag.compressed_children_iter->second}};
                    } else {
                        if constexpr (is_tsl_map) {
                            diag.value = value_type{diag.iter.value()};
                        } else {
                            diag.value = value_type{diag.iter->second};
                        }
                    }
                }
            }
        }

        static key_part_type currentKeyPart(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if (diag.inCompressedMode) {
                return diag.compressed_children_iter->first;
            } else {
                return diag.iter->first;
            }
        }

        static void *currentValue(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.value.getPointer();
        }

        static bool contains(void *diag_ptr, key_part_type key_part) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            diag.value = diag.rawboolhypertrie->template diagonal<diag_depth>(diag.diag_poss, key_part);
            return !diag.value.isEmpty();
        }

        static void inc(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            assert(not empty(diag_ptr));
            do {
                if (diag.inCompressedMode) {
                    diag.compressed_children_iter++;
                    if (diag.compressed_children_iter == diag.compressed_children_end) {
                        diag.inCompressedMode = false;
                        continue;
                    }
                } else {
                    diag.iter++;
                }
            } while (not empty(diag_ptr) and not diagonal(diag_ptr));
        }

        // There is nothing to iterate over anymore
        static bool empty(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if (not diag.inCompressedMode) {
                return diag.iter == diag.end;
            } else {
                // Although Diagonal in compressed mode 'empty' predicate applies for all states of diagonal (compressed and non-compressed)
                return diag.iter == diag.end and diag.compressed_children_iter == diag.compressed_children_end;
            }
        }

        static size_t size(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            const auto min_card_pos = diag.rawboolhypertrie->minCardPos();
            return diag.rawboolhypertrie->edges[min_card_pos].size()
                   + diag.rawboolhypertrie->compressed_edges[min_card_pos].size();
        }

    private:
        static bool diagonal(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            if constexpr (diag_depth > 1) {
                if (diag.inCompressedMode) {
                    typename Node<depth>::compressed_child_type const &compressed_child_val = diag.compressed_children_iter->second;
                    CompressedNode<depth - 1> const *compressed_child = new CompressedNode<depth - 1>{
                            compressed_child_val};
                    diag.value = compressed_child->template diagonal<diag_depth - 1>(diag.diag_poss,
                                                                                     diag.compressed_children_iter->first);
                } else {
                    diag.value = diag.iter->second->template diagonal<diag_depth - 1>(diag.diag_poss,
                                                                                      diag.iter->first);
                }
                return not diag.value.isEmpty();
            } else {
                if (diag.inCompressedMode) {
                    diag.value = value_type{new CompressedNode<depth - 1>{diag.compressed_children_iter->second}};
                } else {
                    if constexpr (is_tsl_map) {
                        diag.value = value_type{diag.iter.value()};
                    } else {
                        diag.value = value_type{diag.iter->second};
                    }
                }
                return true;
            }
        }

    };

    // When depth != diag_depth
    template<pos_type diag_depth, pos_type depth, typename key_part_type, template<typename, typename> typename map_type,
            template<typename> typename set_type>
    class RawCompressedBHTHashDiagonal<diag_depth, depth, false, key_part_type, map_type, set_type, std::enable_if_t<(
            depth == 2 and diag_depth == 1)>> {
        template<pos_type depth_>
        using BHTNode = RawCompressedBoolHypertrie<depth_, key_part_type, map_type, set_type, false>;
        using children_type = typename BHTNode<depth>::children_type;
        using compressed_child_type = typename BHTNode<depth>::compressed_child_type;
        using child_type = typename BHTNode<depth>::child_type;
    public:
        using value_type = child_type;

    private:
        mutable BHTNode<depth> const *rawCompressedBoolhypertrie;
        std::vector<pos_type> diag_poss;
        typename children_type::const_iterator iter;
        typename children_type::const_iterator end;
        value_type value;

    public:
        RawCompressedBHTHashDiagonal(BHTNode<depth> const *const boolhypertrie, std::vector<pos_type> positions)
                : rawCompressedBoolhypertrie{boolhypertrie}, diag_poss{std::move(positions)} {}

        RawCompressedBHTHashDiagonal(BHTNode<depth> const &boolhypertrie, const std::vector<pos_type> &positions)
                : rawCompressedBoolhypertrie(&boolhypertrie, positions) {}

        /*RawCompressedBHTHashDiagonal(BHTNode<depth> *const &boolhypertrie, const std::vector<pos_type> &positions)
                : RawCompressedBHTHashDiagonal(boolhypertrie, positions) {}*/

        static void init(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            auto min_card_pos_it = diag.rawCompressedBoolhypertrie->minCardPos(diag.diag_poss);
            auto &min_dim_edges = diag.rawCompressedBoolhypertrie->edges[*min_card_pos_it];
            diag.iter = min_dim_edges.begin();
            diag.end = min_dim_edges.end();
            if (not empty(diag_ptr)) {
                diag.value = diag.rawCompressedBoolhypertrie->get(*min_card_pos_it, diag.iter->first);
            }
        }

        static key_part_type currentKeyPart(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.iter->first;
        }

        static void *currentValue(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.value.getPointer();
        }

        static bool contains(void *diag_ptr, key_part_type key_part) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            diag.value = diag.rawCompressedBoolhypertrie->template diagonal<diag_depth>(diag.diag_poss, key_part);
            return not diag.value.isEmpty();
        }

        static void inc(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            auto min_card_pos_it = diag.rawCompressedBoolhypertrie->minCardPos(diag.diag_poss);
            ++diag.iter;
            if (empty(diag_ptr)) return;
            diag.value = diag.rawCompressedBoolhypertrie->get(*min_card_pos_it, diag.iter->first);
        }

        static bool empty(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.iter == diag.end;
        }

        static size_t size(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if constexpr (depth > 1) {
                const auto min_card_pos_it = diag.rawCompressedBoolhypertrie->minCardPos(diag.diag_poss);
                return diag.rawCompressedBoolhypertrie->edges[*min_card_pos_it].size();
            } else {
                return diag.rawCompressedBoolhypertrie->size();
            }
        }
    };
}
#endif //HYPERTRIE_RAWCOMPRESSEDBOOLHYPERTRIE_HASH_DIAGONAL_IMPL_HPP
