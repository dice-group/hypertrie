#ifndef HYPERTRIE_DICEHASH_HPP
#define HYPERTRIE_DICEHASH_HPP
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <set>
#include <utility>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace dice::hash {
    namespace detail {
        using SizeT = uint64_t;

        template<typename T>
        T rotr(T x, unsigned k) {
            return (x >> k) | (x << (8U * sizeof(T) - k));
        }

        template<typename T>
        inline T unaligned_load(void const *ptr) noexcept {
            // using memcpy so we don't get into unaligned load problems.
            // compiler should optimize this very well anyways.
            T t;
            std::memcpy(&t, ptr, sizeof(T));
            return t;
        }
    }// namespace detail
    inline size_t hash_bytes(void const *ptr, size_t len) noexcept {
        static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
        static constexpr uint64_t seed = UINT64_C(0xe17a1465);
        static constexpr unsigned int r = 47;

        auto const *const data64 = static_cast<uint64_t const *>(ptr);
        uint64_t h = seed ^(len * m);

        size_t const n_blocks = len / 8;
        for (size_t i = 0; i < n_blocks; ++i) {
            auto k = detail::unaligned_load<uint64_t>(data64 + i);

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }

        auto const *const data8 =
                reinterpret_cast<uint8_t const *>(data64 + n_blocks);
        switch (len & 7U) {
            case 7:
                h ^= static_cast<uint64_t>(data8[6]) << 48U;
                [[fallthrough]];
            case 6:
                h ^= static_cast<uint64_t>(data8[5]) << 40U;
                [[fallthrough]];
            case 5:
                h ^= static_cast<uint64_t>(data8[4]) << 32U;
                [[fallthrough]];
            case 4:
                h ^= static_cast<uint64_t>(data8[3]) << 24U;
                [[fallthrough]];
            case 3:
                h ^= static_cast<uint64_t>(data8[2]) << 16U;
                [[fallthrough]];
            case 2:
                h ^= static_cast<uint64_t>(data8[1]) << 8U;
                [[fallthrough]];
            case 1:
                h ^= static_cast<uint64_t>(data8[0]);
                h *= m;
                [[fallthrough]];
            default:
                break;
        }

        h ^= h >> r;
        h *= m;
        h ^= h >> r;
        return static_cast<size_t>(h);
    }
    inline size_t hash_int(uint64_t x) noexcept {
        // inspired by lemire's strongly universal hashing
        // https://lemire.me/blog/2018/08/15/fast-strongly-universal-64-bit-hashing-everywhere/
        //
        // Instead of shifts, we use rotations so we don't lose any bits.
        //
        // Added a final multiplcation with a constant for more mixing. It is most
        // important that the lower bits are well mixed.
        auto h1 = x * UINT64_C(0xA24BAED4963EE407);
        auto h2 = detail::rotr(x, 32U) * UINT64_C(0x9FB21C651E98DF25);
        auto h = detail::rotr(h1 + h2, 32U);
        return static_cast<size_t>(h);
    }

    /** Combines two hashes to a new hash.
     * It is used in the (un-)ordered container functions. However this __will__ be replaced in the future.
     * @param a First hash.
     * @param b Second hash.
     * @return Combination of a and b.
     */
    inline std::size_t rh_combine_hashes(std::size_t a, std::size_t b) {
        return a xor b;
    }


    /** Calculates the Hash over an ordered container.
     * An example would be a vector, a map, an array or a list.
     * Needs a ForwardIterator in the Container-type, and an member type "value_type".
     *
     * @tparam Container The container type (vector, map, list, etc).
     * @param container The container to calculate the hash value of.
     * @return The combined hash of all Values inside of the container.
     */
    template<typename Container> size_t rh_hash_ordered_container(Container const& container);

    /** Calculates the Hash over an unordered container.
     * An example would be a unordered_map or an unordered_set.
     * CAUTION: This function is not evaluated yet! It's performance it not tested.
     * Also it is simply an xor on the data inside, because a specific layout of data cannot be assumed.
     * Needs a ForwardIterator in the Container-type, and an member type "value_type".
     *
     * @tparam Container The container type (unordered_map/set etc).
     * @param container The container to calculate the hash value of.
     * @return The combined hash of all Values inside of the container.
     */
    template<typename Container> size_t rh_hash_unordered_container(Container const& container);

    template<typename... Ts>
    std::size_t rh_hash_tuple(Ts &&...values);

    template <typename T>
    std::size_t dice_hash(T const &obj) noexcept{
        std::hash<T> hasher;
        auto result = hasher(obj);
        return hash_int(static_cast<detail::SizeT>(result));
    }

   template <typename CharT>
   std::size_t dice_hash(std::basic_string<CharT> const &str) noexcept {
       return hash_bytes(str.data(), sizeof(CharT) * str.size());
   }

   template <typename CharT>
   std::size_t dice_hash(std::basic_string_view<CharT> const &sv) noexcept {
       return hash_bytes(sv.data(), sizeof(CharT) * sv.size());
   }

   template <typename T>
   std::size_t dice_hash(T *ptr) noexcept {
       return hash_int(reinterpret_cast<detail::SizeT>(ptr));
   }

   template <typename T>
   std::size_t dice_hash(std::unique_ptr<T> const &ptr) noexcept {
       return hash_int(reinterpret_cast<detail::SizeT>(ptr.get()));
   }

   template <typename T>
   std::size_t dice_hash(std::shared_ptr<T> const &ptr) noexcept {
       return hash_int(reinterpret_cast<detail::SizeT>(ptr.get()));
   }

   template <typename T, std::size_t N>
   std::size_t dice_hash(std::array<T,N> const &arr) noexcept {
       if constexpr (std::is_fundamental_v<T>) {
           return hash_bytes(arr.data(), sizeof(T) * N);
       } else {
           return rh_hash_ordered_container(arr);
       }
   }

   template <typename T>
   std::size_t dice_hash(std::vector<T> const &vec) noexcept {
      if constexpr (std::is_fundamental_v<T>) {
          static_assert(!std::is_same_v<T, bool>,
                        "vector of booleans has a special implementation which results into errors!");
          return hash_bytes(vec.data(), sizeof(T) * vec.size());
      } else {
          return rh_hash_ordered_container(vec);
      }
   }

   template<typename... TupleArgs, std::size_t... ids>
   size_t tupleHash(std::tuple<TupleArgs...> const &tuple, std::index_sequence<ids...> const &) {
       return rh_hash_tuple(std::get<ids>(tuple)...);
   }

  template <typename... TupleArgs>
  std::size_t dice_hash(std::tuple<TupleArgs...> const &tpl) noexcept {
      return tupleHash(tpl, std::make_index_sequence<sizeof...(TupleArgs)>());
  }

  template <typename T, typename V>
  std::size_t dice_hash(std::pair<T,V> const &p) noexcept {
      return rh_hash_tuple(p.first, p.second);
  }

  template <typename Key, typename Value>
  std::size_t dice_hash(std::map<Key, Value> const& m) noexcept {
      return rh_hash_ordered_container(m);
  }

    template <typename Key, typename Value>
    std::size_t dice_hash(std::unordered_map<Key, Value> const& m) noexcept {
        return rh_hash_unordered_container(m);
    }

    template <typename T>
    std::size_t dice_hash(std::set<T> const& s) noexcept {
        return rh_hash_ordered_container(s);
    }

    template <typename T>
    std::size_t dice_hash(std::unordered_set<T> const& s) noexcept {
        return rh_hash_unordered_container(s);
    }


    // Helperfunctions
    template<typename... Ts>
    std::size_t rh_hash_tuple(Ts &&...values) {
        static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
        std::size_t h{};
        for (auto const &hash : {dice_hash(std::forward<Ts>(values))...})
            h = rh_combine_hashes(h, hash) * m;
        return h;
    }


    template<typename Container>
    size_t rh_hash_ordered_container(Container const& container) {
        static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
        std::size_t h{};
        for (auto const &it : container) {
            h = rh_combine_hashes(h, dice_hash(it)) * m;
        }
        return h;
    }

    template<typename Container>
    size_t rh_hash_unordered_container(Container const& container) {
        std::size_t h{};
        for (auto const &it : container) {
            h = rh_combine_hashes(h,dice_hash(it));
        }
        return h;
    }

    /** Override für std::hash. Muss besser gemacht werden, ich weiß.
     *
     */
    template <typename T>
    requires T::activate_stdhash::value
    struct Test {
        size_t operator()(T const &t) const noexcept {
            return dice_hash(t);
        }
    };


    /** Wrapper
     *
     * @tparam T
     */
    template <typename T>
    struct DiceHash {
        std::size_t operator()(T const &t) const noexcept {
            return dice_hash(t);
        }
    };

    template <typename T>
    requires T::activate_stdhash::value
    struct enableDice {
    };
}

namespace std {
    template <typename T>
    struct hash<dice::hash::enableDice<T>> {
        std::size_t operator()(T const &t) const noexcept {
            return dice::hash::dice_hash(t);
        }
    };
}

#endif //HYPERTRIE_DICEHASH_HPP