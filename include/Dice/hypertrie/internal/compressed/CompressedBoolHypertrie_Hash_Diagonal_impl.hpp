//
// Created by root on 4/5/20.
//

#ifndef HYPERTRIE_COMPRESSEDBOOLHYPERTRIE_HASH_DIAGONAL_IMPL_HPP
#define HYPERTRIE_COMPRESSEDBOOLHYPERTRIE_HASH_DIAGONAL_IMPL_HPP

#include "Dice/hypertrie/internal/compressed/CompressedBoolHypertrie_impl.hpp"


namespace hypertrie::internal::compressed {
    template<typename key_part_type, template<typename, typename> class map_type,
            template<typename> class set_type>
    class CompressedHashDiagonal {
        using const_CompressedBoolHypertrie = hypertrie::internal::compressed::const_CompressedBoolHypertrie<key_part_type, map_type, set_type>;

        template<pos_type depth>
        using NodePointer = typename const_CompressedBoolHypertrie::template NodePointer<depth>;

        template<pos_type depth, bool compressed>
        using RawCompressedBoolHypertrie = typename const_CompressedBoolHypertrie::template RawCompressedBoolHypertrie<depth, compressed>;
        using Key = typename const_CompressedBoolHypertrie::Key;
        using SliceKey = typename const_CompressedBoolHypertrie::SliceKey;
        template<pos_type depth, pos_type diag_depth, bool compressed>
        using RawCompressedHashDiagonal = typename hypertrie::internal::compressed::interface::rawcompressedboolhypertrie<key_part_type, map_type, set_type>::template RawCompressedBHTHashDiagonal<depth, diag_depth, compressed>;


        struct RawDiagFunctions {
            void (*init)(void *);

            key_part_type (*currentKeyPart)(void const *);

            void *(*currentValue)(void const *);

            bool (*contains)(void *, key_part_type);

            void (*inc)(void *);

            bool (*empty)(void const *);

            size_t (*size)(void const *);
        };

        template<pos_type diag_depth_, pos_type depth, bool compressed>
        static auto call_currentValue([[maybe_unused]]void const *diag_ptr) -> void * {
            if constexpr (depth > diag_depth_) {
                return RawCompressedHashDiagonal<diag_depth_, depth, compressed>::currentValue(diag_ptr);
            } else {
                throw std::invalid_argument{"currentValue is only implemented for depth > diag_depth"};
            }
        }

        template<pos_type depth, pos_type diag_depth_, bool compressed>
        inline static RawDiagFunctions getRawDiagFunctions() {
            return RawDiagFunctions{
                    &RawCompressedHashDiagonal<diag_depth_, depth, compressed>::init,
                    &RawCompressedHashDiagonal<diag_depth_, depth, compressed>::currentKeyPart,
                    &call_currentValue<diag_depth_, depth, compressed>,
                    &RawCompressedHashDiagonal<diag_depth_, depth, compressed>::contains,
                    &RawCompressedHashDiagonal<diag_depth_, depth, compressed>::inc,
                    &RawCompressedHashDiagonal<diag_depth_, depth, compressed>::empty,
                    &RawCompressedHashDiagonal<diag_depth_, depth, compressed>::size
            };
        }

        inline static std::vector<std::vector<RawDiagFunctions>> functions{
                {
                        getRawDiagFunctions<1, 1, false>()
                },
                {
                        getRawDiagFunctions<2, 1, false>(),
                        getRawDiagFunctions<2, 2, false>()
                },
                {
                        getRawDiagFunctions<3, 1, false>(),
                        getRawDiagFunctions<3, 2, false>(),
                        getRawDiagFunctions<3, 3, false>()
                },
        };

        inline static std::vector<std::vector<RawDiagFunctions>> compressed_functions{
                {
                        getRawDiagFunctions<1, 1, true>()
                },
                {
                        getRawDiagFunctions<2, 1, true>(),
                        getRawDiagFunctions<2, 2, true>()
                },
        };

    public:
        using poss_type = std::vector<pos_type>;
    private:

        std::shared_ptr<void> raw_diag;
        RawDiagFunctions *raw_diag_funcs;

        template<pos_type diag_depth_, pos_type depth, bool compressed>
        static inline std::shared_ptr<void>
        getRawDiagonal(const const_CompressedBoolHypertrie &CBHT, [[maybe_unused]]const poss_type &positions) {
            NodePointer<depth> node_ptr{CBHT.hypertrie};
            if constexpr (depth == diag_depth_) {
                if constexpr (compressed) {
                    return std::make_shared<RawCompressedHashDiagonal<diag_depth_, depth, true>>(
                            node_ptr.getCompressedNode());
                } else {
                    return std::make_shared<RawCompressedHashDiagonal<diag_depth_, depth, false>>(node_ptr.getNode());
                }
            } else {
                if constexpr(compressed) {
                    return std::make_shared<RawCompressedHashDiagonal<diag_depth_, depth, true>>(
                            node_ptr.getCompressedNode(),
                            positions);
                } else {
                    return std::make_shared<RawCompressedHashDiagonal<diag_depth_, depth, false>>(node_ptr.getNode(),
                                                                                                  positions);
                }
            }
        }

        static inline std::shared_ptr<void>
        getRawDiagonal(const const_CompressedBoolHypertrie &boolhypertrie, const poss_type &positions,
                       bool const compressed) {
            switch (boolhypertrie.depth()) {
                case 1: {
                    if (compressed) {
                        return getRawDiagonal<1, 1, true>(boolhypertrie, positions);
                    } else {
                        return getRawDiagonal<1, 1, false>(boolhypertrie, positions);
                    }
                }
                case 2: {
                    if (compressed) {
                        switch (positions.size()) {
                            case 1: {
                                return getRawDiagonal<1, 2, true>(boolhypertrie, positions);
                            }
                            case 2: {
                                return getRawDiagonal<2, 2, true>(boolhypertrie, positions);
                            }
                            default:
                                break;
                        }
                    } else {
                        switch (positions.size()) {
                            case 1: {
                                return getRawDiagonal<1, 2, false>(boolhypertrie, positions);
                            }
                            case 2: {
                                return getRawDiagonal<2, 2, false>(boolhypertrie, positions);
                            }
                            default:
                                break;
                        }
                    }
                    break;
                }
                case 3: {
                    switch (positions.size()) {
                        case 1: {
                            return getRawDiagonal<1, 3, false>(boolhypertrie, positions);
                        }
                        case 2: {
                            return getRawDiagonal<2, 3, false>(boolhypertrie, positions);
                        }
                        case 3: {
                            return getRawDiagonal<3, 3, false>(boolhypertrie, positions);
                        }
                        default:
                            break;
                    }
                    break;
                }
                default:
                    break;
            }

        }

    public:

        CompressedHashDiagonal() = default;

        CompressedHashDiagonal(CompressedHashDiagonal &) = default;

        CompressedHashDiagonal(const CompressedHashDiagonal &) = default;

        CompressedHashDiagonal(CompressedHashDiagonal &&) noexcept = default;

        CompressedHashDiagonal &operator=(CompressedHashDiagonal &&) noexcept = default;

        CompressedHashDiagonal &operator=(const CompressedHashDiagonal &) = default;


        CompressedHashDiagonal(const_CompressedBoolHypertrie const *const boolhypertrie, const poss_type &positions) {

            switch (boolhypertrie->depth()) {
                case 1: {
                    NodePointer<1> node_ptr{boolhypertrie->hypertrie};
                    if (node_ptr.getTag() == NodePointer<1>::COMPRESSED_TAG) {
                        this->raw_diag_funcs = &compressed_functions[boolhypertrie->depth() - 1][positions.size() - 1];
                        this->raw_diag = getRawDiagonal(*boolhypertrie, positions, true);
                    } else {
                        this->raw_diag_funcs = &functions[boolhypertrie->depth() - 1][positions.size() - 1];
                        this->raw_diag = getRawDiagonal(*boolhypertrie, positions, false);
                    }
                    break;
                }
                case 2: {
                    NodePointer<2> node_ptr{boolhypertrie->hypertrie};
                    if (node_ptr.getTag() == NodePointer<2>::COMPRESSED_TAG) {
                        this->raw_diag_funcs = &compressed_functions[boolhypertrie->depth() - 1][positions.size() - 1];
                        this->raw_diag = getRawDiagonal(*boolhypertrie, positions, true);
                    } else {
                        this->raw_diag_funcs = &functions[boolhypertrie->depth() - 1][positions.size() - 1];
                        this->raw_diag = getRawDiagonal(*boolhypertrie, positions, false);
                    }
                    break;
                }
                case 3: {
                    this->raw_diag_funcs = &functions[boolhypertrie->depth() - 1][positions.size() - 1];
                    this->raw_diag = getRawDiagonal(*boolhypertrie, positions, false);
                    break;
                }
                default:
                    throw std::logic_error{"not implemented."};
            }
        }

        CompressedHashDiagonal(const const_CompressedBoolHypertrie &boolhypertrie, const poss_type &positions) :
                CompressedHashDiagonal(&boolhypertrie, positions) {}


        /*
        * Potentially forwards the Diagonal and leafs it in a safe state. <br/>
        * It checks if the current key_part is valid and increments it until it is valid.
        */
        void init() const { // #
            raw_diag_funcs->init(raw_diag.get());
        }

        /*
        * Must only be called in a safe state. <br/>
        * Returns the current value.
        */
        [[nodiscard]]
        key_part_type currentKeyPart() const { // #
            return raw_diag_funcs->currentKeyPart(raw_diag.get());
        }

        [[nodiscard]]
        void *currentValue() const {
            return raw_diag_funcs->currentValue(raw_diag.get());
        }

        /**
         * use only if the diagonal is calculated over all dimensions.
         * @param key_part
         * @return
         */
        [[nodiscard]]
        bool contains(key_part_type key_part) const {
            return raw_diag_funcs->contains(raw_diag.get(), key_part);
        }

        /*
        * Forwards the Diagonal and leafs it in a safe state. <br/>
        * Increments the diagonal to the next valid key_part.
        */
        void operator++() { // #
            return raw_diag_funcs->inc(raw_diag.get());
        }

        /*
        * If it returns true there are no key_parts left for sure.
        * Otherwise there are potential key_parts left and therefore, there may also be valid key_parts left.
        * @return
        */
        [[nodiscard]]
        bool empty() const { // #
            return raw_diag_funcs->empty(raw_diag.get());
        }

        /*
        * Always safe. <br/>
        * Get the number of potential key_parts. This is a upper bound to the valid key_parts.
        * @return number of potential key_parts
        */
        [[nodiscard]]
        size_t size() const {
            return raw_diag_funcs->size(raw_diag.get());
        }

        bool operator<(const CompressedHashDiagonal &other) const {
            return this->size() < other.size();
        }
    };


}


#endif //HYPERTRIE_COMPRESSEDBOOLHYPERTRIE_HASH_DIAGONAL_IMPL_HPP
