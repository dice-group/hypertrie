#ifndef HYPERTRIE_BOOLCOMPRESSEDHYPERTRIE_IMPL_H
#define HYPERTRIE_BOOLCOMPRESSEDHYPERTRIE_IMPL_H


/**
 *
* Created by burhan otour on 11.01.20
 *
*/
#include <tuple>

#include <utility>
#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>
#include <memory>
#include <variant>
#include <vector>
#include <cassert>
#include <cstdint>
#include <itertools.hpp>
#include "Dice/hypertrie/internal/util/TaggedPointerWrapper.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/util/NTuple.hpp"
#include "Dice/hypertrie/internal/util/CountDownNTuple.hpp"
#include "Dice/hypertrie/internal/util/FrontSkipIterator.hpp"
#include "Dice/hypertrie/internal/util/PosCalc.hpp"
#include "Dice/hypertrie/internal/container/AllContainer.hpp"

namespace hypertrie::internal::compressed {

    constexpr int factorial(int n) {
        return n <= 1 ? 1 : (n * factorial(n - 1));
    }

    template<pos_type diag_depth, pos_type depth, bool compressed, typename key_part_type, template<typename, typename> typename map_type,
            template<typename> typename set_type, typename = typename std::enable_if_t<(diag_depth <= depth)>>
    class RawCompressedBHTHashDiagonal;

    using pos_type = uint8_t;

    template<pos_type depth, typename key_part_type>
    using RawKey = std::array<key_part_type, depth>;

    template<pos_type depth, typename key_part_type>
    using RawSliceKey = std::array<std::optional<key_part_type>, depth>;

    using size_t = std::size_t;

    template<typename key_part_type_t, int N>
    using compressed_children_array = std::array<key_part_type_t, N>;

    template<pos_type depth, typename key_part_type, template<typename, typename> typename map_type,
            template<typename> typename set_type, bool compressedMode, typename  = typename std::enable_if_t<(
                    depth >= 1)>>
    class RawCompressedBoolHypertrie;

    /**
     * Constant BoolHypertrie Wrappers for compressed children with depth = 1
     */
    template<typename key_part_type_t, template<typename, typename> typename map_type_t,
            template<typename> typename set_type_t> //
    class RawCompressedBoolHypertrie<1, key_part_type_t, map_type_t, set_type_t, true> {
    protected:
        template<pos_type depth_tt>
        using CompressedNode = RawCompressedBoolHypertrie<depth_tt, key_part_type_t, map_type_t, set_type_t, true, void>;
        static constexpr int depth = 1;

        template<pos_type depth_tt>
        using Node = RawCompressedBoolHypertrie<depth_tt, key_part_type_t, map_type_t, set_type_t, false>;
    public:
        typedef RawKey<1, key_part_type_t> Key;
        typedef RawSliceKey<1, key_part_type_t> SliceKey;
        typedef bool child_type;
        typedef key_part_type_t key_part_type;
        template<pos_type depth_tt>
        using NodePointer = util::CompressedBoolHyperTrieTaggedPointer<CompressedNode<depth_tt> *, Node<depth_tt> *, 8>;
    private:
        key_part_type key_part;
    public:
        RawCompressedBoolHypertrie() = delete;

        RawCompressedBoolHypertrie(key_part_type const key_part_instance) : key_part(key_part_instance) {}

        [[nodiscard]]
        bool operator[](key_part_type_t const &key_part_instance) const {
            return key_part == key_part_instance;
        }

        [[nodiscard]]
        bool operator[](Key const &key) const {
            return key_part == key[0];
        }

    protected:
        template<pos_type slice_count, typename  = typename std::enable_if_t<(slice_count == 1)>>
        [[nodiscard]]
        bool
        diagonal_rek([[maybe_unused]]const std::vector<pos_type> &positions, [[maybe_unused]]std::vector<bool> &done,
                     const key_part_type &key_part_instance) const {
            return key_part == key_part_instance;
        }

    public:

        template<pos_type slice_count, typename  = typename std::enable_if_t<(slice_count == 0)>>
        auto operator[](const SliceKey &key) const {
            assert(key[0].has_values());
            return this->operator[](key[0]);
        }

        [[nodiscard]]
        child_type get([[maybe_unused]]pos_type const &pos, key_part_type_t const &key_part_instance) const {
            return key_part == key_part_instance;
        }

        [[nodiscard]]
        bool diagonal(const key_part_type_t &key_part_instance) const {
            return key_part == key_part_instance;
        }

        [[nodiscard]]
        std::vector<size_t> getCards(const std::vector<pos_type> &positions) const {
            assert((positions.size() == 1) ? positions[0] == 0 : positions.empty());
            std::vector<size_t> cards(1);
            cards[0] = 1;
            return cards;
        }

        [[nodiscard]]
        size_t size() const {
            return 1;
        }

        // The wrapper should never return empty
        [[nodiscard]]
        bool empty() const {
            return false;
        }

        key_part_type currentKeyPart([[maybe_unused]] const pos_type &pos) const {
            return key_part;
        }

        template<pos_type depth_t>
        static auto &const_emtpy_instance() {
            static thread_local NodePointer<depth_t> inst{};
            return inst;
        }

        class iterator {
        public:
            using self_type =  iterator;
            using value_type = std::vector<key_part_type>;
        protected:
            mutable value_type value_;
            bool ended_;
        public:

            iterator(CompressedNode<1> const *const boolhypertrie) : value_(1), ended_(false) {
                value_[0] = boolhypertrie->key_part;
            }

            iterator(CompressedNode<1> const &boolhypertrie) : iterator(&boolhypertrie) {}

            inline self_type &operator++() {
                this->ended_ = true;
                return *this;
            }

            static void inc(void *it_ptr) {
                auto &it = *static_cast<iterator *>(it_ptr);
                ++it;
            }

            [[nodiscard]]
            inline const value_type &operator*() const {
                if (ended_) {
                    throw std::logic_error{"iteration ended."};
                }
                return value_;
            }

            [[nodiscard]]
            static const value_type &value(void const *it_ptr) {
                auto &it = *static_cast<iterator const *>(it_ptr);
                return *it;
            }

            [[nodiscard]]
            inline operator bool() const { return not ended_; }

            [[nodiscard]]
            static bool ended(void const *it_ptr) {
                auto &it = *static_cast<iterator const *>(it_ptr);
                return it.ended_;
            }
        };

        using const_iterator = iterator;

        [[nodiscard]]
        iterator begin() const noexcept { return {this}; }

        [[nodiscard]]
        const_iterator cbegin() const noexcept { return {this}; }


        [[nodiscard]]
        bool end() const noexcept { return false; }

        [[nodiscard]]
        bool cend() const noexcept { return false; }
    };

    template<typename key_part_type_t, template<typename, typename> typename map_type_t,
            template<typename> typename set_type_t> //
    class RawCompressedBoolHypertrie<1, key_part_type_t, map_type_t, set_type_t, false> {
        // make the bool hypertries friends to each other
        template<pos_type, typename, template<typename, typename> typename,
                template<typename> typename, bool, typename>
        friend
        class RawCompressedBoolHypertrie;

#ifdef __clang__
        template<pos_type depth2, typename key_part_type_t2, template<typename, typename> typename map_type_t2,
                template<typename> typename set_type_t2, typename enabled2>
        friend
        class RawCompressedBoolHypertrie<depth2, key_part_type_t2, map_type_t2, set_type_t2, false, enabled2>::iterator;
#endif
    protected:
        template<pos_type depth_t>
        using boolhypertrie_c = RawCompressedBoolHypertrie<depth_t, key_part_type_t, map_type_t, set_type_t, false, void>;
        static constexpr int depth = 1;

        using Node = boolhypertrie_c<depth>;

    public:
        typedef RawKey<1, key_part_type_t> Key;
        typedef RawSliceKey<1, key_part_type_t> SliceKey;
        typedef bool child_type;
        typedef bool compressed_child_type;

    protected:
        typedef set_type_t<key_part_type_t> children_type;

    public:
        typedef children_type edges_type;
        typedef key_part_type_t key_part_type;

        template<typename key, typename value>
        using map_type = map_type_t<key, value>;
        template<typename key>
        using set_type = set_type_t<key>;

        template<pos_type depth_t>
        static auto &const_emtpy_instance() {
            static thread_local std::shared_ptr<boolhypertrie_c<depth_t>> inst{};
            return inst;
        }

        /*protected:
            edges_type edges{};*/
    public:
        edges_type edges{};

        RawCompressedBoolHypertrie() = default;

        RawCompressedBoolHypertrie(RawCompressedBoolHypertrie &) = delete;

        RawCompressedBoolHypertrie(RawCompressedBoolHypertrie &&) noexcept = default;

        [[nodiscard]]
        bool operator[](key_part_type_t key_part) const {
            return edges.count(key_part);
        }

        [[nodiscard]]
        bool operator[](Key key) {
            return edges.count(key[0]);
        }

    protected:
        template<pos_type slice_count, typename  = typename std::enable_if_t<(slice_count == 1)>>
        [[nodiscard]]
        bool
        diagonal_rek([[maybe_unused]]const std::vector<pos_type> &positions, [[maybe_unused]]std::vector<bool> &done,
                     const key_part_type_t &key_part) const {
            return edges.count(key_part);
        }

    public:

