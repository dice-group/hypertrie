#ifndef HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP
#define HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP

#include <Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp>
#include <catch2/catch.hpp>
#include <iostream>


namespace dev {

	/** Helper for _inverter.
	 * Takes an INTEGER I and a sequence of INTEGERS and "inserts" I in front of the sequence
	 * @tparam INTEGER
	 * @tparam I
	 * @tparam IDS
	 * @return (I, IDS...)
	 */
    template <typename INTEGER, INTEGER I, INTEGER ...IDS>
    constexpr auto _combiner(std::integer_sequence<INTEGER, IDS...>) {
		return std::integer_sequence<INTEGER, IDS..., I>{};
    }

	/** Takes a integer_sequence and inverts its order.
	 */
    template <typename INTEGER, INTEGER I, INTEGER ...IDS>
    constexpr auto _inverter(std::integer_sequence<INTEGER, I, IDS...>) {
		if constexpr (sizeof...(IDS)==0) {
            return std::integer_sequence<INTEGER, I>{};
		} else {
			return _combiner<INTEGER, I>(_inverter(std::integer_sequence<INTEGER, IDS...>{}));
		}
	}

	/** Equivalent to std::make_integer_sequence, but it generates the sequence (I-1, I-2, ..., 0).
	 *
	 * @tparam INTEGER The type of the values in the sequence.
	 * @tparam I The maximum (-1) value. Needs to be non-negative.
	 * @return (I-1, I-2, ..., 0)
	 */
	template <typename INTEGER, INTEGER I>
    constexpr auto make_inverted_integer_sequence() {
		static_assert(I >= 0, "make_inverted_integer_sequence works only on non-negative values.");
		return _inverter(std::make_integer_sequence<INTEGER, I>{});
	}

    template <typename INTEGER, INTEGER I>
    constexpr auto make_integer_sequence() {
        static_assert(I >= 0, "make_integer_sequence works only on non-negative values.");
        return std::make_integer_sequence<INTEGER, I>();
    }


	/** A helper struct to generate tuples of the Form (T<MIN>, T<MIN+1>,..., T<MAX>).
	 * T uses an allocator to construct.
	 * @tparam EntryTypeTemplate The type used in the tuple entries.
	 * It must be a template of the Form T<INTEGER, ALLOCATOR>.
	 * @tparam Allocator The allocator type to use for the entries.
	 */
	template<template<auto N, typename Allocator> typename EntryTypeTemplate,
			 typename Allocator>
	struct TupleGenerator {
		template<std::integral auto N>
		using Entry = EntryTypeTemplate<N, Allocator>;

		/** Generates a single entry.
		 * This is needed to simplify the folding expression down below.
		 * @tparam N The number to construct the entry with.
		 * @param alloc The allocator to pass to the constructor of the entry.
		 * @return Constructed entry.
		 */
		template<std::integral auto N>
		static Entry<N> gen_single(Allocator const &alloc) {
			return Entry<N>(alloc);
		}

		/** Generates the tuple with gen_single based on entries of an integer_sequence.
		 *
		 * @tparam MIN The start value of the N parameter of gen_single.
		 * @tparam INTEGER Type of the indices.
		 * @tparam IDS The indices itself.
		 * @param alloc The allocator to use.
		 * @return A tuple of entries.
		 */
		template<std::integral auto MIN, typename INTEGER, INTEGER... IDS>
		static auto gen_tuple(Allocator const &alloc, std::integer_sequence<INTEGER, IDS...>) {
			return std::make_tuple(gen_single<MIN + IDS>(alloc)...);
		}

		/** Wrapper for gen_tuple above.
		 * It simply creates a std::integer_sequence of the form (0,...,MAX-MIN+1)
		 * and calls the other function with it.
		 * @tparam MIN See the other gen_tuple implementation.
		 * @tparam MAX See the other gen_tuple implementation.
		 * @param alloc See the other gen_tuple implementation.
		 * @return See the other gen_tuple implementation.
		 */
		template<std::integral auto MIN, std::integral auto MAX>
		requires std::is_same<decltype(MIN), decltype(MAX)>::value static auto gen_up_tuple(Allocator const &alloc) {
			static_assert(MIN <= MAX);
			return gen_tuple<MIN>(alloc, dev::make_integer_sequence<decltype(MIN), MAX - MIN + 1>());
		}
		template<std::integral auto MIN, std::integral auto MAX>
		using up_type = std::invoke_result_t<decltype(gen_up_tuple<MIN, MAX>), Allocator>;

		/**
		 *
		 * @tparam MIN
		 * @tparam MAX
		 * @param alloc
		 * @return
		 */
        template<std::integral auto MIN, std::integral auto MAX>
        requires std::is_same<decltype(MIN), decltype(MAX)>::value static auto gen_down_tuple(Allocator const &alloc) {
            static_assert(MIN <= MAX);
            return gen_tuple<MIN>(alloc, dev::make_inverted_integer_sequence<decltype(MIN), MAX - MIN + 1>());
        }
        template<std::integral auto MIN, std::integral auto MAX>
        using down_type = std::invoke_result_t<decltype(gen_down_tuple<MIN, MAX>), Allocator>;
	};


	template<template<std::integral auto N, typename Allocator> typename EntryTypeTemplate,
			 std::integral auto FIRST, std::integral auto LAST, typename Allocator>
	struct IntegralTemplatedTupleAlloc {
		static constexpr auto MIN = std::min(FIRST, LAST);
        static constexpr auto MAX = std::max(FIRST, LAST);
        static constexpr bool Use_up_type = FIRST <= LAST;
		using generator = TupleGenerator<EntryTypeTemplate, Allocator>;
		using tuple_type = std::conditional_t<Use_up_type,
                                  typename generator::template up_type<MIN, MAX>,
                                  typename generator::template down_type<MIN, MAX>>;
		tuple_type count_tuple_;

		static constexpr auto gen_tuple(Allocator const& alloc) {
			if constexpr (Use_up_type) {
				return generator::template gen_up_tuple<MIN, MAX>(alloc);
			} else{
                return generator::template gen_down_tuple<MIN, MAX>(alloc);
			}
		}


		IntegralTemplatedTupleAlloc(Allocator const &alloc)
			: count_tuple_(gen_tuple(alloc)) {}

		constexpr static auto abs(std::intptr_t a, std::intptr_t b) noexcept {
			return (a < b) ? (b - a) : (a - b);
		}

		template<auto I>
		static constexpr auto calcPos() {
			static_assert(MIN <= I && I <= MAX);
			return I - MIN;
		}

		template<auto I>
		constexpr auto &get() noexcept {
			return std::get<calcPos<I>()>(count_tuple_);
		}

		template<auto I>
		constexpr const auto &get() const noexcept {
			return std::get<calcPos<I>()>(count_tuple_);
		}
	};
}// namespace dev


namespace test_details {
    using dev::IntegralTemplatedTupleAlloc;
    using hypertrie::internal::util::IntegralTemplatedTuple;

    template<size_t N>
    struct NonAlloc {
        static constexpr size_t value = N;
        friend std::ostream &operator<<(std::ostream &os, NonAlloc const &na) {
            return os << na.value << ' ';
        }
    };

    template<size_t N, typename Allocator>
    struct Alloc {
        static constexpr size_t value = N;
        Alloc(Allocator const &) {}
        friend std::ostream &operator<<(std::ostream &os, Alloc const &a) {
            return os << a.value << ' ';
        }
    };

	template<typename INTEGER, INTEGER... IDS>
	void printSeq(std::integer_sequence<INTEGER, IDS...>) {
		std::vector<INTEGER> converted({IDS...});
		for (auto &i : converted) {std::cout << i << ", ";}
	}


	/*
	 * integer_sequence equality
	 */
	template<typename INTEGER, INTEGER I1, INTEGER... IDS1, INTEGER I2, INTEGER... IDS2>
	constexpr bool _equal(std::integer_sequence<INTEGER, I1, IDS1...>, std::integer_sequence<INTEGER, I2, IDS2...>) {
		bool result = (I1 == I2);
		if constexpr (sizeof...(IDS1) == 0 && sizeof...(IDS2) == 0) {return result;}
		else {
			return result && _equal(std::integer_sequence<INTEGER, IDS1...>{}, std::integer_sequence<INTEGER, IDS2...>{});
		}
	}

	template<typename INTEGER, INTEGER I1, INTEGER... IDS1, INTEGER I2, INTEGER... IDS2>
	constexpr bool equal(std::integer_sequence<INTEGER, I1, IDS1...> seq1, std::integer_sequence<INTEGER, I2, IDS2...> seq2) {
		if constexpr (sizeof...(IDS1) == sizeof...(IDS2)) {
			return _equal(seq1, seq2);
		} else {return false;}
	}

	/*
	 * Equality of NoAlloc and WithAlloc
	 */
    template<size_t Index, typename NoAlloc, typename WithAlloc>
    bool equal(NoAlloc const &na, WithAlloc &a) {
        return na.template get<Index>().value == a.template get<Index>().value;
    }

    template<size_t MIN, typename NoAlloc, typename WithAlloc, size_t... IDS>
    bool equal(NoAlloc const &na, WithAlloc &a, std::index_sequence<IDS...>) {
        return (equal<MIN + IDS>(na, a) && ...);
    }
    template<size_t FIRST, size_t LAST, typename NoAlloc, typename WithAlloc>
    bool equal(NoAlloc const &na, WithAlloc &a) {
        constexpr auto MIN = std::min(FIRST, LAST);
        constexpr auto MAX = std::max(FIRST, LAST);
        return equal<MIN>(na, a, std::make_index_sequence<MAX - MIN + 1>());
    }

    template<typename INTEGER, INTEGER MIN, typename ToPrint, INTEGER ...IDS>
	std::ostream& printing(std::ostream& os, ToPrint const& toPrint, std::integer_sequence<INTEGER, IDS...>) {
		return (os << ... << toPrint.template get<MIN+IDS>());
	}

    template<template<auto> typename T, auto FIRST, auto LAST>
    std::ostream& operator<<(std::ostream& os, IntegralTemplatedTuple<T, FIRST, LAST> const& tup) {
        constexpr auto MIN = std::min(FIRST, LAST);
        constexpr auto MAX = std::max(FIRST, LAST);
        using INTEGER = std::remove_cv_t<decltype(MIN)>;
        return printing<INTEGER, MIN>(os, tup, std::make_integer_sequence<INTEGER,MAX-MIN+1>{});
    }
    template<template<auto N, typename Allocator> typename T,
            std::integral auto FIRST, std::integral auto LAST, typename Allocator>
    std::ostream& operator<<(std::ostream& os, IntegralTemplatedTupleAlloc<T, FIRST, LAST, Allocator> const& tup) {
        constexpr auto MIN = std::min(FIRST, LAST);
        constexpr auto MAX = std::max(FIRST, LAST);
        using INTEGER = std::remove_cv_t<decltype(MIN)>;
        return printing<INTEGER, MIN>(os, tup, std::make_integer_sequence<INTEGER,MAX-MIN+1>{});
    }
}
using namespace test_details;

template<size_t FIRST, size_t LAST>
void NonAllocVsAlloc(bool print = false) {
	IntegralTemplatedTuple<NonAlloc, FIRST, LAST> na;
	IntegralTemplatedTupleAlloc<Alloc, FIRST, LAST, std::allocator<int>> a(std::allocator<int>{});
	if (print) {
		std::cout << a << std::endl;
		std::cout << na << std::endl;
	}
	REQUIRE(equal<FIRST, LAST>(na, a));
}

TEST_CASE("inverted_integer_sequence works", "[TupleWithAllocator]") {
    auto seq = dev::make_inverted_integer_sequence<size_t, 10>();
    auto check = std::integer_sequence<size_t, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0>{};
    REQUIRE(equal(seq, check));
}

TEST_CASE("Is same for single value at 0", "[TupleWithAllocator]") { NonAllocVsAlloc<0, 0>(); }
TEST_CASE("Is same for single value at 4", "[TupleWithAllocator]") { NonAllocVsAlloc<4, 4>();}
TEST_CASE("Is same for multiple values starting at 0", "[TupleWithAllocator]") { NonAllocVsAlloc<0, 10>();}
TEST_CASE("Is same for multiple values starting at 4", "[TupleWithAllocator]") { NonAllocVsAlloc<4, 20>();}
TEST_CASE("Is same for multiple values counting down", "[TupleWithAllocator]") { NonAllocVsAlloc<4, 0>(true);}





#endif//HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP