#ifndef HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP
#define HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP

#include <catch2/catch.hpp>
#include <iostream>

namespace dev{
	/** A helper struct to generate tuples of the Form (T<FIRST>, T<FIRST+1>,..., T<LAST>).
	 * T uses an allocator to construct.
	 * @tparam EntryTypeTemplate The type used in the tuple entries.
	 * It must be a template of the Form T<INTEGER, ALLOCATOR>.
	 * @tparam Allocator The allocator type to use for the entries.
	 */
    template<template<auto N, typename Allocator> typename EntryTypeTemplate,
             typename Allocator>
    struct TupleGenerator {
		template <std::integral auto N>
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
		 * @tparam FIRST The start value of the N parameter of gen_single.
		 * @tparam INTEGER Type of the indices.
		 * @tparam IDS The indices itself.
		 * @param alloc The allocator to use.
		 * @return A tuple of entries.
		 */
		template <std::integral auto FIRST, typename INTEGER, INTEGER ...IDS>
		static auto gen_tuple(Allocator const& alloc, std::integer_sequence<INTEGER, IDS...>){
			return std::make_tuple(gen_single<FIRST+IDS>(alloc)...);
		}

		/** Wrapper for gen_tuple above.
		 * It simply creates a std::integer_sequence of the form (0,...,LAST-FIRST+1)
		 * and calls the other function with it.
		 * @tparam FIRST See the other gen_tuple implementation.
		 * @tparam LAST See the other gen_tuple implementation.
		 * @param alloc See the other gen_tuple implementation.
		 * @return See the other gen_tuple implementation.
		 */
		template<std::integral auto FIRST, std::integral auto LAST>
		requires std::is_same<decltype(FIRST), decltype(LAST)>::value
		static auto gen_tuple(Allocator const &alloc) {
			static_assert(FIRST < LAST);
			return gen_tuple<FIRST>(alloc, std::make_integer_sequence<decltype(FIRST), LAST-FIRST+1>());
		}

        template<std::integral auto FIRST, std::integral auto LAST>
        using type = std::invoke_result_t<decltype(gen_tuple<FIRST,LAST>), Allocator>;
    };


    template<template<std::integral auto N, typename Allocator> typename EntryTypeTemplate,
			 std::integral auto FIRST, std::integral auto LAST, typename Allocator>
	struct IntegralTemplatedTupleAlloc {
		using generator = TupleGenerator<EntryTypeTemplate, Allocator>;
		typename generator::template type<FIRST,LAST> count_up_tuple_;

        IntegralTemplatedTupleAlloc(Allocator const& alloc)
			:count_up_tuple_(generator::template gen_tuple<FIRST,LAST>(alloc)){}

		constexpr static auto abs(std::intptr_t a, std::intptr_t b) noexcept {
			return (a < b) ? (b-a) : (a-b);
		}

        template<auto I>
        constexpr auto &get() noexcept {
            constexpr const auto pos = (FIRST <= LAST) ? LAST - I : I - LAST;
            static_assert(pos >= 0 and pos <= abs(LAST, FIRST));
            return std::get<pos>(count_up_tuple_);
        }

        template<auto I>
        constexpr const auto &get() const noexcept {
            constexpr const auto pos = (FIRST <= LAST) ? LAST - I : I - LAST;
            static_assert(pos >= 0 and pos <= abs(LAST, FIRST));
            return std::get<pos>(count_up_tuple_);
        }
    };
}// namespace dev


template<size_t N, typename Allocator>
struct TestStruct {
	static constexpr size_t value = N;

	TestStruct(Allocator const &) {}

	friend std::ostream& operator<<(std::ostream& os, TestStruct const&) {
		return os << N << ' ';
	}
};

template <typename ...Args, size_t ...IDS>
std::ostream& printTuple(std::ostream &os, std::tuple<Args...> const& tuple, std::index_sequence<IDS...>) {
    return (os << ... << get<IDS>(tuple));
}

template<typename ...Args>
std::ostream& operator<<(std::ostream& os, std::tuple<Args...> const& tuple) {
	return printTuple(os, tuple, std::make_index_sequence<sizeof...(Args)>());
}

template <size_t N, typename Allocator>
using PrepTestStruct = TestStruct<N, Allocator>;

TEST_CASE("Single value can be constructed", "[TupleWithAllocator]") {
	constexpr size_t N = 4;
	auto test = dev::TupleGenerator<PrepTestStruct, std::allocator<int>>::
	        gen_single<N>(std::allocator<int>{});
	REQUIRE(test.value == N);
}

TEST_CASE("Tuple of values can be constructed", "[TupleWithAllocator]") {
    constexpr size_t FIRST = 4;
    constexpr size_t LAST = 8;
    auto [a, b, c, d, e]= dev::TupleGenerator<PrepTestStruct, std::allocator<int>>::
    gen_tuple<FIRST, LAST>(std::allocator<int>{});
	REQUIRE(a.value == FIRST);
    REQUIRE(b.value == FIRST+1);
    REQUIRE(c.value == FIRST+2);
    REQUIRE(d.value == FIRST+3);
    REQUIRE(e.value == FIRST+4);
}


template <typename>
struct A {
	template <typename>
	static auto f(int) {
		return 42;
	}
};

TEST_CASE("Wrapper can be constructed", "[TupleWithAllocator]") {
    constexpr size_t FIRST = 4;
    constexpr size_t LAST = 8;

	dev::IntegralTemplatedTupleAlloc<PrepTestStruct, FIRST, LAST, std::allocator<int>> test(std::allocator<int>{});

	std::cout << test.get<4>() << std::endl;
    std::cout << test.get<5>() << std::endl;
    std::cout << test.get<6>() << std::endl;
    std::cout << test.get<7>() << std::endl;
    std::cout << test.get<8>() << std::endl;
}

#endif//HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP
