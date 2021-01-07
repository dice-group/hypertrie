#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <utility>
#include <string>
#include <array>
#include <vector>
#include <tuple>

#include <catch2/catch.hpp>

#include "Dice/hash/DiceHash.hpp"

namespace Dice::hash {
    template<typename T>
    size_t getHash(T const &parameter) {
        DiceHash<T> hasher;
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
    bool test_pair_tuple(T const &first, V const &second) {
        size_t p = getHash(std::pair<std::decay_t<T>, std::decay_t<V>>(first, second));
        size_t t = getHash(std::tuple<std::decay_t<T>, std::decay_t<V>>(first, second));
        return p == t;
    }

    TEST_CASE("Vectors and arrays of char generate the same hash", "[DiceHash]") {
        REQUIRE(test_vec_arr('0', '1', '2', '3', '4', '5', '6', '7', '8'));
    }

    TEST_CASE("Strings, vectors and arrays of char generate the same hash", "[DiceHash]") {
        REQUIRE(test_str_vec_arr('0', '1', '2', '3', '4', '5', '6', '7', '8'));
    }

    TEST_CASE("Vectors and arrays of int generate the same hash (basic type)", "[DiceHash]") {
        REQUIRE(test_vec_arr(1, 2, 3, 4, 5, 6, 7, 8));
    }

    TEST_CASE("Vectors and arrays of double generate the same hash (basic type)", "[DiceHash]") {
        REQUIRE(test_vec_arr(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
    }

    TEST_CASE("Vectors and arrays of tuples generate the same hash (non-basic type)", "[DiceHash]") {
        REQUIRE(test_vec_arr(std::make_tuple(1, 2), std::make_tuple(3, 4), std::make_tuple(5, 6)));
    }

    TEST_CASE("Pairs and tuples of size_t generate the same hash (basic type)", "[DiceHash]") {
        size_t first = 12;
        size_t second = 42;
        REQUIRE(test_pair_tuple(first, second));
    }

    TEST_CASE("Pairs and tuples of double generate the same hash (same types)", "[DiceHash]") {
        REQUIRE(test_pair_tuple(3.14159, 4.2));
    }

    TEST_CASE("Pairs and tuples of booleans generate the same hash (same types)", "[DiceHash]") {
        REQUIRE(test_pair_tuple(true, false));
    }

    TEST_CASE("Pairs and tuples of double and size_t generate the same hash (mixed types)", "[DiceHash]") {
        size_t second = 42;
        REQUIRE(test_pair_tuple(3.141, second));
    }

    TEST_CASE("Pairs and tuples of char and string generate the same hash (mixed types)", "[DiceHash]") {
        REQUIRE(test_pair_tuple('a', std::string("abc")));
    }

    TEST_CASE("set of strings compiles", "[DiceHash]") {
        std::set<std::string> exampleSet;
        exampleSet.insert("cat");
        exampleSet.insert("dog");
        exampleSet.insert("horse");
        getHash(exampleSet);
    }

    TEST_CASE("map of string -> int compiles", "[DiceHash]") {
        std::map<std::string, int> exampleMap;
        exampleMap["cat"] = 1;
        exampleMap["horse"] = 5;
        exampleMap["dog"] = 100;
        getHash(exampleMap);
    }

    TEST_CASE("unordered map of string ->int compiles", "[DiceHash]") {
        std::unordered_map<std::string, int> exampleMap;
        exampleMap["cat"] = 1;
        exampleMap["horse"] = 5;
        exampleMap["dog"] = 100;
        getHash(exampleMap);
    }

    TEST_CASE("unordered maps of string ->int are equal, if the entries are equal", "[DiceHash]") {
        std::vector<std::pair<std::string, int>> entries {{"cat",1}, {"horse", 5},{"dog", 100}};
        std::unordered_map<std::string, int> exampleMap1 (entries.begin(), entries.end());
        std::unordered_map<std::string, int> exampleMap2 (entries.rbegin(), entries.rend());
        REQUIRE(getHash(exampleMap1) == getHash(exampleMap2));
    }

    TEST_CASE("unordered set of integers compiles", "[DiceHash]") {
        std::vector<int> entries {1, 2, 42, 512};
        std::unordered_set<int> exampleSet(entries.begin(), entries.end());
        getHash(exampleSet);
    }

    TEST_CASE("unordered set of strings compiles", "[DiceHash]") {
        std::vector<std::string> entries {"cat", "dog", "horse"};
        std::unordered_set<std::string> exampleSet (entries.begin(), entries.end());
        getHash(exampleSet);
    }

    TEST_CASE("unordered sets of strings are equal if entries are equal", "[DiceHash]") {
        std::vector<std::string> entries {"cat", "dog", "horse"};
        std::unordered_set<std::string> exampleSet1(entries.begin(), entries.end());
        std::unordered_set<std::string> exampleSet2(entries.rbegin(), entries.rend());
        REQUIRE(getHash(exampleSet1) == getHash(exampleSet2));
    }

    TEST_CASE("Raw pointer hash themself, not the value pointed to", "[DiceHash]") {
        int i = 42;
        auto raw = &i;
        auto firstHash = getHash(raw);
        i = 43;
        auto secondHash = getHash(raw);
        REQUIRE(firstHash == secondHash);
    }

    TEST_CASE("Unique pointer hash the managed pointer, not the value pointed to", "[DiceHash]") {
        auto smartPtr = std::make_unique<int>(42);
        REQUIRE(getHash(smartPtr) == getHash(smartPtr.get()));
    }

    TEST_CASE("Shared pointer hash the managed pointer, not the value pointed to", "[DiceHash]") {
        auto smartPtr = std::make_shared<int>(42);
        REQUIRE(getHash(smartPtr) == getHash(smartPtr.get()));
    }

    TEST_CASE("Complicated types can be hashed (fix for the definition/declaration order bug)", "[DiceHash]") {
       int i = 42;
       int *first = &i;
       int *second = &i;
       getHash(std::make_tuple(first, second));
    }

    TEST_CASE("Fundamental types can be hashed", "[DiceHash]") {
       int i = 42;
       REQUIRE(getHash(i) == getHash(42));
    }

    /*
    TEST_CASE("If the hash is not defined for a specific type, it will not compile", "[DiceHash]") {
        struct NotImplementedHashType {};
        NotImplementedHashType test;
        getHash(test);
    }
     */

}