        template<pos_type slice_count, typename  = typename std::enable_if_t<(slice_count == 0)>>
        auto operator[](const SliceKey &key) const {
            assert(key[0].has_values());
            return this->operator[](key[0]).value();
        }

        [[nodiscard]]
        child_type get([[maybe_unused]]pos_type pos, key_part_type_t key_part) const {
            assert(pos == 0);
            return edges.count(key_part);
        }

        void set(const Key &key) {
            edges.insert(key[0]);
        }

        void set(key_part_type_t key) {
            edges.insert(key);
        }

        [[nodiscard]]
        child_type diagonal(const key_part_type_t &key_part) const {
            return edges.count(key_part);
        }

        [[nodiscard]]
        std::vector<size_t> getCards(const std::vector<pos_type> &positions) const {
            assert((positions.size() == 1) ? positions[0] == 0 : positions.empty());
            std::vector<size_t> cards(positions.size());
            for (auto i : iter::range(positions.size()))
                cards[i] = edges.size();
            return cards;
        }

        [[nodiscard]]
        size_t size() const {
            return edges.size();
        }

        [[nodiscard]]
        bool empty() const {
            return edges.empty();
        }

        template<pos_type, pos_type, bool, typename, template<typename, typename> typename,
                template<typename> typename, typename>
        friend
        class RawCompressedBHTHashDiagonal;

        class iterator {
        public:
            using self_type =  iterator;
            using value_type = std::vector<key_part_type>;
        protected:
            typename edges_type::const_iterator iter;
            typename edges_type::const_iterator end;
            mutable value_type value_;
        public:

            iterator(boolhypertrie_c<depth> const *const raw_boolhypertrie) : iter(raw_boolhypertrie->edges.cbegin()),
                                                                              end(raw_boolhypertrie->edges.cend()),
                                                                              value_(1) {
                if (iter != end)
                    value_[0] = *iter;
            }

            iterator(boolhypertrie_c<depth> const &raw_boolhypertrie) : iterator(&raw_boolhypertrie) {}

            inline self_type &operator++() {
                ++iter;
                return *this;
            }

            static void inc(void *it_ptr) {
                auto &it = *static_cast<iterator *>(it_ptr);
                ++it;
            }

            [[nodiscard]]
            inline const value_type &operator*() const {
                value_[0] = *iter;
                return value_;
            }

            [[nodiscard]]
            static const value_type &value(void const *it_ptr) {
                auto &it = *static_cast<iterator const *>(it_ptr);
                return *it;
            }

            [[nodiscard]]
            inline operator bool() const { return iter != end; }

            [[nodiscard]]
            static bool ended(void const *it_ptr) {
                auto &it = *static_cast<iterator const *>(it_ptr);
                return not it;
            }

        };

        using const_iterator = iterator;

        [[nodiscard]]
        iterator begin() const noexcept { return {this}; }

        [[nodiscard]]
        const_iterator cbegin() const noexcept { return {this}; }


        [[nodiscard]]
        bool end() const noexcept { return false; }

        [[nodiscard]]
        bool cend() const noexcept { return false; }
    };

    /**
     * Constant BoolHypertrie Wrappers for compressed children with depth = 2
     */
    template<typename key_part_type_t, template<typename, typename> typename map_type_t,
            template<typename> typename set_type_t>
    class RawCompressedBoolHypertrie<2, key_part_type_t, map_type_t, set_type_t, true> {

    protected:
        // make the bool hypertries friends to each other
        template<pos_type, typename, template<typename, typename> typename,
                template<typename> typename, bool, typename>
        friend
        class RawCompressedBoolHypertrie;

        template<pos_type, pos_type, bool, typename, template<typename, typename> typename,
                template<typename> typename, typename>
        friend
        class RawCompressedBHTHashDiagonal;


    public:
        static constexpr int depth = 2;
        using Key = RawKey<2, key_part_type_t>;
        using SliceKey = RawSliceKey<2, key_part_type_t>;
    protected:
        template<pos_type depth_t, bool compressed_t>
        using rawCompressedboolhypertrie_c = RawCompressedBoolHypertrie<depth_t, key_part_type_t, map_type_t, set_type_t, compressed_t>;

        template<pos_type depth_tt>
        using Node = RawCompressedBoolHypertrie<depth_tt, key_part_type_t, map_type_t, set_type_t, false>;

        template<pos_type depth_tt>
        using CompressedNode = RawCompressedBoolHypertrie<depth_tt, key_part_type_t, map_type_t, set_type_t, true>;


        using key_part_type = key_part_type_t;
        using children_type = compressed_children_array<key_part_type_t, 2>;
        using edges_type = children_type;
        using size_t = std::size_t;
    public:
        template<pos_type depth_tt>
        using NodePointer = util::CompressedBoolHyperTrieTaggedPointer<CompressedNode<depth_tt> *, Node<depth_tt> *, 8>;

        using PosCalc = util::PosCalc;

    protected:
        using ChildNode = Node<depth - 1>;
        using CompressedChildNode = CompressedNode<depth - 1>;
    public:
        using child_type = NodePointer<depth - 1>;
    public:
        edges_type const &edges;

        RawCompressedBoolHypertrie() = delete;

        RawCompressedBoolHypertrie(edges_type const &children) : edges(children) {}

        [[nodiscard]]
        inline bool operator[](const Key &key) const {
            return edges[0] == key[0] and edges[1] == key[1];
        }

        [[nodiscard]]
        bool diagonal(const key_part_type_t &key_part) const {
            return edges[0] == key_part and edges[1] == key_part;
        }

        key_part_type currentKeyPart(const pos_type &pos) const {
            return edges[pos];
        }

        template<pos_type diag_depth, typename  = typename std::enable_if_t<((diag_depth > 0) and
                                                                             (diag_depth <= depth))>>
        [[nodiscard]]
        auto diagonal(const std::vector<pos_type> &positions, const key_part_type_t &key_part) const {
            assert(positions.size() == diag_depth);
            std::vector<bool> done(positions.size(), false);
            return this->template diagonal_rek<diag_depth>(positions, done, key_part);
        }

        template<pos_type depth_t>
        static auto &const_emtpy_instance() {
            static thread_local NodePointer<depth_t> inst{};
            return inst;
        }

    protected:
        template<pos_type diag_depth, typename  = typename std::enable_if_t<((diag_depth > 0) and
                                                                             (diag_depth <= depth))>>
        [[nodiscard]]
        auto diagonal_rek(const std::vector<pos_type> &positions, std::vector<bool> &done,
                          const key_part_type_t &key_part) const
        -> std::conditional_t<(depth != diag_depth), NodePointer<depth - diag_depth>, bool> {
            std::size_t min_i = 0;
            auto delta_i = 0;
            auto delta = 0;
            for (auto i : iter::range(positions.size())) {
                if (not done[i]) {
                    min_i = i;
                    delta_i = delta;
                    break;
                } else {
                    delta++;
                }
            }
            auto min_pos = positions[min_i] - delta_i;
            done[min_i] = true;

            if (edges[min_pos] == key_part) {
                pos_type child_index = min_pos == 0 ? 1 : 0;
                if constexpr (diag_depth == 1) {
                    return {new CompressedChildNode{edges[child_index]}};
                } else {
                    auto child_node = new CompressedChildNode{edges[child_index]};
                    return child_node->template diagonal_rek<diag_depth - 1>(positions, done, key_part);
                }
            }

            if constexpr (depth != diag_depth)
                return CompressedNode<depth - diag_depth>::template const_emtpy_instance<depth - diag_depth>();
            else return false;
        }

    public:
        size_t size() const {
            return 1;
        }

        child_type get(const pos_type edge_index, const key_part_type_t key_part) const {
            if (edges[edge_index] == key_part) {
                pos_type child_index = edge_index == 0 ? 1 : 0;
                return {new CompressedChildNode{edges[child_index]}};
            } else {
                return {};
            }
        }

        template<pos_type slice_count, typename  = typename std::enable_if_t<((slice_count >= 0) and
                                                                              (slice_count < depth))>>
        [[nodiscard]]
        auto operator[](const SliceKey &key) const {
            if constexpr (slice_count > 0) {
                auto[positions, key_parts] = extractPossAndKeyParts(key);
                const PosCalc *posCalc = PosCalc::getInstance(depth);
                return resolve<slice_count>(std::move(positions), std::move(key_parts), posCalc);
            } else {
                Key full_key;
                for (auto[slice, full] : iter::zip(key, full_key))
                    full = *slice;
                return this->operator[](full_key);
            }
        }

        key_part_type get(const pos_type &pos) {
            return edges[pos];
        }

        [[nodiscard]]
        std::vector<size_t> getCards(const std::vector<pos_type> &positions) const {
            std::vector<size_t> cards(positions.size());
            for (auto i : iter::range(positions.size()))
                cards[i] = 1;
            return cards;
        }

        // The wrapper should never return empty
        [[nodiscard]]
        bool empty() const {
            return false;
        }

    protected:
        template<pos_type slice_count>
        [[nodiscard]] auto
        resolve(std::vector<pos_type> &&positions, std::vector<key_part_type> &&key_parts, const PosCalc *posCalc) const
        -> NodePointer<slice_count> {
            auto pos_it = positions.begin();
            auto key_part_it = key_parts.begin();
            return get(posCalc->key_to_subkey_pos(*pos_it), *key_part_it);
        }


