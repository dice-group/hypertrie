#ifndef HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP
#define HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP

#include <Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp>
#include <Dice/hypertrie/internal/util/IntegralTemplatedTupleNew.hpp>
#include <catch2/catch.hpp>
#include <iostream>

namespace test_details {
	namespace new_version {
        using hypertrie::internal::util::new_impl::IntegralTemplatedTuple;
        using hypertrie::internal::util::IntegralTemplatedTupleAlloc;
	}
	namespace old_version {
		using hypertrie::internal::util::IntegralTemplatedTuple;
	}

    template <typename T, T ...Values>
    struct CheckWrapper {
        template <T V>
        struct SingleCheckWrapper {
			static constexpr T value = V;
            friend std::ostream &operator<<(std::ostream &os, SingleCheckWrapper const &a) {
				return os << a.value << ' ';
			}
        };
        static auto generate() {return std::make_tuple(SingleCheckWrapper<Values>{}...);}
        std::invoke_result_t<decltype(generate)> values = generate();
        template <T Index>
        constexpr const auto &get() const noexcept{
            return std::get<Index>(values);
        }
    };
    template <typename T, T ...Values>
    auto create_check() {return CheckWrapper<T, Values...>{};}


    template <typename T>
    struct TestTrait {};
    template<template<auto N, typename Allocator> typename T,
            std::integral auto FIRST, std::integral auto LAST, typename Allocator>
    struct TestTrait<new_version::IntegralTemplatedTupleAlloc<T, FIRST, LAST, Allocator>> {
        static constexpr auto MIN = std::min(FIRST, LAST);
        static constexpr auto MAX = std::max(FIRST, LAST);
        static constexpr auto INDEX(auto Value) {return MIN + Value;}
        using LIST = decltype(std::make_index_sequence<MAX-MIN+1>());
    };
    template<template<auto N> typename T,
            std::integral auto FIRST, std::integral auto LAST>
    struct TestTrait<new_version::IntegralTemplatedTuple<T, FIRST, LAST>> {
        static constexpr auto MIN = std::min(FIRST, LAST);
        static constexpr auto MAX = std::max(FIRST, LAST);
        static constexpr auto INDEX(auto Value) {return MIN + Value;}
        using LIST = decltype(std::make_index_sequence<MAX-MIN+1>());
    };
    template<template<auto N> typename T,
            std::integral auto FIRST, std::integral auto LAST>
    struct TestTrait<old_version::IntegralTemplatedTuple<T, FIRST, LAST>> {
        static constexpr auto MIN = std::min(FIRST, LAST);
        static constexpr auto MAX = std::max(FIRST, LAST);
        static constexpr auto INDEX(auto Value) {return MIN + Value;}
        using LIST = decltype(std::make_index_sequence<MAX-MIN+1>());
    };
	template <typename T, T...Values>
	struct TestTrait<CheckWrapper<T,Values...>> {
        static constexpr auto INDEX(auto Value) {return Value;}
        using LIST = decltype(std::make_index_sequence<sizeof...(Values)>());
	};



	template<size_t N>
	struct UnsignedNonAlloc {
		static constexpr size_t value = N;
		friend std::ostream &operator<<(std::ostream &os, UnsignedNonAlloc const &na) {
			return os << na.value << ' ';
		}
	};

    template<short N>
    struct SignedNonAlloc {
        static constexpr short value = N;
        friend std::ostream &operator<<(std::ostream &os, SignedNonAlloc const &a) {
			return os << a.value << ' ';
		}
    };

	template<size_t N, typename Allocator>
	struct UnsignedAlloc {
		static constexpr size_t value = N;
        UnsignedAlloc(Allocator const &) {}
		friend std::ostream &operator<<(std::ostream &os, UnsignedAlloc const &a) {
			return os << a.value << ' ';
		}
	};

    template<short N, typename Allocator>
    struct SignedAlloc {
        static constexpr short value = N;
        SignedAlloc(Allocator const &) {}
        friend std::ostream &operator<<(std::ostream &os, SignedAlloc const &a) {return os << a.value << ' ';}
    };


    template <size_t ...Values>
    auto create_unsigned_check () {return create_check<size_t, Values...>();}
    template <short ...Values>
    auto create_signed_check () {return create_check<short, Values...>();}



	template<size_t Index, typename FirstType, typename SecondType>
	bool equal(FirstType const &first, SecondType const &second) {
		return first.template get<TestTrait<FirstType>::INDEX(Index)>().value == second.template get<TestTrait<SecondType>::INDEX(Index)>().value;
	}
    template<typename FirstType, typename SecondType, typename T, T... IDS>
	bool equal(FirstType const &first, SecondType const &second, std::integer_sequence<T, IDS...>) {
		return (equal<IDS>(first, second) && ...);
	}
	template<typename FirstType, typename SecondType>
	bool equal(FirstType const &first, SecondType const &second) {
		static_assert(std::is_same_v<typename TestTrait<FirstType>::LIST, typename TestTrait<SecondType>::LIST>);
		return equal(first, second, typename TestTrait<FirstType>::LIST{});
	}

    // BEGIN PRINTING
	template<typename ToPrint, size_t... IDS>
	std::ostream &printing(std::ostream &os, ToPrint const &toPrint, std::index_sequence<IDS...>) {
		return (os << ... << toPrint.template get<TestTrait<ToPrint>::INDEX(IDS)>());
	}
    template<typename ToPrint>
    std::ostream &printing(std::ostream &os, ToPrint const &toPrint) {
        return printing(os, toPrint, typename TestTrait<ToPrint>::LIST{});
	}

    template<template<auto> typename T, auto FIRST, auto LAST>
    std::ostream &operator<<(std::ostream &os, new_version::IntegralTemplatedTuple<T, FIRST, LAST> const &tup) {
        return printing(os, tup);
    }
    template<template<auto> typename T, auto FIRST, auto LAST>
    std::ostream &operator<<(std::ostream &os, old_version::IntegralTemplatedTuple<T, FIRST, LAST> const &tup) {
        return printing(os, tup);
    }
    template<template<auto N, typename Allocator> typename T,
            std::integral auto FIRST, std::integral auto LAST, typename Allocator>
    std::ostream &operator<<(std::ostream &os, new_version::IntegralTemplatedTupleAlloc<T, FIRST, LAST, Allocator> const &tup) {
		return printing(os, tup);
    }

    template<typename T, T...Args>
    std::ostream &operator<<(std::ostream &os, CheckWrapper<T, Args...> const& check) {
        return printing(os, check);
    }
	// END PRINTING


}// namespace test_details
using namespace test_details;



template <template <auto> typename NonAlloc, template <auto, typename> typename Alloc, size_t FIRST, size_t LAST, typename Check>
void comparison_Test(Check const& check, bool print) {
    old_version::IntegralTemplatedTuple<NonAlloc, FIRST, LAST> old_na;
    new_version::IntegralTemplatedTuple<NonAlloc, FIRST, LAST> na;
    new_version::IntegralTemplatedTupleAlloc<Alloc, FIRST, LAST, std::allocator<int>> a(std::allocator<int>{});
    if (print) {
		std::cout << "check:         " << check << std::endl;
        std::cout << "allocate:      " << a << std::endl;
        std::cout << "non-alloc:     " << na << std::endl;
        std::cout << "old-non-alloc: " << old_na << std::endl;
    }
    REQUIRE((equal(check, a) && equal(a, na) && equal(na, old_na)));
}

template<size_t FIRST, size_t LAST, typename Check>
void Unsigned_NonAllocVsAlloc(Check const& check, bool print = false) {
	comparison_Test<UnsignedNonAlloc, UnsignedAlloc, FIRST, LAST>(check, print);
}

template<size_t FIRST, size_t LAST, typename Check>
void Signed_NonAllocVsAlloc(Check const& check, bool print = false) {
    comparison_Test<SignedNonAlloc, SignedAlloc, FIRST, LAST>(check, print);
}

TEST_CASE("Unsigned: Is same for single value at 0", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<0, 0>(create_unsigned_check<0>()); }
TEST_CASE("Unsigned: Is same for single value at 4", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<4, 4>(create_unsigned_check<4>()); }
TEST_CASE("Unsigned: Is same for multiple values starting at 0", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<0, 10>(create_unsigned_check<0,1,2,3,4,5,6,7,8,9,10>()); }
TEST_CASE("Unsigned: Is same for multiple values starting at 4", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<4, 10>(create_unsigned_check<4,5,6,7,8,9,10>()); }
TEST_CASE("Unsigned: Is same for multiple values counting down", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<4, 0>(create_unsigned_check<0, 1, 2, 3, 4>()); }

TEST_CASE("Signed: Is same for single value at 0", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<0, 0>(create_signed_check<0>()); }
TEST_CASE("Signed: Is same for single value at 4", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<4, 4>(create_signed_check<4>()); }
TEST_CASE("Signed: Is same for multiple values starting at 0", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<0, 10>(create_signed_check<0,1,2,3,4,5,6,7,8,9,10>()); }
TEST_CASE("Signed: Is same for multiple values starting at 4", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<4, 10>(create_signed_check<4,5,6,7,8,9,10>()); }
TEST_CASE("Signed: Is same for multiple values counting down", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<4, 0>(create_signed_check<0,1,2,3,4>()); }


#endif//HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP