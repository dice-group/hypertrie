#ifndef HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP
#define HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP

#include <Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp>
#include <Dice/hypertrie/internal/util/IntegralTemplatedTupleNew.hpp>
#include <catch2/catch.hpp>
#include <iostream>

namespace test_details {
	namespace new_version {
		using hypertrie::internal::util::new_impl::IntegralTemplatedTuple;
	}// namespace new_version
	namespace old_version {
		using hypertrie::internal::util::IntegralTemplatedTuple;
	}

	/** Class used for result checking in the test cases.
	 * Should only be used via the create_x_check functions.
	 * @tparam T Integer type of the values.
	 * @tparam Values The expected values of the test.
	 */
	template<typename T, T... Values>
	struct CheckWrapper {
		template<T V>
		struct SingleCheckWrapper {
			static constexpr T value = V;
			friend std::ostream &operator<<(std::ostream &os, SingleCheckWrapper const &a) {
				return os << a.value << ' ';
			}
		};
		static auto generate() { return std::make_tuple(SingleCheckWrapper<Values>{}...); }
		std::invoke_result_t<decltype(generate)> values = generate();
		template<T Index>
		constexpr const auto &get() const noexcept {
			return std::get<Index>(values);
		}
	};
	/** Function used in the specific creation functions.
	 * Should only be used via the create_x_check functions.
	 * @tparam T Integer type of the values.
	 * @tparam Values The expected values of the test.
	 * @return
	 */
	template<typename T, T... Values>
	auto create_check() { return CheckWrapper<T, Values...>{}; }


	/** A type trait that collects information from the tested tuples to simplify the access of single values.
	 * ELEMENT_COUNT is equal to the number of elements in the tuple. It is useful for std::make_integer_sequence().
	 * INDEX() converts an index from (0,...,ELEMENT_COUNT-1) to (MIN,...,MAX), where MIN and MAX are used in the tuple.
	 * This unspecialized template cannot be instantiated.
	 * @tparam T
	 */
	template<typename T>
	struct TestTrait {
		template<typename>
		struct AlwaysFalse : std::false_type {};
		static_assert(AlwaysFalse<T>::value, "TestTrait is not defined for this type");
		static constexpr auto INDEX(auto Value);
		static constexpr size_t ELEMENT_COUNT{};
	};

	template<template<auto N> typename T,
			 std::integral auto FIRST, std::integral auto LAST, typename... Args>
	struct TestTrait<new_version::IntegralTemplatedTuple<T, FIRST, LAST, Args...>> {
		static constexpr auto MIN = std::min(FIRST, LAST);
		static constexpr auto MAX = std::max(FIRST, LAST);
		static constexpr auto INDEX(auto Value) { return MIN + Value; }
		static constexpr size_t ELEMENT_COUNT = MAX - MIN + 1;
	};
	template<template<auto N> typename T,
			 std::integral auto FIRST, std::integral auto LAST>
	struct TestTrait<old_version::IntegralTemplatedTuple<T, FIRST, LAST>> {
		static constexpr auto MIN = std::min(FIRST, LAST);
		static constexpr auto MAX = std::max(FIRST, LAST);
		static constexpr auto INDEX(auto Value) { return MIN + Value; }
		static constexpr size_t ELEMENT_COUNT = MAX - MIN + 1;
	};
	template<typename T, T... Values>
	struct TestTrait<CheckWrapper<T, Values...>> {
		static constexpr auto INDEX(auto Value) { return Value; }
		static constexpr size_t ELEMENT_COUNT = sizeof...(Values);
	};

	/** Test class template for types that do not use an allocator.
	 * Should only be used via the xNonAlloc aliases.
	 */
	template<typename INTEGER, INTEGER N>
	struct NonAlloc {
		static constexpr INTEGER value = N;
		friend std::ostream &operator<<(std::ostream &os, NonAlloc const &na) {
			return os << na.value << ' ';
		}
	};
	/** Test class template for types that do use an allocator.
     * Should only be used via the xAlloc aliases.
     */
	template<typename INTEGER, INTEGER N>
	struct Alloc {
		static constexpr INTEGER value = N;
		template<typename Allocator>
		Alloc(Allocator const &) {}
		friend std::ostream &operator<<(std::ostream &os, Alloc const &a) {
			return os << a.value << ' ';
		}
	};

	/* Aliases to use directly in the tests.
	 */
	template<size_t N>
	using UnsignedNonAlloc = NonAlloc<size_t, N>;
	template<short N>
	using SignedNonAlloc = NonAlloc<short, N>;
	template<size_t N>
	using UnsignedAlloc = Alloc<size_t, N>;
	template<short N>
	using SignedAlloc = Alloc<short, N>;
	/* Function wrapper to use directly in the tests.
	 */
	template<size_t... Values>
	auto create_unsigned_check() { return create_check<size_t, Values...>(); }
	template<short... Values>
	auto create_signed_check() { return create_check<short, Values...>(); }


	/* Helper function to check whether two tuples (or a tuple and a check) contain equal elements.
	 * The types need to specialize the TestTrait and they need to define get<INDEX>().
	 */
	template<typename INTEGER, INTEGER Index, typename FirstType, typename SecondType>
	bool equal(FirstType const &first, SecondType const &second) {
		return first.template get<TestTrait<FirstType>::INDEX(Index)>().value == second.template get<TestTrait<SecondType>::INDEX(Index)>().value;
	}
	template<typename INTEGER, typename FirstType, typename SecondType, INTEGER... IDS>
	bool equal(FirstType const &first, SecondType const &second, std::integer_sequence<INTEGER, IDS...>) {
		return (equal<INTEGER, IDS>(first, second) && ...);
	}
	template<typename INTEGER, typename FirstType, typename SecondType>
	bool equal(FirstType const &first, SecondType const &second) {
		static_assert(TestTrait<FirstType>::ELEMENT_COUNT == TestTrait<SecondType>::ELEMENT_COUNT);
		return equal<INTEGER>(first, second, std::make_integer_sequence<INTEGER, TestTrait<FirstType>::ELEMENT_COUNT>{});
	}

	/* Helper functions to print a tuple.
	 * The type needs to specialize the TestTrait and needs to define get<INDEX>().
	 */
	template<typename INTEGER, typename ToPrint, INTEGER... IDS>
	std::ostream &printing(std::ostream &os, ToPrint const &toPrint, std::integer_sequence<INTEGER, IDS...>) {
		return (os << ... << toPrint.template get<TestTrait<ToPrint>::INDEX(IDS)>());
	}
	template<typename INTEGER, typename ToPrint>
	std::ostream &printing(std::ostream &os, ToPrint const &toPrint) {
		return printing(os, toPrint, std::make_integer_sequence<INTEGER, TestTrait<ToPrint>::ELEMENT_COUNT>{});
	}
	/* Helper class to print a tuple directly via std::ostream.
	 */
	template<typename INTEGER, typename T>
	struct PrintingWrapper {
		T const &ref;
		friend std::ostream &operator<<(std::ostream &os, PrintingWrapper const &wrap) { return printing<INTEGER>(os, wrap.ref); }
	};
	/* Helper function to enable better type deduction.
	 */
	template<typename INTEGER, typename T>
	auto Printer(T const &t) { return PrintingWrapper<INTEGER, T>{t}; }


}// namespace test_details
using namespace test_details;


/** Test to check whether all the tuples create the result that the check describes.
 * Should only be used via the x_NonAllocVsAlloc functions.
 */
template<template<auto> typename NonAlloc, template<auto> typename Alloc, typename INTEGER, INTEGER FIRST, INTEGER LAST, typename Check>
void comparison_Test(Check const &check, bool print) {
	old_version::IntegralTemplatedTuple<NonAlloc, FIRST, LAST> old_na;
	new_version::IntegralTemplatedTuple<NonAlloc, FIRST, LAST> na;
	new_version::IntegralTemplatedTuple<Alloc, FIRST, LAST, std::allocator<int>> a(std::allocator<int>{});
	if (print) {
		size_t fill_width = 20;
		std::cout << std::left << std::setfill(' ')
				  << std::setw(fill_width) << "check:" << Printer<INTEGER>(check) << '\n'
				  << std::setw(fill_width) << "allocate:" << Printer<INTEGER>(a) << '\n'
				  << std::setw(fill_width) << "non-alloc:" << Printer<INTEGER>(na) << '\n'
				  << std::setw(fill_width) << "old-non-alloc:" << Printer<INTEGER>(old_na) << '\n';
	}
	REQUIRE((equal<INTEGER>(check, a) && equal<INTEGER>(check, na) && equal<INTEGER>(check, old_na)));
}

/** Test to check whether all the tuples create the result that the check describes if a reinterpret_cast is used on them.
 * Should only be used via the x_reinterpret_cast_Test functions.
 */
template<template<auto> typename NonAlloc, template<auto> typename Alloc, typename INTEGER, INTEGER FIRST, INTEGER LAST, INTEGER NFIRST, INTEGER NLAST, typename Check>
void reinterpret_cast_Test(Check const &check, bool print) {
	old_version::IntegralTemplatedTuple<NonAlloc, FIRST, LAST> old_na;
	new_version::IntegralTemplatedTuple<NonAlloc, FIRST, LAST> na;
	new_version::IntegralTemplatedTuple<Alloc, FIRST, LAST, std::allocator<int>> a(std::allocator<int>{});
	auto recast_old_na = reinterpret_cast<old_version::IntegralTemplatedTuple<NonAlloc, NFIRST, NLAST> *>(&old_na);
	auto recast_na = reinterpret_cast<new_version::IntegralTemplatedTuple<NonAlloc, NFIRST, NLAST> *>(&old_na);
	auto recast_a = reinterpret_cast<new_version::IntegralTemplatedTuple<Alloc, NFIRST, NLAST, std::allocator<int>> *>(&a);

	if (print) {
		size_t fill_width = 20;
		std::cout << std::left << std::setfill(' ');
		auto lambda_output = [fill_width](auto const &old_na, auto const &na, auto const &a) {
			std::cout << std::setw(fill_width) << "  allocate:" << Printer<INTEGER>(a) << '\n'
					  << std::setw(fill_width) << "  non-alloc:" << Printer<INTEGER>(na) << '\n'
					  << std::setw(fill_width) << "  old-non-alloc:" << Printer<INTEGER>(old_na) << '\n';
		};
		std::cout << "before recast:\n";
		lambda_output(old_na, na, a);
		std::cout << "after recast:\n";
		lambda_output(*recast_old_na, *recast_na, *recast_a);
		std::cout << std::setw(fill_width) << "check:" << Printer<INTEGER>(check) << '\n';
	}

	REQUIRE((equal<INTEGER>(check, *recast_a) && equal<INTEGER>(check, *recast_na) && equal<INTEGER>(check, *recast_old_na)));
}

