#ifndef HYPERTRIE_DICEHASH_HPP
#define HYPERTRIE_DICEHASH_HPP
#include "xxhash.hpp"
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace dice::hash {

	inline static constexpr uint64_t seed = UINT64_C(0xA24BAED4963EE407);

	namespace detail {
		inline static constexpr std::size_t size_t_bits = 8 * sizeof(std::size_t);

		inline std::size_t hash_bytes(void const *ptr, std::size_t len) noexcept {
			return xxh::xxhash3<size_t_bits>(ptr, len, seed);
		}

		inline std::size_t hash_int(uint64_t x) noexcept {
			return xxh::xxhash3<size_t_bits>(&x, sizeof(std::size_t), seed);
		}
	}// namespace detail


	/** Combines two hashes to a new hash.
     * It is used in the (un-)ordered container functions. However this __will__ be replaced in the future.
     * @param a First hash.
     * @param b Second hash.
     * @return Combination of a and b.
     */
	inline std::size_t dice_hash_invertible_combine(std::size_t a, std::size_t b) {
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
	template<typename Container>
	std::size_t dice_hash_ordered_container(Container const &container);

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
	template<typename Container>
	std::size_t dice_hash_unordered_container(Container const &container);

	template<typename... Ts>
	std::size_t dice_hash_combine(Ts &&...values);

	template<typename T>
	std::size_t dice_hash(T const &obj) noexcept {
		std::hash<T> hasher;
		auto result = hasher(obj);
		return detail::hash_int(static_cast<std::size_t>(result));
	}

	template<typename CharT>
	std::size_t dice_hash(std::basic_string<CharT> const &str) noexcept {
		return detail::hash_bytes(str.data(), sizeof(CharT) * str.size());
	}

	template<typename CharT>
	std::size_t dice_hash(std::basic_string_view<CharT> const &sv) noexcept {
		return detail::hash_bytes(sv.data(), sizeof(CharT) * sv.size());
	}

	template<typename T>
	std::size_t dice_hash(T *ptr) noexcept {
		return detail::hash_int(reinterpret_cast<std::size_t>(ptr));
	}

	template<typename T>
	std::size_t dice_hash(std::unique_ptr<T> const &ptr) noexcept {
		return detail::hash_int(reinterpret_cast<std::size_t>(ptr.get()));
	}

	template<typename T>
	std::size_t dice_hash(std::shared_ptr<T> const &ptr) noexcept {
		return detail::hash_int(reinterpret_cast<std::size_t>(ptr.get()));
	}

	template<typename T, std::size_t N>
	std::size_t dice_hash(std::array<T, N> const &arr) noexcept {
		if constexpr (std::is_fundamental_v<T>) {
			return detail::hash_bytes(arr.data(), sizeof(T) * N);
		} else {
			return dice_hash_ordered_container(arr);
		}
	}

	template<typename T>
	std::size_t dice_hash(std::vector<T> const &vec) noexcept {
		if constexpr (std::is_fundamental_v<T>) {
			static_assert(!std::is_same_v<T, bool>,
						  "vector of booleans has a special implementation which results into errors!");
			return detail::hash_bytes(vec.data(), sizeof(T) * vec.size());
		} else {
			return dice_hash_ordered_container(vec);
		}
	}

	template<typename... TupleArgs, std::size_t... ids>
	std::size_t tupleHash(std::tuple<TupleArgs...> const &tuple, std::index_sequence<ids...> const &) {
		return dice_hash_combine(std::get<ids>(tuple)...);
	}

	template<typename... TupleArgs>
	std::size_t dice_hash(std::tuple<TupleArgs...> const &tpl) noexcept {
		return tupleHash(tpl, std::make_index_sequence<sizeof...(TupleArgs)>());
	}

	template<typename T, typename V>
	std::size_t dice_hash(std::pair<T, V> const &p) noexcept {
		return dice_hash_combine(p.first, p.second);
	}

	template<typename Key, typename Value>
	std::size_t dice_hash(std::map<Key, Value> const &m) noexcept {
		return dice_hash_ordered_container(m);
	}

	template<typename Key, typename Value>
	std::size_t dice_hash(std::unordered_map<Key, Value> const &m) noexcept {
		return dice_hash_unordered_container(m);
	}

	template<typename T>
	std::size_t dice_hash(std::set<T> const &s) noexcept {
		return dice_hash_ordered_container(s);
	}

	template<typename T>
	std::size_t dice_hash(std::unordered_set<T> const &s) noexcept {
		return dice_hash_unordered_container(s);
	}


	// Helperfunctions
	template<typename... Ts>
	std::size_t dice_hash_combine(Ts &&...values) {
		return xxh::xxhash3<detail::size_t_bits>(std::initializer_list<uint64_t>{dice_hash(std::forward<Ts>(values))...});
	}


	template<typename Container>
	std::size_t dice_hash_ordered_container(Container const &container) {
		xxh::hash3_state_t<detail::size_t_bits> hash_state(seed);
		std::size_t item_hash;
		for (const auto &item : container) {
			item_hash = dice_hash(item);
			hash_state.update(&item_hash, sizeof(std::size_t));
		}
		return hash_state.digest();
	}

	template<typename Container>
	std::size_t dice_hash_unordered_container(Container const &container) {
		std::size_t h{};
		for (auto const &it : container) {
			h = dice_hash_invertible_combine(h, dice_hash(it));
		}
		return h;
	}


	/** Wrapper
     *
     * @tparam T
     */
	template<typename T>
	struct DiceHash {
		std::size_t operator()(T const &t) const noexcept {
			return dice_hash(t);
		}
	};

}// namespace dice::hash


#endif//HYPERTRIE_DICEHASH_HPP