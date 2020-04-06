#ifndef HYPERTRIE_GENERATENTUPLES_HPP
#define HYPERTRIE_GENERATENTUPLES_HPP

#include <array>
#include <random>
#include <itertools.hpp>

namespace hypertrie::tests::utils {
    namespace {
    }

    static inline auto defaultRandomNumberGenerator = std::mt19937_64{42};

    static inline void resetDefaultRandomNumberGenerator() {
        defaultRandomNumberGenerator = std::mt19937_64{42};
    }

    template<typename key_part_type = std::size_t>
    inline std::vector<std::vector<key_part_type>>
    generateNTuples(const std::size_t count, const std::size_t depth, const key_part_type max, const int multiple) {
        using Entry = std::vector<key_part_type>;
        // set up distributions
        std::vector<std::uniform_int_distribution<key_part_type>> distributions;
        distributions.reserve(count);
        for ([[maybe_unused]] auto j : iter::range(count))
            distributions.push_back(std::uniform_int_distribution<key_part_type>{0, max});

        // generate n-tuples
        std::vector<Entry> entries;
        entries.reserve(count);
        for ([[maybe_unused]] auto i : iter::range(count)) {
            Entry entry;
            entry.reserve(depth);
            for (auto j : iter::range(depth))
                entry.push_back(distributions[j](defaultRandomNumberGenerator) * multiple);
            entries.emplace_back(std::move(entry));
        }
        return entries;
    }


/**
 * Generates a std::array of length count that contains deterministic n-tuples. 
 * Calling the function twice will return exactly the same n-tuples. 
 * The range for the j-th entry of a n-triple is [0, J]
 * where J is ranges[j]. 
 * @tparam value_type  the type of the entries of the n-tuples
 * @tparam count the number of n-tuples
 * @tparam n the length of the tuples
 * @tparam ranges the maximum values for the positions of the n-tuples.
 * @return an array of n-tuples
 */
    template<typename value_type, std::size_t count, std::size_t n, typename tuple_type = std::array<value_type, n>>
    inline std::vector<tuple_type> generateNTuples(const std::array<std::size_t, n> &ranges, const int multiple) {
        // set up distributions
        std::array<std::uniform_int_distribution<value_type>, n> distributions;
        for (auto j : iter::range(n)) {
            distributions[j] = std::uniform_int_distribution<value_type>{1, ranges[j] + 1};
        }

        // generate n-tuples
        std::vector<tuple_type> entries;
        entries.reserve(count);
        for ([[maybe_unused]] auto i : iter::range(count)) {

            static tuple_type entry;
            if constexpr (not std::is_same_v<tuple_type, std::array<value_type, n>>)
                entry.resize(n);

            for (auto j : iter::range(n)) {
                // @TODO using shifting instead of muliplying by 8
                entry[j] = distributions[j](defaultRandomNumberGenerator) * multiple;
            }
            entries.emplace_back(entry);
        }
        return entries;
    }

    template<typename T, typename RandomNumberGenerator>
    struct Randoms {
        size_t size;
        T min;
        T max;
        RandomNumberGenerator &randomNumberGenerator;
        std::uniform_int_distribution<T> distribution;

        Randoms(size_t size, T min, T max, RandomNumberGenerator &randomNumberGenerator) :
                size(size),
                min(min),
                max(max),
                randomNumberGenerator(randomNumberGenerator),
                distribution{min, max} {}

        struct iterator {
            Randoms &container;
            T sample;
            size_t i;


            explicit iterator(Randoms &container, size_t i = 0) : container(container), i(i) {
                if (i != container.size)
                    sample = container.distribution(container.randomNumberGenerator);
            }

            T operator*() {
                return sample;
            }

            void operator++() {
                ++i;
                sample = container.distribution(container.randomNumberGenerator);
            }

            bool operator==(const iterator &other) { return i == other.i; }

            bool operator!=(const iterator &other) { return i != other.i; }
        };

        decltype(auto) begin() {
            return iterator(*this);
        }

        decltype(auto) end() {
            return iterator(*this, size);
        }

    };

    template<typename T, typename RandomNumberGenerator>
    decltype(auto) gen_random(const size_t &size, T min, T max, RandomNumberGenerator &randomNumberGenerator) {
        return Randoms{size, min, max, randomNumberGenerator};
    }

    template<typename T>
    decltype(auto) gen_random(const size_t &size, T min, T max) {
        return Randoms{size, min, max, defaultRandomNumberGenerator};
    }


}
#endif //HYPERTRIE_GENERATENTUPLES_HPP
