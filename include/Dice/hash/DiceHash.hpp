#ifndef HYPERTRIE_DICEHASH_HPP
#define HYPERTRIE_DICEHASH_HPP

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

#include "Dice/hash/martinus_robinhood_hash.hpp"
#include "Dice/hash/Container_trait.h"

namespace Dice::hash {

	namespace detail {
		inline static constexpr std::size_t size_t_bits = 8 * sizeof(std::size_t);

		inline auto hash_bytes = &Dice::hash::martinus::hash_bytes;

		inline auto hash_combine = &Dice::hash::martinus::hash_combine;

		template<typename T>
		requires std::is_fundamental_v<std::decay_t<T>> or std::is_pointer_v<std::decay_t<T>>
		inline std::size_t hash_primitive(T x) noexcept {
			if (sizeof(std::decay_t<T>) == sizeof(size_t)) {
				return Dice::hash::martinus::hash_int(*reinterpret_cast<size_t const *>(&x));
			} else if (sizeof(std::decay_t<T>) > sizeof(size_t)) {
				return hash_bytes(&x, sizeof(x));
			} else {
				if constexpr (std::is_floating_point_v<std::decay_t<T>>) {
					return hash_bytes(&x, sizeof(x));
				} else {
					return Dice::hash::martinus::hash_int(static_cast<size_t>(x));
				}
			}
		}

		using HashState = Dice::hash::martinus::HashState;
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

	template<typename T>
	std::size_t dice_hash(T const &) noexcept {
		throw std::logic_error("Hash must be declared explicitly.");
	}

	template<typename T>
	requires std::is_fundamental_v<T> or std::is_pointer_v<T>
	std::size_t dice_hash(T const &fundamental) noexcept {
		return detail::hash_primitive(fundamental);
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
	std::size_t dice_hash(std::unique_ptr<T> const &ptr) noexcept {
		return dice_hash(ptr.get());
	}

	template<typename T>
	std::size_t dice_hash(std::shared_ptr<T> const &ptr) noexcept {
		return dice_hash(ptr.get());
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
			static_assert(!std::is_same_v<std::decay_t<T>, bool>,
						  "vector of booleans has a special implementation which results into errors!");
			return detail::hash_bytes(vec.data(), sizeof(T) * vec.size());
		} else {
			return dice_hash_ordered_container(vec);
		}
	}

	template<typename... Ts>
	std::size_t hash_and_combine(Ts &&...values) {
		return detail::hash_combine(std::initializer_list<std::size_t>{dice_hash(std::forward<Ts>(values))...});
	}

	namespace detail {
		template<typename... TupleArgs, std::size_t... ids>
		std::size_t hash_tuple(std::tuple<TupleArgs...> const &tuple, std::index_sequence<ids...> const &) {
			return hash_and_combine(std::get<ids>(tuple)...);
		}
	}

	template<typename... TupleArgs>
	std::size_t dice_hash(std::tuple<TupleArgs...> const &tpl) noexcept {
		return detail::hash_tuple(tpl, std::make_index_sequence<sizeof...(TupleArgs)>());
	}

	template<typename T, typename V>
	std::size_t dice_hash(std::pair<T, V> const &p) noexcept {
		return hash_and_combine(p.first, p.second);
	}

    template <typename T> requires is_ordered_container_v<T>
    std::size_t dice_hash(T const &container) {
        return dice_hash_ordered_container(container);
    }

    template <typename T> requires is_unordered_container_v<T>
    std::size_t dice_hash(T const &container) {
        return dice_hash_unordered_container(container);
    }

	template<typename Container>
	std::size_t dice_hash_ordered_container(Container const &container) {
		detail::HashState hash_state(container.size());
		std::size_t item_hash;
		for (const auto &item : container) {
			item_hash = dice_hash(item);
			hash_state.add(item_hash);
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

}// namespace Dice::hash


#endif//HYPERTRIE_DICEHASH_HPP