        [[nodiscard]]
        static auto extractPossAndKeyParts(const SliceKey &key) {
            std::vector<pos_type> positions;
            std::vector<key_part_type> key_parts;
            positions.reserve(key.size());
            for (const auto[position, key_part] : iter::enumerate(key))
                if (key_part.has_value()) {
                    positions.push_back(position);
                    key_parts.push_back(key_part.value());
                }
            return std::pair{positions, key_parts};
        }

    public:
        class iterator {
        public:
            using self_type =  iterator;
            using value_type = std::vector<key_part_type>;
        protected:
            mutable value_type value_;
            bool ended_;
        public:

            iterator(CompressedNode<2> const *const boolhypertrie) : value_(2), ended_(false) {
                value_[0] = boolhypertrie->edges[0];
                value_[1] = boolhypertrie->edges[1];
            }

            iterator(CompressedNode<2> const &boolhypertrie) : iterator(&boolhypertrie) {}

            inline self_type &operator++() {
                ended_ = true;
                return *this;
            }

            static void inc(void *it_ptr) {
                auto &it = *static_cast<iterator *>(it_ptr);
                ++it;
            }

            [[nodiscard]]
            inline const value_type &operator*() const {
                if (ended_) {
                    throw std::logic_error{"iteration ended."};
                }
                return value_;
            }

            [[nodiscard]]
            static const value_type &value(void const *it_ptr) {
                auto &it = *static_cast<iterator const *>(it_ptr);
                return *it;
            }

            [[nodiscard]]
            inline operator bool() const { return not ended_; }

            [[nodiscard]]
            static bool ended(void const *it_ptr) {
                auto &it = *static_cast<iterator const *>(it_ptr);
                return it.ended_;
            }
        };

        using const_iterator = iterator;

        [[nodiscard]]
        iterator begin() const noexcept { return {this}; }

        [[nodiscard]]
        const_iterator cbegin() const noexcept { return {this}; }


        [[nodiscard]]
        bool end() const noexcept { return false; }

        [[nodiscard]]
        bool cend() const noexcept { return false; }
    };

    // Node type (depth == 2) . Here the solo child of a key is compressed in the same node using pointer tagging
    template<typename key_part_type_t, template<typename, typename> typename map_type_t,
            template<typename> typename set_type_t>
    class RawCompressedBoolHypertrie<2, key_part_type_t, map_type_t, set_type_t, false> {


        template<pos_type, pos_type, bool, typename, template<typename, typename> typename,
                template<typename> typename, typename>
        friend
        class RawCompressedBHTHashDiagonal;

    protected:
        template<pos_type depth_t, bool compressed_t>
        using rawCompressedboolhypertrie_c = RawCompressedBoolHypertrie<depth_t, key_part_type_t, map_type_t, set_type_t, compressed_t>;

    public:
        constexpr static const pos_type depth = 2;

        typedef key_part_type_t key_part_type;

        template<typename key, typename value>
        using map_type = map_type_t<key, value>;

        template<typename key>
        using set_type = set_type_t<key>;

        using Key = RawKey<2, key_part_type_t>;
        using SliceKey = RawSliceKey<2, key_part_type_t>;

    protected:
        // make the bool hypertries friends to each other
        template<pos_type, typename, template<typename, typename> typename,
                template<typename> typename, bool, typename>
        friend
        class RawCompressedBoolHypertrie;

        using PosCalc = util::PosCalc;
        using subkey_mask_t = PosCalc::subkey_mask_t;


        template<pos_type depth_tt>
        using Node = rawCompressedboolhypertrie_c<depth_tt, false>;

        template<pos_type depth_tt>
        using CompressedNode = rawCompressedboolhypertrie_c<depth_tt, true>;

    public:
        using ChildNode = Node<depth - 1>;
        using CompressedChildNode = CompressedNode<depth - 1>;

        template<pos_type depth_tt>
        using NodePointer = util::CompressedBoolHyperTrieTaggedPointer<CompressedNode<depth_tt> *, Node<depth_tt> *, 8>;

        using child_type = NodePointer<1>;
    protected:
        using compressed_child_type = util::KeyPartTaggedPointer<ChildNode, key_part_type_t, 2>;

        using children_type = map_type<key_part_type_t, compressed_child_type>;

        using edges_type = std::array<children_type, 2>;
        using size_t = std::size_t;
    protected:
        // The node holds one edge type, the one whose children are tagged pointers to node(depth = 1)
        static constexpr bool is_tsl_map = std::is_same_v<map_type<int, int>, container::tsl_sparse_map<int, int>>;
        static constexpr bool is_tsl_set = std::is_same_v<set_type<int>, container::tsl_sparse_set<int>>;
        size_t _size;

    public:
        edges_type edges{};

        RawCompressedBoolHypertrie() = default;

        RawCompressedBoolHypertrie(RawCompressedBoolHypertrie &) = delete;

        RawCompressedBoolHypertrie(RawCompressedBoolHypertrie &&) noexcept = default;

        [[nodiscard]]
        inline bool operator[](const Key &key) const {
            auto pos = minCardPos();
            auto compressed_child_type_it = edges[pos].find(key[pos]);
            if (compressed_child_type_it != edges[pos].end()) {
                compressed_child_type child = compressed_child_type_it->second;
                if (child.getTag() == compressed_child_type::INT_TAG) {
                    if (pos == 0 and key[1] == child.getInt()) {
                        return true;
                    } else if (pos == 1 and key[0] == child.getInt()) {
                        return true;
                    } else return false;
                } else {
                    static typename ChildNode::Key next_key;
                    if (pos == 0) {
                        next_key[0] = key[1];
                    } else {
                        next_key[0] = key[0];
                    }
                    return child.getPointer()->operator[](next_key);
                }
            } else {
                return false;
            }
        }

        template<pos_type slice_count, typename  = typename std::enable_if_t<((slice_count >= 0) and
                                                                              (slice_count < depth))>>
        [[nodiscard]]
        auto operator[](const SliceKey &key) const {
            if constexpr (slice_count > 0) {
                auto[positions, key_parts] = extractPossAndKeyParts(key);
                const PosCalc *posCalc = PosCalc::getInstance(depth);
                return resolve<slice_count>(std::move(positions), std::move(key_parts), posCalc);
            } else {
                Key full_key;
                for (auto[slice, full] : iter::zip(key, full_key))
                    full = *slice;
                return this->operator[](full_key);
            }
        }

        [[nodiscard]]
        std::vector<size_t> getCards(const std::vector<pos_type> &positions) const {
            std::vector<size_t> cards(positions.size());
            for (auto i : iter::range(positions.size()))
                cards[i] = edges[positions[i]].size();
            return cards;
        }

        [[nodiscard]]
        pos_type minCardPos() const {
            if (edges[0].size() > edges[1].size()) {
                return 1;
            } else {
                return 0;
            }
        }

        [[nodiscard]]
        std::vector<pos_type>::const_iterator minCardPos(std::vector<pos_type> const &positions) const {
            auto pos_it = positions.begin();
            auto min_pos = pos_it;
            auto min_card = std::numeric_limits<typename children_type::size_type>::max();
            for (; pos_it != positions.end(); ++pos_it) {
                const auto &children = edges[*pos_it];
                if (children.size() < min_card) {
                    min_card = children.size();
                    min_pos = pos_it;
                }
            }
            return min_pos;
        }

        [[nodiscard]]
        std::vector<pos_type>::iterator minCardPos(std::vector<pos_type> &positions, const PosCalc *posCalc) const {
            auto pos_it = positions.begin();
            auto min_pos = pos_it;
            auto min_card = std::numeric_limits<typename children_type::size_type>::max();
            for (; pos_it != positions.end(); ++pos_it) {
                const auto &children = edges[posCalc->key_to_subkey_pos(*pos_it)];
                if (children.size() < min_card) {
                    min_card = children.size();
                    min_pos = pos_it;
                }
            }
            return min_pos;
        }

    protected:
        template<pos_type slice_count>
        [[nodiscard]] auto
        resolve(std::vector<pos_type> &&positions, std::vector<key_part_type> &&key_parts, const PosCalc *posCalc) const
        -> NodePointer<slice_count> {
            auto pos_it = minCardPos(positions, posCalc);
            auto key_part_it = key_parts.begin() + std::distance(positions.begin(), pos_it);
            auto child = get(posCalc->key_to_subkey_pos(*pos_it), *key_part_it);

            return child;
        }


    public:
        [[nodiscard]]
        bool diagonal(const key_part_type_t &key_part) const {
            pos_type min_pos = minCardPos();
            auto found = edges[min_pos].find(key_part);
            if (found != edges[min_pos].end()) {
                const compressed_child_type &compressed_child = found->second;
                if (compressed_child.getTag() == compressed_child_type::INT_TAG) {
                    return compressed_child.getInt() == key_part;
                } else {
                    return found->second.getPointer()->diagonal(key_part);
                }
            } else {
                return false;
            }
        }

        template<pos_type diag_depth, typename  = typename std::enable_if_t<((diag_depth > 0) and
                                                                             (diag_depth <= depth))>>
        [[nodiscard]]
        auto diagonal(const std::vector<pos_type> &positions, const key_part_type_t &key_part) const {
            assert(positions.size() == diag_depth);
            std::vector<bool> done(positions.size(), false);
            return this->template diagonal_rek<diag_depth>(positions, done, key_part);
        }


