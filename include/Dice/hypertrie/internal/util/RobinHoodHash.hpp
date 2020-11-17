#ifndef HYPERTRIE_ROBINHOODHASH_HPP
#define HYPERTRIE_ROBINHOODHASH_HPP
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <set>
#include <type_traits>
#include <utility>
#include <map>
#include <unordered_map>
#include <unordered_set>

// all non-argument macros should use this facility. See
// https://www.fluentcpp.com/2019/05/28/better-macros-better-flags/

namespace hypertrie::internal::robin_hood {
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
		uint64_t h = seed ^ (len * m);

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

	// A thin wrapper around std::hash, performing an additional simple mixing step
	// of the result.
	template<typename T, typename Enable = void>
	struct hash : public std::hash<T> {
		size_t operator()(T const &obj) const noexcept(noexcept(
				std::declval<std::hash<T>>().operator()(std::declval<T const &>()))) {
			// call base hash
			auto result = std::hash<T>::operator()(obj);
			// return mixed of that, to be save against identity has
			return hash_int(static_cast<detail::SizeT>(result));
		}
	};

	template<typename T>
	std::size_t rh_hash(T const &value) {
		hash<T> hasher;
		return hasher(value);
	}

    /* not needed at the moment
   template<class... Ts>
   concept all_std_size_t =
           sizeof...(Ts) == 0 or// empty list
           // non-empty list ↓
           (std::is_same_v<std::decay_t<std::tuple_element_t<0, std::tuple<Ts...>>>, std::size_t> and                                // first one must be std::size_t
                    std::conjunction_v<std::is_same<std::decay_t<std::tuple_element_t<0, std::tuple<Ts...>>>, std::decay_t<Ts>>...>);// all must be equal to first

   template<typename... size_type>
   requires all_std_size_t<size_type...>
           std::size_t rh_combine_hashes(size_type &&...hashes) { //perfect forwarding, not move semantics!
       static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
       std::size_t h{};
       for (auto const &hash : {hashes...})
           h = (h xor hash) * m;
       return h;
   }
 */

	template<typename Container>
	size_t rh_combine_container(Container const& container) {
        static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
        hash<typename Container::value_type> hasher;
        std::size_t h{};
        auto end = container.end();
        for (auto it = container.begin(); it != end; ++it) {
            h = (h xor hasher(*it)) * m;
        }
        return h;
    }

	template<typename... Ts>
	std::size_t rh_combine(Ts &&...values) {
		static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
		std::size_t h{};
		for (auto const &hash : {rh_hash(std::forward<Ts>(values))...})
			h = (h xor hash) * m;
		return h;
	}

	template<typename CharT>
	struct hash<std::basic_string<CharT>> {
		size_t operator()(std::basic_string<CharT> const &str) const noexcept {
			return hash_bytes(str.data(), sizeof(CharT) * str.size());
		}
	};

	template<typename CharT>
	struct hash<std::basic_string_view<CharT>> {
		size_t operator()(std::basic_string_view<CharT> const &sv) const noexcept {
			return hash_bytes(sv.data(), sizeof(CharT) * sv.size());
		}
	};

	template<class T>
	struct hash<T *> {
		size_t operator()(T *ptr) const noexcept {
			return hash_int(reinterpret_cast<detail::SizeT>(ptr));
		}
	};

	template<class T>
	struct hash<std::unique_ptr<T>> {
		size_t operator()(std::unique_ptr<T> const &ptr) const noexcept {
			return hash_int(reinterpret_cast<detail::SizeT>(ptr.get()));
		}
	};

	template<class T>
	struct hash<std::shared_ptr<T>> {
		size_t operator()(std::shared_ptr<T> const &ptr) const noexcept {
			return hash_int(reinterpret_cast<detail::SizeT>(ptr.get()));
		}
	};

	template<typename Enum>
	struct hash<Enum, typename std::enable_if<std::is_enum<Enum>::value>::type> {
		size_t operator()(Enum e) const noexcept {
			using Underlying = typename std::underlying_type<Enum>::type;
			return hash<Underlying>{}(static_cast<Underlying>(e));
		}
	};

	// custom hashes
	// declarations and definitions might need to be split. If there is a tuple of maps for example, this code might not compile
    //check if fundamental type needed?
	template<typename T, std::size_t N>
	struct hash<std::array<T, N>> {
	private:
		static size_t arrayHash(std::array<T, N> const &arr) {
			static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
			std::size_t h{};
			for (const auto &item : arr)
				h = (h xor rh_hash(item)) * m;
			return h;
		};

	public:
		size_t operator()(std::array<T, N> const &arr) const noexcept {
			if constexpr (std::is_fundamental_v<T>)
				return hash_bytes(arr.data(), sizeof(T) * N);
			else {
				return arrayHash(arr);
			}
		}
	};

	template<typename T>
	struct hash<std::vector<T>> {
	private:
		static size_t vecHash(std::vector<T> const &vec) {
			static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
			std::size_t h{};
			for (const auto &item : vec)
				h = (h xor rh_hash(item)) * m;
			return h;
		};

	public:
		size_t operator()(std::vector<T> const &vec) const noexcept {
			if constexpr (std::is_fundamental_v<T>)
				return hash_bytes(vec.data(), sizeof(T) * vec.size());
			else {
				return vecHash(vec);
			}
		}
	};

	template<class... TupleArgs>
	struct hash<std::tuple<TupleArgs...>> {
	private:
		using TupleType = std::tuple<TupleArgs...>;

		template<std::size_t... ids>
		static size_t tupleHash(TupleType const &tuple, std::index_sequence<ids...> const &) {
			return rh_combine(std::get<ids>(tuple)...);
		};

	public:
		size_t operator()(TupleType const &tpl) const noexcept {
			return tupleHash(tpl, std::make_index_sequence<sizeof...(TupleArgs)>());
		}
	};

	//decay?
	template <typename T, typename V>
	struct hash<std::pair<T,V>> {
	    size_t operator()(std::pair<T,V> const &p) const noexcept {
	        return rh_combine(p.first, p.second);
	    }
	};

	template <typename T>
	struct hash<std::set<T>> {
	    size_t operator()(std::set<T> const &s) const noexcept {
	        return rh_combine_container(s);
	    }
	};

	template <typename Key, typename Value>
	struct hash<std::map<Key, Value>> {
	    size_t operator()(std::map<Key, Value> const& m) const noexcept {
	       return rh_combine_container(m);
	    }
	};

	template <typename Key, typename Value>
	struct hash <std::unordered_map<Key, Value>> {
	    size_t operator()(std::unordered_map<Key, Value> const& m) const noexcept {
	        hash<std::pair<Key, Value>> hasher;
	        size_t h{};
	        for (auto keyValuePair : m) {
	           h ^= hasher(keyValuePair);
	        }
	        return h;
	    }
	};

    template <typename T>
    struct hash <std::unordered_set<T>> {
        size_t operator()(std::unordered_set<T> const& s) const noexcept {
            hash<T> hasher;
            size_t h{};
            for (auto value : s) {
                h ^= hasher(value);
            }
            return h;
        }
    };


}// namespace hypertrie::internal::robin_hood

#endif//HYPERTRIE_ROBINHOODHASH_HPP