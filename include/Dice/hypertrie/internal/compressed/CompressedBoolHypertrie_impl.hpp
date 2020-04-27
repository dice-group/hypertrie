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
        template<pos_type depth, pos_type diag_depth, bool compressed>
        using RawCompressedBHTHashDiagonal = typename hypertrie::internal::compressed::interface::rawcompressedboolhypertrie<key_part_type, map_type, set_type>::template RawCompressedBHTHashDiagonal<diag_depth, depth, compressed>;

        template<pos_type depth_t>
        using Node = RawCompressedBoolHypertrie<depth_t, false>;

        template<pos_type depth_t>
        using CompressedNode = RawCompressedBoolHypertrie<depth_t, true>;

        template<pos_type depth_t>
        using NodePointer = util::CompressedBoolHyperTrieTaggedPointer<CompressedNode<depth_t> *, Node<depth_t> *, 8>;

    public:
        typedef std::vector <std::optional<key_part_type>> SliceKey;
        typedef std::vector <key_part_type> Key;

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

        inline static const_CompressedBoolHypertrie instance(const pos_type depth, void *hypertrie) {
            return const_CompressedBoolHypertrie(depth, hypertrie);
        }

    protected:
        explicit const_CompressedBoolHypertrie(pos_type depth, void *boolhypertrie)
                : depth_(depth), hypertrie(boolhypertrie) {}

    public:
        [[nodiscard]]
        std::vector <size_t> getCards(const std::vector <pos_type> &positions) const {
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
                    return node_ptr.getNode()->getCards(positions);
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
                    NodePointer<3> node_ptr = NodePointer<3>{hypertrie};
                    return node_ptr.getNode()->size();
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
                        return instance(depth_val, result.getPointer());
                    } else {
                        return std::nullopt;
                    }
                } else {
                    auto result = node_ptr.getNode()->template operator[]<result_depth>(raw_slice_key);
                    if (!result.isEmpty()) {
                        return instance(depth_val, result.getPointer());
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
                        else return {std::optional < const_CompressedBoolHypertrie > {}};
                    }
                }

                case 2: {
                    auto[raw_slice_key, count] = extractRawSliceKey<2>(slice_key);
                    switch (count) {
                        case 2: {
                            if (this->size()) return {*this};
                            else return {std::optional < const_CompressedBoolHypertrie > {}};
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
                            else return {std::optional < const_CompressedBoolHypertrie > {}};
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

        class iterator {
        protected:
            struct RawMethods {

                void (*destruct)(const void *);

                void *(*begin)(const const_CompressedBoolHypertrie &boolHypertrie);

                const Key &(*value)(void const *);

                void (*inc)(void *);

                bool (*ended)(void const *);

            };

            template<pos_type depth>
            inline static RawMethods generateRawMethods() {
                return RawMethods{
                        [](const void *rawboolhypertrie_iterator) {
                            using T = const typename Node<depth>::iterator;
                            if (rawboolhypertrie_iterator != nullptr) {
                                delete static_cast<T *>(rawboolhypertrie_iterator);
                            }
                        },
                        [](const const_CompressedBoolHypertrie &boolHypertrie) -> void * {
                            NodePointer<depth> hypertrie_ptr{boolHypertrie.hypertrie};
                            return new typename Node<depth>::iterator(*hypertrie_ptr.getNode());
                        },
                        &Node<depth>::iterator::value,
                        &Node<depth>::iterator::inc,
                        &Node<depth>::iterator::ended};
            }

            template<pos_type depth>
            inline static RawMethods generateCompressedRawMethods() {
                return RawMethods{
                        [](const void *rawboolhypertrie_iterator) {
                            using T = const typename CompressedNode<depth>::iterator;
                            if (rawboolhypertrie_iterator != nullptr) {
                                delete static_cast<T *>(rawboolhypertrie_iterator);
                            }
                        },
                        [](const const_CompressedBoolHypertrie &boolHypertrie) -> void * {
                            NodePointer<depth> hypertrie_ptr{boolHypertrie.hypertrie};
                            return new typename CompressedNode<depth>::iterator(*hypertrie_ptr.getCompressedNode());
                        },
                        &CompressedNode<depth>::iterator::value,
                        &CompressedNode<depth>::iterator::inc,
                        &CompressedNode<depth>::iterator::ended};
            }

            inline static const std::vector <RawMethods> RawMethodsCache{
                    generateRawMethods<1>(),
                    generateRawMethods<2>(),
                    generateRawMethods<3>()
            };

            inline static const std::vector <RawMethods> CompressedRawMethodsCache{
                    generateCompressedRawMethods<1>(),
                    generateCompressedRawMethods<2>()
            };

            static RawMethods const &getRawMethods(pos_type depth) {
                return RawMethodsCache[depth - 1];
            };

            static RawMethods const &getCompressedRawMethods(pos_type depth) {
                return CompressedRawMethodsCache[depth - 1];
            };


        protected:
            RawMethods const *raw_methods = nullptr;
            void *raw_iterator = nullptr;

        public:
            using self_type =  iterator;
            using value_type = Key;

            iterator() = default;

            iterator(iterator &) = delete;

            iterator(const iterator &) = delete;

            iterator(iterator &&) = delete;

            iterator &operator=(iterator &&other) noexcept {
                if (this->raw_methods != nullptr)
                    this->raw_methods->destruct(this->raw_iterator);
                this->raw_methods = other.raw_methods;
                this->raw_iterator = other.raw_iterator;
                other.raw_iterator = nullptr;
                other.raw_methods = nullptr;
                return *this;
            }

            iterator &operator=(iterator &) = delete;

            iterator &operator=(const iterator &) = delete;

            iterator(const_CompressedBoolHypertrie const *const boolHypertrie) {
                switch (boolHypertrie->depth()) {
                    case 1: {
                        NodePointer<1> node_ptr{boolHypertrie->hypertrie};
                        if (node_ptr.getTag() == NodePointer<1>::COMPRESSED_TAG) {
                            raw_methods = &getCompressedRawMethods(1);
                            raw_iterator = raw_methods->begin(*boolHypertrie);
                        } else {
                            raw_methods = &getRawMethods(1);
                            raw_iterator = raw_methods->begin(*boolHypertrie);
                        }
                        break;
                    }
                    case 2: {
                        NodePointer<2> node_ptr{boolHypertrie->hypertrie};
                        if (node_ptr.getTag() == NodePointer<2>::COMPRESSED_TAG) {
                            raw_methods = &getCompressedRawMethods(2);
                            raw_iterator = raw_methods->begin(*boolHypertrie);
                        } else {
                            raw_methods = &getRawMethods(2);
                            raw_iterator = raw_methods->begin(*boolHypertrie);
                        }
                        break;
                    };
                    case 3: {
                        raw_methods = &getRawMethods(3);
                        raw_iterator = raw_methods->begin(*boolHypertrie);
                        break;
                    }
                }
            }

            iterator(const_CompressedBoolHypertrie &boolHypertrie) : iterator(&boolHypertrie) {}

            ~iterator() {
                if (raw_methods != nullptr)
                    raw_methods->destruct(raw_iterator);
                raw_methods = nullptr;
                raw_iterator = nullptr;
            }

            self_type &operator++() {
                raw_methods->inc(raw_iterator);
                return *this;
            }

            value_type operator*() const { return raw_methods->value(raw_iterator); }

            operator bool() const { return not raw_methods->ended(raw_iterator); }

        };

        using const_iterator = iterator;

        [[nodiscard]]
        iterator begin() const { return iterator{this}; }

        [[nodiscard]]
        const_iterator cbegin() const { return iterator{this}; }

        [[nodiscard]]
        bool end() const { return false; }

        [[nodiscard]]
        bool cend() const { return false; }
    };

    template<typename key_part_type, template<typename, typename> class map_type,
            template<typename> class set_type>
    class CompressedBoolHypertrie : public const_CompressedBoolHypertrie<key_part_type, map_type, set_type> {
    protected:
        using base = const_CompressedBoolHypertrie<key_part_type, map_type, set_type>;
        template<pos_type depth, bool compressed>
        using RawCompressedBoolHypertrie = typename base::template RawCompressedBoolHypertrie<depth, compressed>;
        template<pos_type depth, pos_type diag_depth, bool compressed>
        using RawCompressedBHTHashDiagonal =  typename base::template RawCompressedBHTHashDiagonal<diag_depth, depth, compressed>;

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
            typename Node<depth>::Key raw_key;
            std::copy_n(key.begin(), depth, raw_key.begin());

            if (node_ptr.getTag() == NodePointer<depth>::COMPRESSED_TAG) {
                throw std::logic_error{"not implemented."};
            } else {
                node_ptr.getNode()->set(raw_key);
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
                case 2: {
                    rawSet<2>(hypertrie, key, value);
                    break;
                }
                case 1: {
                    rawSet<1>(hypertrie, key, value);
                    break;
                }
                default:
                    throw std::logic_error{"not implemented."};
            }
        }
    };

}
#endif //HYPERTRIE_COMPRESSEDBOOLHYPERTRIE_IMPL_HPP