    protected:
        template<pos_type diag_depth, typename  = typename std::enable_if_t<((diag_depth > 0) and
                                                                             (diag_depth <= depth))>>
        [[nodiscard]]
        auto diagonal_rek(const std::vector<pos_type> &positions, std::vector<bool> &done,
                          const key_part_type_t &key_part) const
        -> std::conditional_t<(depth != diag_depth), NodePointer<depth - diag_depth>, bool> {
            std::size_t min_i = 0;
            auto min_size = std::numeric_limits<std::size_t>::max();
            std::size_t delta = 0;
            std::size_t delta_i = 0;
            for (auto i : iter::range(positions.size()))
                if (not done[i]) {
                    if (auto current_size = edges[positions[i] - delta].size(); current_size < min_size) {
                        min_i = i;
                        min_size = current_size;
                        delta_i = delta;
                    }
                } else {
                    delta += 1;
                    continue;
                }

            auto min_pos = positions[min_i] - delta_i;
            done[min_i] = true;
            auto found = edges[min_pos].find(key_part);
            if (found != edges[min_pos].end()) {
                if constexpr (diag_depth == 1) {
                    compressed_child_type const &child = found->second;
                    if (child.getTag() == compressed_child_type::INT_TAG) {
                        CompressedChildNode *BHT_ptr = new CompressedChildNode{
                                child.getInt()};
                        return {BHT_ptr};
                    } else {
                        return {child.getPointer()};
                    }
                } else {
                    compressed_child_type const &child = found->second;
                    if (child.getTag() == compressed_child_type::INT_TAG) {
                        CompressedChildNode *child_ptr = new CompressedChildNode{
                                child.getInt()};
                        return child_ptr->template diagonal_rek<diag_depth - 1>(positions, done, key_part);
                    } else {
                        ChildNode *child_ptr = child.getPointer();
                        return child_ptr->template diagonal_rek<diag_depth - 1>(positions, done, key_part);
                    }
                }
            }
            if constexpr (depth != diag_depth) return {};
            else return false;
        }

        [[nodiscard]]
        static auto extractPossAndKeyParts(const SliceKey &key) {
            std::vector<pos_type> positions;
            std::vector<key_part_type> key_parts;
            positions.reserve(key.size());
            for (const auto[position, key_part] : iter::enumerate(key))
                if (key_part.has_value()) {
                    positions.push_back(position);
                    key_parts.push_back(key_part.value());
                }
            return std::pair{positions, key_parts};
        }

    public:
        size_t size() const {
            return _size;
        }

        [[nodiscard]]
        bool empty() const {
            return _size == 0;
        }

        // This method hold the information on which edge to look at (in this case, compressed_edges)
        key_part_type_t get_compressed_child(const pos_type edge_index, const key_part_type_t key_part) const {
            auto child_it = edges[edge_index].find(key_part);
            if (child_it != edges[edge_index].end() and child_it->second.getTag() == compressed_child_type::INT_TAG) {
                return child_it->second.getInt();
            } else {
                return 0;
            }
        }

        child_type get(const pos_type edge_index, const key_part_type_t key_part) const {
            auto child_it = edges[edge_index].find(key_part);
            if (child_it != edges[edge_index].end()) {
                if (child_it->second.getTag() == compressed_child_type::INT_TAG) {
                    CompressedChildNode *BHT_ptr = new CompressedChildNode{
                            child_it->second.getInt()};
                    return {BHT_ptr};
                } else {
                    return {child_it->second.getPointer()};
                }
            } else {
                return {};
            }
        }

        child_type get_unsafe(const pos_type edge_index, const key_part_type_t key_part) {
            auto child_it = edges[edge_index].find(key_part);
            if (child_it != edges[edge_index].end()) {
                if (child_it->second.getTag() == compressed_child_type::INT_TAG) {
                    CompressedChildNode *BHT_ptr = new CompressedChildNode{
                            child_it->second.getInt()};
                    return {BHT_ptr};
                } else {
                    return {child_it->second.getPointer()};
                }
            } else {
                return {};
            }
        }

        // Set a new key in the CompressedBoolHypertrie - ignoring the bool value here
        void set(const Key &key) {
            if (!this->operator[](key)) {
                // Prepare a starting point for recusive insetion of key
                const PosCalc *posCalc = PosCalc::getInstance(depth);

                static tsl::hopscotch_map<subkey_mask_t, void *> created_nodes(factorial(2));

                _<2>::setRek(this, key, created_nodes, posCalc);
                created_nodes.clear();
            }
        }

    protected:
        template<pos_type current_depth, typename = typename std::enable_if_t<(current_depth <=
                                                                               2)>, typename DUMMY = void>
        struct _;

        template<pos_type current_depth, typename DUMMY>
        struct _<current_depth, typename std::enable_if_t<(current_depth == 2)>, DUMMY> {
            using child_ = typename Node<2>::template _<1>;
            using CurrentHypertrie = Node<current_depth>;
            using ChildNode = typename CurrentHypertrie::ChildNode;

            inline static void setRek(CurrentHypertrie *current, const Key &key,
                                      tsl::hopscotch_map<subkey_mask_t, void *> &created_nodes,
                                      PosCalc const *posCalc) {
                // CurrentHypertrie current = curre
                using compressed_child_type_t = typename CurrentHypertrie::compressed_child_type;
                current->_size += 1;
                created_nodes[posCalc->getSubKeyMask()] = current;

                for (pos_type key_pos: posCalc->getKeyPoss()) {
                    const key_part_type_t key_part = key[key_pos];
                    const PosCalc *next_pos_calc = posCalc->use(key_pos);
                    auto subkey_pos = posCalc->key_to_subkey_pos(key_pos);

                    auto child_it = current->edges[subkey_pos].find(key_part);
                    // current->get
                    if (child_it != current->edges[subkey_pos].end()) {
                        compressed_child_type_t *child_pptr;
                        if constexpr (is_tsl_map) {
                            child_pptr = &child_it.value();
                        } else {
                            child_pptr = &child_it->second;
                        }
                        compressed_child_type_t &child = *child_pptr;
                        if (child.getTag() == compressed_child_type_t::POINTER_TAG) {
                            child_::setRek(child.getPointer(), key, created_nodes, next_pos_calc);
                        } else if (child.getTag() == compressed_child_type_t::INT_TAG) {
                            const auto &created_child_node = created_nodes.find(next_pos_calc->getSubKeyMask());
                            if (created_child_node != created_nodes.end()) {
                                auto created_child_node_ptr = static_cast<ChildNode *>(
                                        created_child_node->second);
                                child.setPointer(created_child_node_ptr);
                            } else {
                                Key reconstructed_key{};
                                for (size_t i = 0; i < reconstructed_key.size(); ++i) {
                                    if (posCalc->getSubKeyMask().at(i)) {
                                        reconstructed_key[i] = key[i];
                                    } else if (i == key_pos) {
                                        reconstructed_key[i] = key_part;
                                    } else {
                                        // std::cout << child.getInt();
                                        reconstructed_key[i] = child.getInt();
                                    }
                                }
                                auto child_node = new ChildNode{};
                                child.setPointer(child_node);
                                //std::cout << "Key: " << key[0] << "," << key[1] << "," << key[2] << "," << std::endl;
                                // std::cout << "Reconstructed Key: " << reconstructed_key[0] << ","
                                //           << reconstructed_key[1] << "," << reconstructed_key[2] << std::endl;
                                child_::setRek(child_node, key, reconstructed_key, created_nodes, next_pos_calc);
                            }
                        }
                    } else {
                        // a new compressed child node can be added
                        // we get the remaining key part first
                        key_part_type_t child_key_part = key[next_pos_calc->getKeyPoss()[0]];
                        // we set the compressed child
                        compressed_child_type_t compressed_child{child_key_part};
                        current->edges[subkey_pos].insert(std::make_pair(key_part, compressed_child));
                    }
                }
            }
        };

        template<typename DUMMY>
        struct _<1, typename std::enable_if_t<(true)>, DUMMY> {
            using CurrentHypertrie = Node<1>;

            inline static void setRek(CurrentHypertrie *current, const Key &key,
                                      tsl::hopscotch_map<subkey_mask_t, void *> &created_nodes,
                                      PosCalc const *posCalc) {
                // add it to the finished ( means updated ) nodes.
                if (created_nodes.find(posCalc->getSubKeyMask()) == created_nodes.end()) {
                    created_nodes[posCalc->getSubKeyMask()] = current;
                }
                //std::cout << "Key1(depth=1): " << key[0] << "," << key[1] << "," << key[2] << "," << std::endl;
                // set the entry in the set
                key_part_type_t key_part = key[posCalc->subkey_to_key_pos(0)];

                current->set(key_part);
            }

            inline static void
            setRek(CurrentHypertrie *current, const Key &key, const Key &reconstruced_key,
                   tsl::hopscotch_map<subkey_mask_t, void *> &created_nodes,
                   PosCalc const *posCalc) {
                // add it to the finished ( means updated ) nodes.
                if (created_nodes.find(posCalc->getSubKeyMask()) == created_nodes.end()) {
                    created_nodes[posCalc->getSubKeyMask()] = current;
                }
                //std::cout << "Key(depth=1): " << key[0] << "," << key[1] << "," << key[2] << "," << std::endl;
                //std::cout << "Reconstruced Key(depth=1): " << reconstruced_key[0] << "," << reconstruced_key[1] << "," << reconstruced_key[2] << "," << std::endl;
                // set the entry in the set
                key_part_type_t key_part = key[posCalc->subkey_to_key_pos(0)];
                key_part_type_t reconstruced_key_part = reconstruced_key[posCalc->subkey_to_key_pos(0)];

                current->set(key_part);
                current->set(reconstruced_key_part);
            }
        };

    public:
        class iterator {
            template<pos_type depth_>
            using childen_t  = typename Node<depth_>::children_type::const_iterator;

            util::CountDownNTuple<childen_t, depth> iters;
            util::CountDownNTuple<childen_t, depth> ends;
        public:
            using self_type =  iterator;
            using value_type = std::vector<key_part_type>;
        protected:
            Node<depth> const *const raw_boolhypertrie;

            std::vector<key_part_type> key;
            bool ended_;
        public:

            iterator(Node<depth> const *const raw_boolhypertrie)
                    : raw_boolhypertrie(raw_boolhypertrie),
                      key(depth),
                      ended_{raw_boolhypertrie->empty()} {
                if (not ended_)
                    init_rek();
            }

            iterator(Node<depth> const &raw_boolhypertrie) : iterator(&raw_boolhypertrie) {}

            inline self_type &operator++() {
                inc_rek();
                return *this;
            }

            static void inc(void *it_ptr) {
                auto &it = *static_cast<iterator *>(it_ptr);
                ++it;
            }

            inline const value_type &operator*() const { return key; }

            static const value_type &value(void const *it_ptr) {
                auto &it = *static_cast<iterator const *>(it_ptr);
                return *it;
            }

            inline operator bool() const { return not ended_; }

            static bool ended(void const *it_ptr) {
                auto &it = *static_cast<iterator const *>(it_ptr);
                return it.ended_;
            }

        protected:

            inline void init_rek() {
                // get the iterator
                auto &iter = std::get<depth - 1>(iters);
                iter = raw_boolhypertrie->edges[0].cbegin();
                auto &end = std::get<depth - 1>(ends);
                end = raw_boolhypertrie->edges[0].cend();

                // set the key_part in the key
                key[0] = std::get<depth - 1>(iters)->first;
                init_rek < depth - 1 > ();
            }

            template<pos_type current_depth,
                    typename =std::enable_if_t<(current_depth < depth and current_depth >= 1)> >
            inline void init_rek() {
                // get parent iterator
                auto &iter = std::get<current_depth - 1>(iters);
                auto &end = std::get<current_depth - 1>(ends);
                childen_t<current_depth + 1> &parent_it = std::get<current_depth>(iters);


                compressed_child_type current_child = parent_it->second;

                if (current_child.getTag() == compressed_child_type::INT_TAG) {
                    key[depth - 1] = current_child.getInt();
                } else {
                    Node<1> *current_boolhypertrie = current_child.getPointer();
                    iter = current_boolhypertrie->edges.cbegin();
                    end = current_boolhypertrie->edges.cend();
                    key[depth - 1] = *iter;
                }
            }

            inline void inc_rek() {
                bool inc_done = inc_rek < 1 > ();
                if (not inc_done) {
                    auto &iter = std::get<depth - 1>(iters);
                    auto &end = std::get<depth - 1>(ends);
                    ++iter;
                    if (iter != end) {
                        key[0] = iter->first;
                        init_rek<depth - 1>();
                    } else {
                        ended_ = true;
                    }
                }
            }

            template<pos_type current_depth,
                    typename =std::enable_if_t<(current_depth == 1)> >
            inline bool inc_rek() {
                childen_t<current_depth + 1> &parent_it = std::get<current_depth>(iters);
                compressed_child_type current_parent_child = parent_it->second;
                if (current_parent_child.getTag() == compressed_child_type::INT_TAG) {
                    return false;
                } else {
                    // get the iterator
                    auto &iter = std::get<0>(iters);
                    auto &end = std::get<0>(ends);
                    // increment it
                    ++iter;
                    // check if it is still valid
                    if (iter != end) {
                        key[depth - 1] = *iter;
                        return true;
                    } else {
                        return false;
                    }
                }
            }
        };

        using const_iterator = iterator;

        iterator begin() const noexcept { return {this}; }

        const_iterator cbegin() const noexcept { return {this}; }


        bool end() const noexcept { return false; }

        bool cend() const noexcept { return false; }

    };

    // Node type (depth == 3).
    template<pos_type depth_t, typename key_part_type_t, template<typename, typename> typename map_type_t,
            template<typename> typename set_type_t>
    class RawCompressedBoolHypertrie<depth_t, key_part_type_t, map_type_t, set_type_t, true, typename std::enable_if_t<(
            depth_t > 2)>> {
    protected:
        template<pos_type depth_tt, bool compressed_tt>
        using rawCompressedboolhypertrie_c = RawCompressedBoolHypertrie<depth_tt, key_part_type_t, map_type_t, set_type_t, compressed_tt>;
    public:
        using Key = RawKey<depth_t, key_part_type_t>;
        using SliceKey = typename rawCompressedboolhypertrie_c<depth_t, false>::SliceKey;
        static constexpr pos_type depth = depth_t;
    protected:
        using root = rawCompressedboolhypertrie_c<depth_t, false>;

        template<pos_type depth_k>
        using Node = rawCompressedboolhypertrie_c<depth_k, false>;

        template<pos_type depth_k>
        using CompressedNode = rawCompressedboolhypertrie_c<depth_k, true>;

        template<pos_type depth_m>
        using NodePointer = util::CompressedBoolHyperTrieTaggedPointer<CompressedNode<depth_m> *, Node<depth_m> *, 8>;

        using ChildNode = Node<depth - 1>;
        using CompressedChildNode = CompressedNode<depth - 1>;
    public:
        using child_type = NodePointer<depth_t - 1>;

    public:
        // Set a new key in the CompressedBoolHypertrie - ignoring the bool value here
        void set(const Key &key) {

        }

        [[nodiscard]]
        child_type get(pos_type position, key_part_type_t key_part) const;

        template<pos_type slice_count, typename  = typename std::enable_if_t<((slice_count >= 0) and
                                                                              (slice_count < depth))>>
        [[nodiscard]]
        auto operator[](const SliceKey &key) const -> std::conditional_t<(slice_count >
                                                                          0), NodePointer<slice_count>, bool> const {
            if constexpr (slice_count > 0) {
                return {};
            } else {
                return false;
            }
        }

        [[nodiscard]]
        bool diagonal(const key_part_type_t &key_part) const;

        [[nodiscard]]
        inline bool operator[](const Key &key) const {
            return false;
        }

        [[nodiscard]]
        bool empty() const {
            return false;
        }

        [[nodiscard]]
        std::vector<size_t> getCards(const std::vector<pos_type> &positions) const;

        [[nodiscard]]
        std::vector<pos_type>::iterator
        minCardPos(std::vector<pos_type> &positions, const util::PosCalc *posCalc) const;

        [[nodiscard]]
        pos_type minCardPos() const;

        [[nodiscard]]
        std::vector<pos_type>::iterator minCardPos(std::vector<pos_type> &positions) const;

        [[nodiscard]]
        pos_type minCompressedCardPos() const;

        template<pos_type diag_depth, typename  = typename std::enable_if_t<((diag_depth > 0) and
                                                                             (diag_depth <= depth))>>
        [[nodiscard]]
        auto
        diagonal(const std::vector<pos_type> &positions, const key_part_type_t &key_part) const -> std::conditional_t<(
                depth != diag_depth), NodePointer<depth - diag_depth>, bool>;

        size_t size() const;
    };