template<size_t FIRST, size_t LAST, typename Check>
void Unsigned_NonAllocVsAlloc(Check const &check, bool print = false) {
	comparison_Test<UnsignedNonAlloc, UnsignedAlloc, size_t, FIRST, LAST>(check, print);
}

template<short FIRST, short LAST, typename Check>
void Signed_NonAllocVsAlloc(Check const &check, bool print = false) {
	comparison_Test<SignedNonAlloc, SignedAlloc, short, FIRST, LAST>(check, print);
}

template<size_t FIRST, size_t LAST, size_t NFIRST, size_t NLAST, typename Check>
void Unsigned_reinterpret_cast_Test(Check const &check, bool print = false) {
	reinterpret_cast_Test<UnsignedNonAlloc, UnsignedAlloc, size_t, FIRST, LAST, NFIRST, NLAST>(check, print);
}

template<short FIRST, short LAST, short NFIRST, short NLAST, typename Check>
void Signed_reinterpret_cast_Test(Check const &check, bool print = false) {
	reinterpret_cast_Test<SignedNonAlloc, SignedAlloc, short, FIRST, LAST, NFIRST, NLAST>(check, print);
}

TEST_CASE("Unsigned: Is same for single value at 0", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<0, 0>(create_unsigned_check<0>(), false); }
TEST_CASE("Unsigned: Is same for single value at 4", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<4, 4>(create_unsigned_check<4>(), false); }
TEST_CASE("Unsigned: Is same for multiple values starting at 0", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<0, 10>(create_unsigned_check<0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10>(), false); }
TEST_CASE("Unsigned: Is same for multiple values starting at 4", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<4, 10>(create_unsigned_check<4, 5, 6, 7, 8, 9, 10>(), false); }
TEST_CASE("Unsigned: Is same for multiple values counting down", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<4, 0>(create_unsigned_check<0, 1, 2, 3, 4>(), false); }

TEST_CASE("Unsigned: reinterpret_cast counting up", "[TupleWithAllocator]") { Unsigned_reinterpret_cast_Test<0, 3, 0, 2>(create_unsigned_check<0, 1, 2>(), false); }
TEST_CASE("Unsigned: reinterpret_cast counting down", "[TupleWithAllocator]") { Unsigned_reinterpret_cast_Test<3, 0, 3, 1>(create_unsigned_check<1, 2, 3>(), false); }

TEST_CASE("Signed: Is same for single value at 0", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<0, 0>(create_signed_check<0>(), false); }
TEST_CASE("Signed: Is same for single value at 4", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<4, 4>(create_signed_check<4>(), false); }
TEST_CASE("Signed: Is same for multiple values starting at 0", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<0, 10>(create_signed_check<0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10>(), false); }
TEST_CASE("Signed: Is same for multiple values starting at 4", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<4, 10>(create_signed_check<4, 5, 6, 7, 8, 9, 10>(), false); }
TEST_CASE("Signed: Is same for multiple values counting down", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<4, 0>(create_signed_check<0, 1, 2, 3, 4>(), false); }
TEST_CASE("Signed: negative values counting up work fine", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<-1, 3>(create_signed_check<-1, 0, 1, 2, 3>(), false); }
TEST_CASE("Signed: negative values counting down work fine", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<3, -1>(create_signed_check<-1, 0, 1, 2, 3>(), false); }

TEST_CASE("Signed: reinterpret_cast counting up", "[TupleWithAllocator]") { Signed_reinterpret_cast_Test<0, 3, 0, 2>(create_signed_check<0, 1, 2>(), false); }
TEST_CASE("Signed: reinterpret_cast counting up from negative", "[TupleWithAllocator]") { Signed_reinterpret_cast_Test<-1, 3, -1, 2>(create_signed_check<-1, 0, 1, 2>(), false); }
TEST_CASE("Signed: reinterpret_cast counting down", "[TupleWithAllocator]") { Signed_reinterpret_cast_Test<3, 0, 3, 1>(create_signed_check<1, 2, 3>(), false); }
TEST_CASE("Signed: reinterpret_cast counting down into negative", "[TupleWithAllocator]") { Signed_reinterpret_cast_Test<3, -2, 3, -1>(create_signed_check<-1, 0, 1, 2, 3>(), false); }


#endif//HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP