//
// Created by root on 4/5/20.
//

#ifndef HYPERTRIE_COMPRESSEDBOOLHYPERTRIE_IMPL_HPP
#define HYPERTRIE_COMPRESSEDBOOLHYPERTRIE_IMPL_HPP

#include "Dice/hypertrie/internal/compressed/RawCompressedBoolHypertrie.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <functional>
#include <memory>

namespace hypertrie::internal::compressed {
    template<typename key_part_type, template<typename, typename> class map_type,
            template<typename> class set_type>
    class CompressedHashDiagonal;

    template<typename key_part_type, template<typename, typename> class map_type,
            template<typename> class set_type>
    class CompressedBoolHypertrie;

    template<typename key_part_type, template<typename, typename> class map_type,
            template<typename> class set_type>
    class const_CompressedBoolHypertrie {
    protected:
        template<pos_type depth, bool compressed>
        using RawCompressedBoolHypertrie = typename hypertrie::internal::compressed::interface::rawcompressedboolhypertrie<key_part_type, map_type, set_type>::template RawCompressedBoolHypertrie<depth, compressed>;
        template<pos_type depth, pos_type diag_depth>
        using RawCompressedBHTHashDiagonal = typename hypertrie::internal::compressed::interface::rawcompressedboolhypertrie<key_part_type, map_type, set_type>::template RawCompressedBHTHashDiagonal<diag_depth, depth>;

        template<pos_type depth_t>
        using Node = RawCompressedBoolHypertrie<depth_t, false>;

        template<pos_type depth_t>
        using CompressedNode = RawCompressedBoolHypertrie<depth_t, true>;

        template<pos_type depth_t>
        using NodePointer = util::CompressedBoolHyperTrieTaggedPointer<CompressedNode<depth_t> *, Node<depth_t> *, 8>;

    public:
        typedef std::vector<std::optional<key_part_type>> SliceKey;
        typedef std::vector<key_part_type> Key;

    protected:

        pos_type depth_ = 0;

        void *hypertrie;

        static void *get_hypertrie(pos_type depth) {
            switch (depth) {
                case 1: {
                    // We call this first so we tag the pointer properly
                    NodePointer<1> node_ptr{new Node<1>{}};
                    return node_ptr.getPointer();
                }
                case 2: {
                    // We call this first so we tag the pointer properly
                    NodePointer<2> node_ptr{new Node<2>{}};
                    return node_ptr.getPointer();
                }
                case 3: {
                    // We call this first so we tag the pointer properly
                    NodePointer<3> node_ptr{new Node<3>{}};
                    return node_ptr.getPointer();
                }
                default:
                    throw std::logic_error{"not implemented."};
            }
        }

    public:
        const_CompressedBoolHypertrie() = default;

        const_CompressedBoolHypertrie(const_CompressedBoolHypertrie &) = default;

        const_CompressedBoolHypertrie(const const_CompressedBoolHypertrie &) = default;

        const_CompressedBoolHypertrie(const_CompressedBoolHypertrie &&) noexcept = default;

        const_CompressedBoolHypertrie &
        operator=(const CompressedBoolHypertrie<key_part_type, map_type, set_type> &other) {
            this->depth_ = other.depth_;
            this->hypertrie = other.hypertrie;
            return *this;
        }

        const_CompressedBoolHypertrie &operator=(const const_CompressedBoolHypertrie &other) = default;

        const_CompressedBoolHypertrie &operator=(CompressedBoolHypertrie<key_part_type, map_type, set_type> other) {
            this->depth_ = other.depth_;
            this->hypertrie = other.hypertrie;
            return *this;
        }

        explicit const_CompressedBoolHypertrie(pos_type depth) : depth_(depth), hypertrie(get_hypertrie(depth)) {}

        const_CompressedBoolHypertrie &operator=(const_CompressedBoolHypertrie &&other) noexcept = default;

        template<pos_type depth_t>
        inline static const_CompressedBoolHypertrie instance(pos_type depth, NodePointer<depth_t> nodePointer) {
            return const_CompressedBoolHypertrie(depth, nodePointer.getPointer());
        }

    protected:
        explicit const_CompressedBoolHypertrie(pos_type depth, void *boolhypertrie)
                : depth_(depth), hypertrie(boolhypertrie) {}

    public:
        [[nodiscard]]
        std::vector<size_t> getCards(const std::vector<pos_type> &positions) const {
            assert(positions.size() <= depth());
            switch (depth_) {
                case 1: {
                    NodePointer<1> node_ptr = NodePointer<1>{hypertrie};
                    if (node_ptr.getTag() == NodePointer<1>::COMPRESSED_TAG) {
                        return node_ptr.getCompressedNode()->getCards(positions);
                    } else {
                        return node_ptr.getNode()->getCards(positions);
                    }
                }
                case 2: {
                    NodePointer<2> node_ptr = NodePointer<2>{hypertrie};
                    if (node_ptr.getTag() == NodePointer<2>::COMPRESSED_TAG) {
                        return node_ptr.getCompressedNode()->getCards(positions);
                    } else {
                        return node_ptr.getNode()->getCards(positions);
                    }
                }
                case 3: {
                    NodePointer<3> node_ptr = NodePointer<3>{hypertrie};
                    if (node_ptr.getTag() == NodePointer<3>::COMPRESSED_TAG) {
                        return node_ptr.getCompressedNode()->getCards(positions);
                    } else {
                        return node_ptr.getNode()->getCards(positions);
                    }
                }
                default:
                    throw std::logic_error{"not implemented."};
            }

        }

