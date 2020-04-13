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
        using BHTCompressedNode = RawCompressedBoolHypertrie<depth_, key_part_type, map_type, set_type, true>;

    private:
        BHTCompressedNode<depth> const &rawCompressedboolhypertrie;
        bool isEmpty;

    public:
        explicit RawCompressedBHTHashDiagonal(const BHTCompressedNode<depth> &boolhypertrie) :
                rawCompressedboolhypertrie{boolhypertrie} {}

        static void init(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            diag.isEmpty = !diag.rawCompressedboolhypertrie.diagonal(diag.rawCompressedboolhypertrie.currentKeyPart(0));
        }

        static key_part_type currentKeyPart(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.rawCompressedboolhypertrie.currentKeyPart(0);
        }

        static bool contains(void *diag_ptr, key_part_type key_part) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            return diag.rawCompressedboolhypertrie.diagonal(key_part);
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
        using BHTCompressedNode = RawCompressedBoolHypertrie<depth_, key_part_type, map_type, set_type, true>;
        using child_type = typename BHTCompressedNode<depth>::child_type;

    public:
        using value_type = child_type;

    private:
        mutable BHTCompressedNode<depth> const *rawCompressedBoolhypertrie;
        std::vector<pos_type> diag_poss;
        value_type value;
        bool isEmpty;

    public:
        RawCompressedBHTHashDiagonal(BHTCompressedNode<depth> const *const boolhypertrie,
                                     std::vector<pos_type> positions)
                : rawCompressedBoolhypertrie{boolhypertrie}, diag_poss{std::move(positions)} {}

        RawCompressedBHTHashDiagonal(BHTCompressedNode<depth> const &boolhypertrie,
                                     const std::vector<pos_type> &positions)
                : rawCompressedBoolhypertrie(&boolhypertrie, positions) {}

        RawCompressedBHTHashDiagonal(BHTCompressedNode<depth> *const &boolhypertrie,
                                     const std::vector<pos_type> &positions)
                : RawCompressedBHTHashDiagonal(boolhypertrie, positions) {}

        static void init(void *diag_ptr) {
            auto const &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            pos_type pos = *diag.diag_poss.begin();
            diag.value = diag.rawCompressedBoolhypertrie->get(pos, diag.rawCompressedBoolhypertrie->currentKeyPart(pos));
            diag.isEmpty = false;
        }

        static key_part_type currentKeyPart(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.rawCompressedBoolhypertrie->currentKeyPart(*diag.diag_poss.begin());
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
        BHTNode<depth> const &rawCompressedboolhypertrie;
        typename compressed_children_type::const_iterator compressed_children_iter;
        typename compressed_children_type::const_iterator compressed_children_end;
        typename children_type::const_iterator iter;
        typename children_type::const_iterator end;
        bool inCompressedMode;

    public:
        explicit RawCompressedBHTHashDiagonal(const BHTNode<depth> &compressedboolhypertrie) :
                rawCompressedboolhypertrie{compressedboolhypertrie} {}

        static void init(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            // Find the edges with least cardinality
            const auto min_card_pos = diag.rawCompressedboolhypertrie.minCardPos();
            const auto &min_dim_compressed_edges = diag.rawCompressedboolHypertrie.compressed_edges[min_card_pos];
            // Setup the iterators
            diag.compressed_children_iter = min_dim_compressed_edges.begin();
            diag.compressed_children_end = min_dim_compressed_edges.end();
            diag.iter = diag.rawCompressedboolHypertrie.edges[min_card_pos].begin();
            diag.end = diag.rawCompressedboolHypertrie.edges[min_card_pos].end();

            // Setup the processing mode flag
            if (diag.compressed_children_iter != diag.compressed_children_end) {
                // We start with the compressed mode
                diag.inCompressedMode = true;
            } else {
                // Even though, the non-compressed children map could also be empty
                diag.inCompressedMode = false;
            }

            if (not empty(diag_ptr)) {
                if (diag.inCompressedMode) {
                    if (not compressed_diag(diag_ptr)) {
                        inc(diag_ptr);
                    }
                } else {
                    if (not diag.iter->second->diagonal(diag.iter->first)) {
                        inc(diag_ptr);
                    }
                }
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
            return diag.rawCompressedboolHypertrie.diagonal(key_part);
        }

        // This will handle the transition from compressed to non-compressed processing mode
        static void inc(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            assert(not empty(diag_ptr));
            do {
                if (diag.inCompressedMode and diag.compressed_children_iter != diag.compressed_children_end) {
                    diag.compressed_children_iter++;
                    if (diag.compressed_children_iter == diag.compressed_children_end) {
                        continue;
                    }
                } else {
                    diag.inCompressedMode = false;
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
            const auto min_card_pos = diag.rawCompressedboolhypertrie.minCardPos();
            return diag.rawCompressedboolhypertrie.edges[min_card_pos].size()
                   + diag.rawCompressedboolhypertrie.compressed_edges[min_card_pos].size();
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
        using BHTNode = RawCompressedBoolHypertrie<depth_, key_part_type, map_type, set_type, false>;
        using children_type = typename BHTNode<depth>::children_type;
        using compressed_child_type = typename BHTNode<depth>::compressed_child_type;
        using child_type = typename BHTNode<depth>::child_type;
    private:
        BHTNode<depth> const &rawCompressedboolhypertrie;
        typename children_type::const_iterator iter;
        typename children_type::const_iterator end;

    public:
        explicit RawCompressedBHTHashDiagonal(const BHTNode<depth> &boolhypertrie) :
                rawCompressedboolhypertrie{boolhypertrie} {}

        static void init(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            if constexpr (depth > 1) {
                const auto min_card_pos = diag.rawCompressedboolhypertrie.minCardPos();
                const auto &min_dim_edges = diag.rawCompressedboolhypertrie.edges[min_card_pos];
                diag.iter = min_dim_edges.begin();
                diag.end = min_dim_edges.end();
                if (not empty(diag_ptr)) {
                    if (diag.iter->second.getTag() == compressed_child_type::INT_TAG and
                        diag.iter->second.getInt() != diag.iter->first) {
                        inc(diag_ptr);
                    } else if (not diag.iter->second.getPointer()->diagonal(diag.iter->first)) {
                        inc(diag_ptr);
                    }
                }
            } else {
                diag.iter = diag.rawCompressedboolhypertrie.edges.begin();
                diag.end = diag.rawCompressedboolhypertrie.edges.end();
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
            return diag.rawCompressedboolhypertrie.diagonal(key_part);
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
            if (diag.iter->second.getTag() == compressed_child_type::INT_TAG and
                diag.iter->second.getInt() == diag.iter->first) {
                return true;
            } else if (diag.iter->second.getPointer()->diagonal(diag.iter->first)) {
                return true;
            } else return false;
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

        using compressed_children_type = typename Node<depth>::compressed_children_type;
        using compressed_child_type = typename Node<depth>::compressed_child_type;

        using children_type = typename Node<depth>::children_type;
        using child_type = typename Node<depth>::ChildNode;
        using CurrentNodePointer = typename Node<depth>::template NodePointer<depth>;
    public:
        using value_type = typename Node<depth>::template NodePointer<depth - diag_depth>;

    private:
        /*
         * @TODO Do I need to make it "mutable" pointer
         */
        Node<depth> const &rawCompressedboolhypertrie;
        std::vector<pos_type> diag_poss;
        typename compressed_children_type::const_iterator compressed_children_iter;
        typename compressed_children_type::const_iterator compressed_children_end;
        typename children_type::const_iterator iter;
        typename children_type::const_iterator end;
        bool inCompressedMode;

        value_type value;

    public:
        RawCompressedBHTHashDiagonal(Node<depth> const *const compressedBoolHypertrie, std::vector<pos_type> positions)
                : rawCompressedboolhypertrie{&compressedBoolHypertrie}, diag_poss{std::move(positions)} {}

        RawCompressedBHTHashDiagonal(Node<depth> const &compressedBoolHypertrie, const std::vector<pos_type> &positions)
                : rawCompressedboolhypertrie(compressedBoolHypertrie, positions) {}

        RawCompressedBHTHashDiagonal(CurrentNodePointer const &nodePointer, const std::vector<pos_type> &positions)
                : RawCompressedBHTHashDiagonal(nodePointer.getNode(), positions) {}

        static void init(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            // Find the edges with least cardinality
            const auto min_card_pos_it = diag.rawCompressedboolhypertrie.minCardPos(diag.diag_poss);
            const auto &min_dim_compressed_children = diag.rawCompressedboolHypertrie.compressed_edges[*min_card_pos_it];
            // Setup the iterators
            diag.compressed_children_iter = min_dim_compressed_children.begin();
            diag.compressed_children_end = min_dim_compressed_children.end();
            diag.iter = diag.rawCompressedboolHypertrie.edges[*min_card_pos_it].begin();
            diag.end = diag.rawCompressedboolHypertrie.edges[*min_card_pos_it].end();
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
                    key_part_type const key_part = diag.get_key_part();
                    if (diag.inCompressedMode) {
                        auto const compressed_child = new CompressedNode<depth - 1>{
                                diag.compressed_children_iter->second};
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
                    key_part_type const key_part = diag.get_key_part();
                    if (diag.inCompressedMode) {
                        diag.value = value_type{new CompressedNode<depth - 1>{diag.compressed_children_iter->second}};
                    } else {
                        diag.value = value_type{diag.iter->second};
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
            diag.value = (&diag.rawCompressedboolhypertrie)->template diagonal<diag_depth>(diag.diag_poss, key_part);
            return !diag.value.isEmpty();
        }

        static void inc(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
            assert(not empty(diag_ptr));
            do {
                if (diag.inCompressedMode and diag.compressed_children_iter != diag.compressed_children_end) {
                    diag.compressed_children_iter++;
                    if (diag.compressed_children_iter == diag.compressed_children_end) {
                        continue;
                    }
                } else {
                    diag.inCompressedMode = false;
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
            const auto min_card_pos = diag.rawCompressedboolhypertrie.minCardPos();
            return diag.rawCompressedboolhypertrie.edges[min_card_pos].size()
                   + diag.rawCompressedboolhypertrie.compressed_edges[min_card_pos].size();
        }

    private:
        static bool diagonal(void *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if constexpr (diag_depth > 1) {
                key_part_type const key_part = diag.get_key_part();
                if (diag.inCompressedMode) {
                    auto const compressed_child = new CompressedNode<depth - 1>{
                            diag.compressed_children_iter->second};
                    diag.value = compressed_child->template diagonal<diag_depth - 1>(diag.diag_poss,
                                                                                     diag.compressed_children_iter->first);
                } else {
                    diag.value = diag.iter->second->template diagonal<diag_depth - 1>(diag.diag_poss,
                                                                                      diag.iter->first);
                }
                return not diag.value.isEmpty();
            } else {
                key_part_type const key_part = diag.get_key_part();
                if (diag.inCompressedMode) {
                    diag.value = value_type{new CompressedNode<depth - 1>{diag.compressed_children_iter->second}};
                } else {
                    diag.value = value_type{diag.iter->second};
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
        typename children_type::iterator iter;
        typename children_type::iterator end;
        value_type value;

    public:
        RawCompressedBHTHashDiagonal(BHTNode<depth> const *const boolhypertrie, std::vector<pos_type> positions)
                : rawCompressedBoolhypertrie{boolhypertrie}, diag_poss{std::move(positions)} {}

        RawCompressedBHTHashDiagonal(BHTNode<depth> const &boolhypertrie, const std::vector<pos_type> &positions)
                : rawCompressedBoolhypertrie(&boolhypertrie, positions) {}

        RawCompressedBHTHashDiagonal(BHTNode<depth> *const &boolhypertrie, const std::vector<pos_type> &positions)
                : RawCompressedBHTHashDiagonal(boolhypertrie, positions) {}

        static void init(void *diag_ptr) {
            auto const &diag = *static_cast<RawCompressedBHTHashDiagonal *>(diag_ptr);
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
            diag.value = diag.rawCompressedBoolhypertrie->get(min_card_pos_it, diag.iter->first);
        }

        static bool empty(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            return diag.iter == diag.end;
        }

        static size_t size(void const *diag_ptr) {
            auto &diag = *static_cast<RawCompressedBHTHashDiagonal const *>(diag_ptr);
            if constexpr (depth > 1) {
                const auto min_card_pos = diag.rawboolhypertrie->minCardPos(diag.diag_poss);
                return diag.rawCompressedBoolhypertrie->edges[min_card_pos].size();
            } else {
                return diag.rawCompressedBoolhypertrie->size();
            }
        }
    };
}
#endif //HYPERTRIE_RAWCOMPRESSEDBOOLHYPERTRIE_HASH_DIAGONAL_IMPL_HPP
