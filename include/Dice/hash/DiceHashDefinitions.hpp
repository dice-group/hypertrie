#ifndef HYPERTRIE_DICEHASHDEFINITIONS_HPP
#define HYPERTRIE_DICEHASHDEFINITIONS_HPP

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

#include "Dice/hash/Container_trait.h"

namespace Dice::hash {
template<typename T>
std::size_t dice_hash(T const &) noexcept {
    throw std::logic_error("Hash must be declared explicitly.");
}

template<typename T>
requires std::is_fundamental_v<T> or std::is_fundamental_v<std::decay_t<T>>
std::size_t dice_hash(T const &fundamental) noexcept;

template<typename CharT>
std::size_t dice_hash(std::basic_string<CharT> const &str) noexcept;

template<typename CharT>
std::size_t dice_hash(std::basic_string_view<CharT> const &sv) noexcept;

template<typename T>
std::size_t dice_hash(std::unique_ptr<T> const &ptr) noexcept;

template<typename T>
std::size_t dice_hash(std::shared_ptr<T> const &ptr) noexcept;

template<typename T, std::size_t N>
std::size_t dice_hash(std::array<T, N> const &arr) noexcept;

template<typename T>
std::size_t dice_hash(std::vector<T> const &vec) noexcept;

template<typename... TupleArgs>
std::size_t dice_hash(std::tuple<TupleArgs...> const &tpl) noexcept;

template<typename T, typename V>
std::size_t dice_hash(std::pair<T, V> const &p) noexcept;

template<typename T>
requires is_ordered_container_v<T>
std::size_t dice_hash(T const &container) noexcept;

template<typename T>
requires is_unordered_container_v<T>
std::size_t dice_hash(T const &container) noexcept;

template<typename T>
requires std::is_pointer_v<T>
std::size_t dice_hash(T const ptr) noexcept;


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

#endif //HYPERTRIE_DICEHASHDEFINITIONS_HPP
