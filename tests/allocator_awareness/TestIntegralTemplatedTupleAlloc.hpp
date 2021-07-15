#ifndef HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP
#define HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP

#include <Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp>
#include <catch2/catch.hpp>
#include <iostream>

namespace dev {
	/** A helper struct to generate tuples of the Form (T<FIRST>, T<FIRST+1>,..., T<LAST>).
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
		 * @tparam FIRST The start value of the N parameter of gen_single.
		 * @tparam INTEGER Type of the indices.
		 * @tparam IDS The indices itself.
		 * @param alloc The allocator to use.
		 * @return A tuple of entries.
		 */
		template<std::integral auto FIRST, typename INTEGER, INTEGER... IDS>
		static auto gen_tuple(Allocator const &alloc, std::integer_sequence<INTEGER, IDS...>) {
			return std::make_tuple(gen_single<FIRST + IDS>(alloc)...);
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
		requires std::is_same<decltype(FIRST), decltype(LAST)>::value static auto gen_tuple(Allocator const &alloc) {
			static_assert(FIRST <= LAST);
			return gen_tuple<FIRST>(alloc, std::make_integer_sequence<decltype(FIRST), LAST - FIRST + 1>());
		}

		template<std::integral auto FIRST, std::integral auto LAST>
		using type = std::invoke_result_t<decltype(gen_tuple<FIRST, LAST>), Allocator>;
	};


	template<template<std::integral auto N, typename Allocator> typename EntryTypeTemplate,
			 std::integral auto FIRST, std::integral auto LAST, typename Allocator>
	struct IntegralTemplatedTupleAlloc {
		using generator = TupleGenerator<EntryTypeTemplate, Allocator>;
		typename generator::template type<FIRST, LAST> count_up_tuple_;

		auto gen_tuple(Allocator const& alloc) {
			if constexpr(FIRST <= LAST) {
				return generator::template gen_tuple<FIRST, LAST>(alloc);
			} else {
				return generator::template gen_tuple<LAST, FIRST>(alloc);
			}
		}

		IntegralTemplatedTupleAlloc(Allocator const &alloc)
			: count_up_tuple_(gen_tuple(alloc)) {}

		constexpr static auto abs(std::intptr_t a, std::intptr_t b) noexcept {
			return (a < b) ? (b - a) : (a - b);
		}

		template<auto I>
		static constexpr auto calcPos() {
			static_assert(FIRST <= I && I <= LAST);
			return abs(I, FIRST);
		}

		template<auto I>
		constexpr auto &get() noexcept {
			return std::get<calcPos<I>()>(count_up_tuple_);
		}

		template<auto I>
		constexpr const auto &get() const noexcept {
			return std::get<calcPos<I>()>(count_up_tuple_);
		}
	};
}// namespace dev


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


/* Helper functions to check if the two approaches result in the same tuple.
 * The std::make_index_sequence trick is used here.
 */
template<size_t Index, typename NoAlloc, typename WithAlloc>
bool equal(NoAlloc const &na, WithAlloc &a) {
	return na.template get<Index>().value == a.template get<Index>().value;
}

template<size_t FIRST, typename NoAlloc, typename WithAlloc, size_t... IDS>
bool equal(NoAlloc const &na, WithAlloc &a, std::index_sequence<IDS...>) {
	return (equal<FIRST + IDS>(na, a) && ...);
}
template<size_t FIRST, size_t LAST, typename NoAlloc, typename WithAlloc>
bool equal(NoAlloc const &na, WithAlloc &a) {
	return equal<FIRST>(na, a, std::make_index_sequence<LAST - FIRST + 1>());
}

using dev::IntegralTemplatedTupleAlloc;
using hypertrie::internal::util::IntegralTemplatedTuple;

template<size_t FIRST, size_t LAST>
void NonAllocVsAlloc() {
	IntegralTemplatedTuple<NonAlloc, FIRST, LAST> na;
	IntegralTemplatedTupleAlloc<Alloc, FIRST, LAST, std::allocator<int>> a(std::allocator<int>{});
	REQUIRE(equal<FIRST, LAST>(na, a));
}

TEST_CASE("Is same for single value at 0", "[TupleWithAllocator]") { NonAllocVsAlloc<0, 0>(); }
TEST_CASE("Is same for single value at 4", "[TupleWithAllocator]") { NonAllocVsAlloc<4, 4>(); }
TEST_CASE("Is same for multiple values starting at 0", "[TupleWithAllocator]") { NonAllocVsAlloc<0, 10>(); }
TEST_CASE("Is same for multiple values starting at 4", "[TupleWithAllocator]") { NonAllocVsAlloc<4, 20>(); }
//when I comment it in, my ram explodes?
//TEST_CASE("Is same for multiple values counting down", "[TupleWithAllocator]") { NonAllocVsAlloc<4, 0>(); }
TEST_CASE("Is same for multiple values starting at 4", "[TupleWithAllocator]") {
	constexpr size_t FIRST = 4;
    constexpr size_t LAST =  0;
    //IntegralTemplatedTupleAlloc<Alloc, FIRST, LAST, std::allocator<int>> a(std::allocator<int>{});
}

#endif//HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP
