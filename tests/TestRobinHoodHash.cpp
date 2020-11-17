#include <catch2/catch.hpp>
#include <utility>
#include <string>
#include <array>
#include <vector>
#include <tuple>
#include <Dice/hypertrie/internal/util/RobinHoodHash.hpp>
namespace hypertrie::internal::robin_hood {

    template<typename T>
    size_t getHash(T const &parameter) {
        hash <T> hasher;
        return hasher(parameter);
    }

    bool equal(std::initializer_list<size_t> l) {
        return std::min(l) == std::max(l);
    }

// Helper to get the first type
    template<typename First, typename...>
    struct Head {
        using type = First;
    };
    template<typename... Args> using Head_t = typename Head<Args...>::type;

    template<typename... Args>
    concept types_all_equal =
    (std::conjunction_v<
            std::is_same<std::decay_t<Head_t<Args...>>, std::decay_t<Args>>...>);

    template<typename... Args>
    requires types_all_equal<Args...> bool test_str_vec_arr(Args &&... args) {
        size_t str = getHash(std::string({args...}));
        size_t vec = getHash(std::vector({args...}));
        size_t arr = getHash(std::array<Head_t<Args...>, sizeof...(Args)>({args...}));
        return equal({str, vec, arr});
    }

    template<typename... Args>
    requires types_all_equal<Args...> bool test_vec_arr(Args &&... args) {
        size_t vec = getHash(std::vector({args...}));
        size_t arr = getHash(std::array<Head_t<Args...>, sizeof...(Args)>({args...}));
        return vec == arr;
    }

    template<typename T, typename V>
    bool test_pair_tuple(T const& first, V const& second) {
        size_t p = getHash(std::pair< std::decay_t<T>, std::decay_t<V>>(first, second));
        size_t t = getHash(std::tuple<std::decay_t<T>, std::decay_t<V>>(first, second));
        return p == t;
    }

    TEST_CASE("Strings, vectors and arrays of char generate the same hash", "[RobinHoodHash]")  {
        REQUIRE(test_str_vec_arr('0', '1', '2', '3', '4', '5', '6', '7', '8'));
    }

    TEST_CASE("Vectors and arrays of basic types generate the same hash", "[RobinHoodHash]") {
        REQUIRE(test_vec_arr(1, 2, 3, 4, 5, 6, 7, 8));
        REQUIRE(test_vec_arr(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
    }

    TEST_CASE("Vectors and arrays of non-basic types generate the same hash", "[RobinHoodHash]") {
        REQUIRE(test_vec_arr(std::make_tuple(1, 2), std::make_tuple(3, 4), std::make_tuple(5, 6)));
    }

    TEST_CASE("Pairs and tuples (of basic and non-basic types) generate the same hash", "[RobinHoodHash]") {
        size_t first = 12;
        size_t second = 42;
        REQUIRE(test_pair_tuple(first, second));
        REQUIRE(test_pair_tuple(3.14159, 4.2));
        REQUIRE(test_pair_tuple(true, false));
        REQUIRE(test_pair_tuple(3.141, second));
        REQUIRE(test_pair_tuple('a', std::string("abc")));
    }

}