        [[nodiscard]]
        size_t size() const {
            switch (depth_) {
                case 1: {
                    NodePointer<1> node_ptr = NodePointer<1>{hypertrie};
                    if (node_ptr.getTag() == NodePointer<1>::COMPRESSED_TAG) {
                        return node_ptr.getCompressedNode()->size();
                    } else {
                        return node_ptr.getNode()->size();
                    }
                }
                case 2: {
                    NodePointer<2> node_ptr = NodePointer<2>{hypertrie};
                    if (node_ptr.getTag() == NodePointer<2>::COMPRESSED_TAG) {
                        return node_ptr.getCompressedNode()->size();
                    } else {
                        return node_ptr.getNode()->size();
                    }
                }
                case 3: {
                    Node<3>* node_ptr = static_cast<Node<3>*>(hypertrie);
                    return node_ptr->size();
                }
                default:
                    throw std::logic_error{"not implemented."};
            }
        }

        [[nodiscard]]
        bool operator[](const Key &key) const {
            switch (depth_) {
                case 1: {
                    NodePointer<1> node_ptr = NodePointer<1>{hypertrie};
                    if (node_ptr.getTag() == NodePointer<1>::COMPRESSED_TAG) {
                        return node_ptr.getCompressedNode()->operator[](key[0]);
                    } else {
                        return node_ptr.getNode()->operator[](key[0]);
                    }
                }
                case 2: {
                    NodePointer<2> node_ptr = NodePointer<2>{hypertrie};
                    typename Node<2>::Key raw_key;
                    std::copy_n(key.begin(), 2, raw_key.begin());
                    if (node_ptr.getTag() == NodePointer<2>::COMPRESSED_TAG) {
                        return node_ptr.getCompressedNode()->operator[](raw_key);
                    } else {
                        return node_ptr.getNode()->operator[](raw_key);
                    }
                }
                case 3: {
                    NodePointer<3> node_ptr = NodePointer<3>{hypertrie};
                    typename Node<3>::Key raw_key;
                    std::copy_n(key.begin(), 3, raw_key.begin());
                    if (node_ptr.getTag() == NodePointer<3>::COMPRESSED_TAG) {
                        return node_ptr.getCompressedNode()->operator[](raw_key);
                    } else {
                        return node_ptr.getNode()->operator[](raw_key);
                    }
                }
                default:
                    throw std::logic_error{"not implemented."};
            }
        }

    protected:
        template<pos_type depth>
        inline static std::tuple<typename Node<depth>::SliceKey, pos_type>
        extractRawSliceKey(const SliceKey &slice_key) {
            typename Node<depth>::SliceKey raw_slice_key;
            std::copy_n(slice_key.begin(), depth, raw_slice_key.begin());
            return {raw_slice_key, std::count(slice_key.begin(), slice_key.end(), std::nullopt)};
        }

        template<pos_type depth, pos_type result_depth>
        inline static auto
        executeRawSlice(void *hypertrie,
                        typename Node<depth>::SliceKey raw_slice_key)
        -> std::conditional_t<(result_depth > 0), std::optional<const_CompressedBoolHypertrie const>, bool> {
            NodePointer<depth> node_ptr(hypertrie);
            constexpr pos_type depth_val = result_depth;

            if constexpr (result_depth > 0) {
                if (node_ptr.getTag() == NodePointer<depth>::COMPRESSED_TAG) {
                    auto result = node_ptr.getCompressedNode()->template operator[]<result_depth>(raw_slice_key);
                    if (!result.isEmpty()) {
                        return instance<result_depth>(depth_val, result);
                    } else {
                        return std::nullopt;
                    }
                } else {
                    auto result = node_ptr.getNode()->template operator[]<result_depth>(raw_slice_key);
                    if (!result.isEmpty()) {
                        return instance<result_depth>(depth_val, result);
                    } else {
                        return std::nullopt;
                    }
                }

            } else {
                if constexpr (depth > 2) {
                    Node<depth> *node_ptr = static_cast<Node<depth> * >(hypertrie);
                    return node_ptr->template operator[]<0>(raw_slice_key);
                } else {
                    if (node_ptr.getTag() == NodePointer<depth>::COMPRESSED_TAG) {
                        return node_ptr.getCompressedNode()->template operator[]<0>(raw_slice_key);
                    } else {
                        return node_ptr.getNode()->template operator[]<0>(raw_slice_key);
                    }
                }

            }
        }

    public:

        [[nodiscard]]
        std::variant<std::optional<const_CompressedBoolHypertrie>, bool> operator[](const SliceKey &slice_key) const {
            switch (depth_) {
                case 1: {
                    NodePointer<1> node_ptr = NodePointer<1>{hypertrie};
                    if (slice_key[0]) {
                        if (node_ptr.getTag() == NodePointer<1>::COMPRESSED_TAG) {
                            return node_ptr.getCompressedNode()->operator[](*(slice_key[0]));
                        } else {
                            return node_ptr.getNode()->operator[](*(slice_key[0]));
                        }
                    } else {
                        if (this->size()) return {*this};
                        else return {std::optional<const_CompressedBoolHypertrie>{}};
                    }
                }

                case 2: {
                    auto[raw_slice_key, count] = extractRawSliceKey<2>(slice_key);
                    switch (count) {
                        case 2: {
                            if (this->size()) return {*this};
                            else return {std::optional<const_CompressedBoolHypertrie>{}};
                        }
                        case 1:
                            return executeRawSlice<2, 1>(hypertrie, std::move(raw_slice_key));
                        case 0:
                            return executeRawSlice<2, 0>(hypertrie, std::move(raw_slice_key));
                    }
                    [[fallthrough]];
                }

                case 3: {
                    auto[raw_slice_key, count] = extractRawSliceKey<3>(slice_key);
                    switch (count) {
                        case 3: {
                            if (this->size()) return {*this};
                            else return {std::optional<const_CompressedBoolHypertrie>{}};
                        }

                        case 2:
                            return executeRawSlice<3, 2>(hypertrie, std::move(raw_slice_key));
                        case 1:
                            return executeRawSlice<3, 1>(hypertrie, std::move(raw_slice_key));
                        case 0:
                            return executeRawSlice<3, 0>(hypertrie, std::move(raw_slice_key));
                    }
                    [[fallthrough]];
                }

                default:
                    throw std::logic_error{"not implemented."};
            }
        }

        [[nodiscard]]
        pos_type depth() const { return depth_; }


        template<typename, template<typename, typename> class,
                template<typename> class>
        friend
        class CompressedHashDiagonal;
    };

    template<typename key_part_type, template<typename, typename> class map_type,
            template<typename> class set_type>
    class CompressedBoolHypertrie : public const_CompressedBoolHypertrie<key_part_type, map_type, set_type> {
    protected:
        using base = const_CompressedBoolHypertrie<key_part_type, map_type, set_type>;
        template<pos_type depth, bool compressed>
        using RawCompressedBoolHypertrie = typename base::template RawCompressedBoolHypertrie<depth, compressed>;
        template<pos_type depth, pos_type diag_depth>
        using RawCompressedBHTHashDiagonal =  typename base::template RawCompressedBHTHashDiagonal<diag_depth, depth>;

        template<pos_type depth>
        using Node = typename base::template Node<depth>;

        template<pos_type depth>
        using CompressedNode = typename base::template CompressedNode<depth>;

        template<pos_type depth>
        using NodePointer = typename base::template NodePointer<depth>;


        // proxy for fields:
        using base::depth_;
        using base::hypertrie;
    public:
        using SliceKey =  typename base::SliceKey;
        using Key =  typename base::Key;

        CompressedBoolHypertrie() : base{} {}

        CompressedBoolHypertrie(CompressedBoolHypertrie &boolhypertrie) : base{boolhypertrie} {}

        CompressedBoolHypertrie(const CompressedBoolHypertrie &) = default;

        CompressedBoolHypertrie(CompressedBoolHypertrie &&boolhypertrie) noexcept : base{boolhypertrie} {}

        explicit CompressedBoolHypertrie(pos_type depth) : base{depth} {}

    protected:
        template<pos_type depth>
        inline static void rawSet(void *hypertrie, const Key &key, [[maybe_unused]] bool value) {
            NodePointer<depth> node_ptr{hypertrie};
            if constexpr (depth == 3) {
                typename Node<depth>::Key raw_key;
                std::copy_n(key.begin(), depth, raw_key.begin());

                if (node_ptr.getTag() == NodePointer<depth>::COMPRESSED_TAG) {
                    node_ptr.getCompressedNode()->set(raw_key);
                } else {
                    node_ptr.getNode()->set(raw_key);
                }
            } else {
                throw std::logic_error{"not implemented."};
            }
        }

    public:
        void set(const Key &key, [[maybe_unused]] bool value) {
            assert(key.size() == depth_);
            switch (this->depth()) {
                case 3: {
                    rawSet<3>(hypertrie, key, value);
                    break;
                }
                default:
                    throw std::logic_error{"not implemented."};
            }
        }
    };

}
#endif //HYPERTRIE_COMPRESSEDBOOLHYPERTRIE_IMPL_HPP