    // Node type (depth == 3). Here the solo child of a key is comressed into the same node as a sequence of a static array
    template<pos_type depth_t, typename key_part_type_t, template<typename, typename> typename map_type_t,
            template<typename> typename set_type_t>
    class RawCompressedBoolHypertrie<depth_t, key_part_type_t, map_type_t, set_type_t, false, typename std::enable_if_t<(
            depth_t > 2)>> {

        template<pos_type, pos_type, bool, typename, template<typename, typename> typename,
                template<typename> typename, typename>
        friend
        class RawCompressedBHTHashDiagonal;

    public:
        using Key = RawKey<depth_t, key_part_type_t>;
        using SliceKey = RawSliceKey<depth_t, key_part_type_t>;
        static constexpr pos_type depth = depth_t;

    protected:
        template<pos_type depth_tt, bool compressed_tt>
        using rawCompressedboolhypertrie_c = RawCompressedBoolHypertrie<depth_tt, key_part_type_t, map_type_t, set_type_t, compressed_tt>;

        using root = rawCompressedboolhypertrie_c<depth_t, false>;

        template<pos_type depth_k>
        using Node = rawCompressedboolhypertrie_c<depth_k, false>;

        template<pos_type depth_k>
        using CompressedNode = rawCompressedboolhypertrie_c<depth_k, true>;

    public:
        template<pos_type depth_m>
        using NodePointer = util::CompressedBoolHyperTrieTaggedPointer<CompressedNode<depth_m> *, Node<depth_m> *, 8>;

    protected:
        using ChildNode = Node<depth - 1>;
        using CompressedChildNode = CompressedNode<depth - 1>;

    public:
        // make the bool hypertries friends to each other
        template<pos_type, typename, template<typename, typename> typename,
                template<typename> typename, bool, typename>
        friend
        class RawCompressedBoolHypertrie;

        typedef key_part_type_t key_part_type;
        template<typename key, typename value>
        using map_type = map_type_t<key, value>;
        template<typename key>
        using set_type = set_type_t<key>;
        //   using child_type = RawCompressedBoolHypertrie<depth_t - 1, key_part_type_t, map_type_t, set_type_t, false>;
        using compressed_child_type = compressed_children_array<key_part_type_t, depth_t - 1>;

        using children_type = map_type<key_part_type_t, ChildNode *>;
        using compressed_children_type = map_type<key_part_type_t, compressed_child_type>;

        using edges_type = std::array<children_type, depth_t>;
        using compressed_edges_type = std::array<compressed_children_type, depth_t>;

        using PosCalc = util::PosCalc;
        using subkey_mask_t = typename PosCalc::subkey_mask_t;
        using size_t = std::size_t;
    public:


        using child_type = NodePointer<depth_t - 1>;
    protected:
        // The node holds two edges types, one with compressed children and the other with pointer to child node
        // normal edges

        // compressed edges
        static constexpr bool is_tsl_map = std::is_same_v<map_type<int, int>, container::tsl_sparse_map<int, int>>;
        static constexpr bool is_tsl_set = std::is_same_v<set_type<int>, container::tsl_sparse_set<int>>;
        size_t _size;
    public:
        edges_type edges;
        compressed_edges_type compressed_edges;
    public:
        RawCompressedBoolHypertrie() = default;

        RawCompressedBoolHypertrie(RawCompressedBoolHypertrie &) = delete;

        RawCompressedBoolHypertrie(RawCompressedBoolHypertrie &&) noexcept = default;


    public:
        // Set a new key in the CompressedBoolHypertrie - ignoring the bool value here
        void set(const Key &key) {
            if (!this->operator[](key)) {
                // Prepare a starting point for recusive insetion of key
                const PosCalc *posCalc = PosCalc::getInstance(depth);

                static tsl::hopscotch_map<subkey_mask_t, void *> created_nodes(factorial(depth_t));

                _<depth_t>::setRek(*this, key, created_nodes, posCalc);
                created_nodes.clear();
            }
        }

        [[nodiscard]]
        child_type get(pos_type position, key_part_type_t key_part) const {
            auto found_compressed = compressed_edges[position].find(key_part);
            if (found_compressed != compressed_edges[position].end()) {
                return {new CompressedChildNode{found_compressed->second}};
            } else {
                auto found = edges[position].find(key_part);
                if (found != edges[position].end()) {
                    return {found->second};
                } else {
                    return {};
                }
            }
        }

        template<pos_type slice_count, typename  = typename std::enable_if_t<((slice_count >= 0) and
                                                                              (slice_count < depth))>>
        [[nodiscard]]
        auto operator[](const SliceKey &key) const {
            if constexpr (slice_count > 0) {
                auto[positions, key_parts] = extractPossAndKeyParts(key);
                const PosCalc *posCalc = PosCalc::getInstance(depth);
                return resolve<slice_count>(std::move(positions), std::move(key_parts), posCalc);
            } else {
                Key full_key;
                for (auto[slice, full] : iter::zip(key, full_key))
                    full = *slice;
                return this->operator[](full_key);
            }
        }

        // @TODO this is a wrong implementation ( I need to look in both children types)
        [[nodiscard]]
        bool diagonal(const key_part_type_t &key_part) const {
            pos_type min_pos = minCardPos();
            auto found_compressed_iter = compressed_edges[min_pos].find(key_part);
            // Look at the compressed edges first
            if (found_compressed_iter != compressed_edges[min_pos].end()) {
                compressed_child_type const &compressed_child = found_compressed_iter->second;
                for (size_t i = 0; i < compressed_child.size(); i++) {
                    if (compressed_child[i] != key_part) {
                        return false;
                    }
                }
                return true;
            } else {
                auto found = edges[min_pos].find(key_part);
                if (found != edges[min_pos].end())
                    return found->second->diagonal(key_part);
                else
                    return false;
            }
        }

        [[nodiscard]]
        inline bool operator[](const Key &key) const {

            pos_type min_card_pos = minCardPos();
            // We start to look in the compressed edges first
            typename compressed_children_type::const_iterator compressed_child_it = compressed_edges[min_card_pos].find(
                    key[min_card_pos]);
            if (compressed_child_it != compressed_edges[min_card_pos].end()) {
                compressed_child_type compressed_child = compressed_child_it->second;
                bool found = true;
                constexpr pos_type depth = depth_t;
                for (auto j = 0, k = 0; j < depth;) {
                    if (j == min_card_pos) {
                        j++;
                    } else {
                        if (compressed_child[k++] != key[j++]) {
                            found = false;
                            break;
                        }
                    }
                }
                return found;
            }

            if (min_card_pos >= 0) {
                auto child = get(min_card_pos, key[min_card_pos]);
                if (!child.isEmpty()) {
                    static typename ChildNode::Key next_key;
                    for (auto i = 0, j = 0; i < depth_t;)
                        if (i == min_card_pos) ++i;
                        else next_key[j++] = key[i++];
                    return (*child.getNode())[next_key];
                    // or return child->operator[](next_key);
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }

        [[nodiscard]]
        std::vector<size_t> getCards(const std::vector<pos_type> &positions) const {
            std::vector<size_t> cards(positions.size());
            for (auto i : iter::range(positions.size()))
                cards[i] = edges[positions[i]].size();
            return cards;
        }

        [[nodiscard]]
        std::vector<pos_type>::iterator minCardPos(std::vector<pos_type> &positions, const PosCalc *posCalc) const {
            auto pos_it = positions.begin();
            auto min_pos = pos_it;
            auto min_card = std::numeric_limits<typename children_type::size_type>::max();
            for (; pos_it != positions.end(); ++pos_it) {
                const auto &children = edges[posCalc->key_to_subkey_pos(*pos_it)];
                if (children.size() < min_card) {
                    min_card = children.size();
                    min_pos = pos_it;
                }
            }
            return min_pos;
        }

        [[nodiscard]]
        pos_type minCardPos() const {
            constexpr pos_type depth = depth_t;
            auto min_pos = 0;
            // @TODO @WARNING it may cause errors in large data set handling
            auto min_card = std::numeric_limits<typename children_type::size_type>::max() +
                            std::numeric_limits<typename compressed_children_type::size_type>::max();
            for (size_t i = 0; i < depth; i++) {
                auto current_card = edges[i].size() + compressed_edges[i].size();
                if (current_card < min_card) {
                    min_pos = i;
                    min_card = current_card;
                }
            }
            return min_pos;
        }

        [[nodiscard]]
        std::vector<pos_type>::iterator minCardPos(std::vector<pos_type> &positions) const {
            auto pos_it = positions.begin();
            auto min_pos = pos_it;
            auto min_card = std::numeric_limits<typename children_type::size_type>::max() +
                            std::numeric_limits<typename compressed_children_type::size_type>::max();
            for (; pos_it != positions.end(); ++pos_it) {
                auto current_card = edges[*pos_it].size() + compressed_edges[*pos_it].size();
                if (current_card < min_card) {
                    min_card = current_card;
                    min_pos = pos_it;
                }
            }
            return min_pos;
        }

        [[nodiscard]]
        pos_type minCompressedCardPos() const {
            auto min_pos = 0;
            auto min_card = std::numeric_limits<typename compressed_children_type::size_type>::max();
            for (const auto[i, children] : enumerate(compressed_edges)) {
                if (children.size() < min_card and children.size() > 0) {
                    min_card = children.size();
                    min_pos = i;
                }
            }
            return min_pos;
        }

        size_t size() const {
            return _size;
        }

        template<pos_type diag_depth, typename  = typename std::enable_if_t<((diag_depth > 0) and
                                                                             (diag_depth <= depth))>>
        [[nodiscard]]
        auto diagonal(const std::vector<pos_type> &positions, const key_part_type_t &key_part) const {
            assert(positions.size() == diag_depth);
            std::vector<bool> done(positions.size(), false);
            return this->template diagonal_rek<diag_depth>(positions, done, key_part);
        }

        [[nodiscard]]
        bool empty() const {
            return _size == 0;
        }

    protected:
        template<pos_type diag_depth, typename  = typename std::enable_if_t<((diag_depth > 0) and
                                                                             (diag_depth <= depth))>>
        [[nodiscard]]
        auto diagonal_rek(const std::vector<pos_type> &positions, std::vector<bool> &done,
                          const key_part_type_t &key_part) const
        -> std::conditional_t<(depth != diag_depth), NodePointer<depth - diag_depth>, bool> {
            std::size_t min_i = 0;
            auto min_size = std::numeric_limits<std::size_t>::max();
            std::size_t delta = 0;
            std::size_t delta_i = 0;
            for (auto i : iter::range(positions.size()))
                if (not done[i]) {
                    if (auto current_size =
                                edges[positions[i] - delta].size() + compressed_edges[positions[i] - delta].size();
                            current_size < min_size) {
                        min_i = i;
                        min_size = current_size;
                        delta_i = delta;
                    }
                } else {
                    delta += 1;
                    continue;
                }

            auto min_pos = positions[min_i] - delta_i;
            done[min_i] = true;
            auto found_compressed = compressed_edges[min_pos].find(key_part);
            if (found_compressed != compressed_edges[min_pos].end()) {
                if constexpr (diag_depth == 1) {
                    return {new CompressedChildNode{found_compressed->second}};
                } else {
                    auto *child_ptr = new CompressedChildNode{found_compressed->second};
                    return child_ptr->template diagonal_rek<diag_depth - 1>(positions, done, key_part);
                }
            }
            auto found = edges[min_pos].find(key_part);
            if (found != edges[min_pos].end()) {
                if constexpr (diag_depth == 1) {
                    return {found->second};
                } else {
                    return found->second->template diagonal_rek<diag_depth - 1>(positions, done, key_part);
                }
            }
            if constexpr (depth != diag_depth) return {};
            else return false;
        }

        template<pos_type slice_count>
        [[nodiscard]] auto
        resolve(std::vector<pos_type> &&positions, std::vector<key_part_type> &&key_parts, const PosCalc *posCalc) const
        -> NodePointer<slice_count> {
            auto pos_it = minCardPos(positions, posCalc);
            auto key_part_it = key_parts.begin() + std::distance(positions.begin(), pos_it);
            auto child = get(posCalc->key_to_subkey_pos(*pos_it), *key_part_it);

            if (!child.isEmpty()) {
                if constexpr (depth - 1 == slice_count) {
                    return child;
                } else {
                    auto nextPosCalc = posCalc->use(*pos_it);
                    positions.erase(pos_it);
                    key_parts.erase(key_part_it);
                    if (child.getTag() == NodePointer<slice_count>::COMPRESSED_TAG) {
                        return child.getCompressedNode()->template resolve<slice_count>(std::move(positions),
                                                                                        std::move(key_parts),
                                                                                        nextPosCalc);
                    } else {
                        return child.getNode()->template resolve<slice_count>(std::move(positions),
                                                                              std::move(key_parts),
                                                                              nextPosCalc);
                    }
                }
            } else {
                return {};
            }
        }

        [[nodiscard]]
        static auto extractPossAndKeyParts(const SliceKey &key) {
            std::vector<pos_type> positions;
            std::vector<key_part_type> key_parts;
            positions.reserve(key.size());
            for (const auto[position, key_part] : iter::enumerate(key))
                if (key_part.has_value()) {
                    positions.push_back(position);
                    key_parts.push_back(key_part.value());
                }
            return std::pair{positions, key_parts};
        }


        /**
         * This must only be used interally for setting key_parts
         * @param position
         * @param key_part
         * @return
         */
        [[nodiscard]]
        child_type get_unsafe(pos_type position, key_part_type_t key_part) {
            // Check the normal edges for a child for the given key_part at subkey position position
            auto found_compressed = compressed_edges[position].find(key_part);
            if (found_compressed != compressed_edges[position].end()) {
                return {new CompressedChildNode{found_compressed->second}};
            } else {
                auto found = edges[position].find(key_part);
                if (found != edges[position].end()) {
                    return {found->second};
                } else {
                    return {};
                }
            }
        }


    protected:
        template<pos_type current_depth, typename = typename std::enable_if_t<(depth_t >= 1 and current_depth <=
                                                                                                depth_t)>, typename DUMMY = void>
        struct _;

        // @TODO From the template declaration, we can see that only node with depth = 3 is defined. For node above depth = 3 , there is node _ struct that could handle them
        template<pos_type current_depth, typename DUMMY>
        struct _<current_depth, typename std::enable_if_t<(current_depth > 2 and depth_t == current_depth)>, DUMMY> {
            using child_ = typename root::template _<current_depth - 1>;

            using currentHypertrie = Node<current_depth>;

            inline static void setRek(currentHypertrie &current, const Key &key,
                                      tsl::hopscotch_map<subkey_mask_t, void *> &created_nodes,
                                      PosCalc const *posCalc) {


                typedef typename currentHypertrie::compressed_child_type compressed_child_type;
                typedef typename currentHypertrie::child_type child_type;
                typedef typename currentHypertrie::ChildNode child_node;
                // update the size of this node
                current._size += 1;
                // We iterate over the key parts we have and place them in the node
                for (pos_type key_pos: posCalc->getKeyPoss()) {
                    // Current key part
                    key_part_type_t key_part = key[key_pos];
                    // Subkey position (selecting the edge)
                    auto subkey_pos = posCalc->key_to_subkey_pos(key_pos);

                    const PosCalc *next_pos_calc = posCalc->use(key_pos);
                    // For the root node, we could ignore the search for an already created child node
                    //const auto &createdNode = created_nodes.find(next_pos_calc->getSubKeyMask());

                    child_type child = current.get_unsafe(subkey_pos, key_part);
                    if (child.isEmpty()) {
                        // Create a compressed child
                        compressed_child_type new_compressed_child;
                        for (size_t i = 0; i < new_compressed_child.size(); i++) {
                            new_compressed_child[i] = key[next_pos_calc->subkey_to_key_pos(i)];
                        }
                        current.compressed_edges[subkey_pos].insert(std::make_pair(key_part, new_compressed_child));
                    } else if (child.getTag() == child_type::NON_COMPRESSED_TAG) {
                        // the child exists ... there is no need to look at the compressed edges anymore
                        // ... and the subtrie starting with the child was not already finished:
                        child_::setRek(child.getNode(), key, created_nodes, next_pos_calc);
                        //if (createdNode == created_nodes.end()) {
                    } else if (child.getTag() == child_type::COMPRESSED_TAG) {
                        child_node *child_ptr = new child_node{};
                        current.edges[subkey_pos].insert(std::make_pair(key_part, child_ptr));
                        child_::setRek(child_ptr, key, created_nodes, next_pos_calc);
                        // reconstruct the key at position subkey_pos
                        Key reconstruced_compressed_key;
                        // we reconstruct the old compressed key stored
                        typename currentHypertrie::CompressedChildNode compressedChildNode = *child.getCompressedNode();
                        for (size_t i = 0; i < reconstruced_compressed_key.size(); i++) {
                            if (i == key_pos) {
                                reconstruced_compressed_key[i] = key_part;
                            } else {
                                reconstruced_compressed_key[i] = compressedChildNode.get(
                                        next_pos_calc->key_to_subkey_pos(
                                                i));
                            }
                        }
                        //std::cout << "Key(depth=3): " << key[0] << ","
                        //          << key[1] << "," << key[2] << std::endl;
                        //std::cout << "Reconstructed Key(depth=3): " << reconstruced_compressed_key[0] << ","
                        //          << reconstruced_compressed_key[1] << "," << reconstruced_compressed_key[2] << std::endl;
                        // remove the compressed key from compressed_edges[subkey_pos]
                        current.compressed_edges[subkey_pos].erase(key_part);
                        child_::setRek(child_ptr, reconstruced_compressed_key, created_nodes, next_pos_calc);
                    }

                }
            }

        };

        template<pos_type current_depth, typename DUMMY>
        struct _<current_depth, typename std::enable_if_t<(current_depth == 2)>, DUMMY> {
            using child_ = typename root::template _<current_depth - 1>;
            using CurrentHypertrie = Node<current_depth>;
            using ChildNode = typename CurrentHypertrie::ChildNode;

            inline static void setRek(CurrentHypertrie *current, const Key &key,
                                      tsl::hopscotch_map<subkey_mask_t, void *> &created_nodes,
                                      PosCalc const *posCalc) {
                // CurrentHypertrie current = curre
                using compressed_child_type_t = typename CurrentHypertrie::compressed_child_type;
                current->_size += 1;
                created_nodes[posCalc->getSubKeyMask()] = current;

                for (pos_type key_pos: posCalc->getKeyPoss()) {
                    const key_part_type_t key_part = key[key_pos];
                    const PosCalc *next_pos_calc = posCalc->use(key_pos);
                    auto subkey_pos = posCalc->key_to_subkey_pos(key_pos);

                    auto child_it = current->edges[subkey_pos].find(key_part);
                    // current->get
                    if (child_it != current->edges[subkey_pos].end()) {
                        compressed_child_type_t *child_pptr;
                        if constexpr (is_tsl_map) {
                            child_pptr = &child_it.value();
                        } else {
                            child_pptr = &child_it->second;
                        }
                        compressed_child_type_t &child = *child_pptr;
                        if (child.getTag() == compressed_child_type_t::POINTER_TAG) {
                            child_::setRek(child.getPointer(), key, created_nodes, next_pos_calc);
                        } else if (child.getTag() == compressed_child_type_t::INT_TAG) {
                            const auto &created_child_node = created_nodes.find(next_pos_calc->getSubKeyMask());
                            if (created_child_node != created_nodes.end()) {
                                auto created_child_node_ptr = static_cast<ChildNode *>(
                                        created_child_node->second);
                                child.setPointer(created_child_node_ptr);
                            } else {
                                Key reconstructed_key{};
                                for (size_t i = 0; i < reconstructed_key.size(); ++i) {
                                    if (posCalc->getSubKeyMask().at(i)) {
                                        reconstructed_key[i] = key[i];
                                    } else if (i == key_pos) {
                                        reconstructed_key[i] = key_part;
                                    } else {
                                        // std::cout << child.getInt();
                                        reconstructed_key[i] = child.getInt();
                                    }
                                }
                                auto child_node = new ChildNode{};
                                child.setPointer(child_node);
                                //std::cout << "Key: " << key[0] << "," << key[1] << "," << key[2] << "," << std::endl;
                                // std::cout << "Reconstructed Key: " << reconstructed_key[0] << ","
                                //           << reconstructed_key[1] << "," << reconstructed_key[2] << std::endl;
                                child_::setRek(child_node, key, reconstructed_key, created_nodes, next_pos_calc);
                            }
                        }
                    } else {
                        // a new compressed child node can be added
                        // we get the remaining key part first
                        key_part_type_t child_key_part = key[next_pos_calc->getKeyPoss()[0]];
                        // we set the compressed child
                        compressed_child_type_t compressed_child{child_key_part};
                        current->edges[subkey_pos].insert(std::make_pair(key_part, compressed_child));
                    }
                }
            }
        };

        template<typename DUMMY>
        struct _<1, typename std::enable_if_t<(true)>, DUMMY> {
            using CurrentHypertrie = Node<1>;

            inline static void setRek(CurrentHypertrie *current, const Key &key,
                                      tsl::hopscotch_map<subkey_mask_t, void *> &created_nodes,
                                      PosCalc const *posCalc) {
                // add it to the finished ( means updated ) nodes.
                if (created_nodes.find(posCalc->getSubKeyMask()) == created_nodes.end()) {
                    created_nodes[posCalc->getSubKeyMask()] = current;
                }
                //std::cout << "Key1(depth=1): " << key[0] << "," << key[1] << "," << key[2] << "," << std::endl;
                // set the entry in the set
                key_part_type_t key_part = key[posCalc->subkey_to_key_pos(0)];

                current->set(key_part);
            }

            inline static void
            setRek(CurrentHypertrie *current, const Key &key, const Key &reconstruced_key,
                   tsl::hopscotch_map<subkey_mask_t, void *> &created_nodes,
                   PosCalc const *posCalc) {
                // add it to the finished ( means updated ) nodes.
                if (created_nodes.find(posCalc->getSubKeyMask()) == created_nodes.end()) {
                    created_nodes[posCalc->getSubKeyMask()] = current;
                }
                //std::cout << "Key(depth=1): " << key[0] << "," << key[1] << "," << key[2] << "," << std::endl;
                //std::cout << "Reconstruced Key(depth=1): " << reconstruced_key[0] << "," << reconstruced_key[1] << "," << reconstruced_key[2] << "," << std::endl;
                // set the entry in the set
                key_part_type_t key_part = key[posCalc->subkey_to_key_pos(0)];
                key_part_type_t reconstruced_key_part = reconstruced_key[posCalc->subkey_to_key_pos(0)];

                current->set(key_part);
                current->set(reconstruced_key_part);
            }
        };

    public:
        class iterator {
            template<pos_type depth_>
            using childen_t  = typename Node<depth_>::children_type::const_iterator;

        public:
            using self_type =  iterator;
            using value_type = std::vector<key_part_type>;
        protected:
            Node<depth> const *const raw_boolhypertrie;

            typename Node<depth_t>::compressed_children_type::const_iterator compressed_iter;
            typename Node<depth_t>::compressed_children_type::const_iterator compressed_end;
            bool compressed_mode;
            util::CountDownNTuple<childen_t, depth> iters;
            util::CountDownNTuple<childen_t, depth> ends;
            std::vector<key_part_type> key;
            bool ended_;
            bool non_compressed_empty;
        public:

            iterator(Node<depth> const *const boolhypertrie)
                    : raw_boolhypertrie(boolhypertrie),
                      key(depth),
                      compressed_mode(false),
                      non_compressed_empty(false),
                      ended_{boolhypertrie->empty()} {
                if (not ended_)
                    init_rek();
            }

            iterator(Node<depth> const &raw_boolhypertrie) : iterator(&raw_boolhypertrie) {}

            inline self_type &operator++() {
                inc_rek();
                return *this;
            }

            static void inc(void *it_ptr) {
                auto &it = *static_cast<iterator *>(it_ptr);
                ++it;
            }

            inline const value_type &operator*() const { return key; }

            static const value_type &value(void const *it_ptr) {
                auto &it = *static_cast<iterator const *>(it_ptr);
                return *it;
            }

            inline operator bool() const { return not ended_; }

            static bool ended(void const *it_ptr) {
                auto &it = *static_cast<iterator const *>(it_ptr);
                return it.ended_;
            }

        protected:

            inline void init_rek() {
                // get the iterator
                auto &iter = std::get<depth - 1>(iters);
                iter = raw_boolhypertrie->edges[0].cbegin();
                auto &end = std::get<depth - 1>(ends);
                end = raw_boolhypertrie->edges[0].cend();

                compressed_iter = raw_boolhypertrie->compressed_edges[0].cbegin();
                compressed_end = raw_boolhypertrie->compressed_edges[0].cend();
                if (compressed_iter != compressed_end) {
                    compressed_mode = true;
                }
                if (iter == end) {
                    non_compressed_empty = true;
                }
                if (compressed_mode) {
                    key[0] = compressed_iter->first;
                    compressed_child_type compressed_child = compressed_iter->second;
                    std::copy(compressed_child.begin(), compressed_child.end(), key.begin() + 1);
                } else {
                    key[0] = std::get<depth - 1>(iters)->first;
                    init_rek<depth - 1>();
                }
            }

            template<pos_type current_depth,
                    typename =std::enable_if_t<(current_depth < depth and current_depth >= 1)> >
            inline void init_rek() {
                // get parent iterator
                auto &iter = std::get<current_depth - 1>(iters);
                auto &end = std::get<current_depth - 1>(ends);
                childen_t<current_depth + 1> &parent_it = std::get<current_depth>(iters);

                if constexpr (current_depth > 2) {
                    throw std::logic_error{"not implemented"};
                }
                if constexpr (current_depth == 2) {
                    Node<current_depth> const *current_rawBoolhypertrie = parent_it->second;
                    iter = current_rawBoolhypertrie->edges[0].cbegin();
                    end = current_rawBoolhypertrie->edges[0].cend();
                    key[depth - current_depth] = iter->first;
                    // set the key_part in the key
                    init_rek<current_depth - 1>();
                }
                if constexpr (current_depth == 1) {
                    using depth_2_compressed_child = typename Node<current_depth + 1>::compressed_child_type;
                    depth_2_compressed_child const &current_child = parent_it->second;
                    if (current_child.getTag() == depth_2_compressed_child::INT_TAG) {
                        key[depth - current_depth] = current_child.getInt();
                    } else {
                        Node<current_depth> *current_boolhypertrie = current_child.getPointer();
                        iter = current_boolhypertrie->edges.cbegin();
                        end = current_boolhypertrie->edges.cend();
                        key[depth - current_depth] = *iter;
                    }
                }
            }

            inline void inc_rek() {
                if (compressed_mode) {
                    compressed_iter++;
                    if (compressed_iter != compressed_end) {
                        key[0] = compressed_iter->first;
                        compressed_child_type compressed_child = compressed_iter->second;
                        std::copy(compressed_child.begin(), compressed_child.end(), key.begin() + 1);
                    } else {
                        compressed_mode = false;
                        if (not non_compressed_empty) {
                            key[0] = std::get<depth - 1>(iters)->first;
                            init_rek<depth - 1>();
                        } else {
                            ended_ = true;
                        }
                    }
                } else {
                    bool inc_done = inc_rek<depth - 1>();
                    if (not inc_done) {
                        auto &iter = std::get<depth - 1>(iters);
                        auto &end = std::get<depth - 1>(ends);
                        ++iter;
                        if (iter != end) {
                            key[0] = iter->first;
                            init_rek<depth - 1>();
                        } else {
                            ended_ = true;
                        }
                    }
                }
            }

            template<pos_type current_depth,
                    typename =std::enable_if_t<(current_depth < depth and current_depth >= 1)> >
            inline bool inc_rek() {
                if constexpr (current_depth == 2) {
                    bool inc_done = inc_rek<current_depth - 1>();
                    if (inc_done) {
                        return true;
                    } else {
                        auto &iter = std::get<current_depth - 1>(iters);
                        auto &end = std::get<current_depth - 1>(ends);
                        ++iter;
                        if (iter != end) {
                            init_rek<current_depth - 1>();
                            key[depth - current_depth] = iter->first;
                            return true;
                        } else {
                            return false;
                        }
                    }
                } else if constexpr (current_depth == 1) {
                    childen_t<current_depth + 1> &parent_it = std::get<current_depth>(iters);
                    using depth_2_compressed_child = typename Node<current_depth + 1>::compressed_child_type;
                    depth_2_compressed_child current_parent_child = parent_it->second;
                    if (current_parent_child.getTag() == depth_2_compressed_child::INT_TAG) {
                        return false;
                    } else {
                        // get the iterator
                        auto &iter = std::get<0>(iters);
                        auto &end = std::get<0>(ends);
                        // increment it
                        ++iter;
                        // check if it is still valid
                        if (iter != end) {
                            key[depth - 1] = *iter;
                            return true;
                        } else {
                            return false;
                        }
                    }
                } else {
                    throw std::logic_error{"not implemented"};
                }
            }
        };

        using const_iterator = iterator;

        iterator begin() const noexcept { return {this}; }

        const_iterator cbegin() const noexcept { return {this}; }


        bool end() const noexcept { return false; }

        bool cend() const noexcept { return false; }
    };


}


#endif // HYPERTRIE_BOOLCOMPRESSEDHYPERTRIE_IMPL_H