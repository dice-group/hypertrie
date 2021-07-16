#ifndef HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP
#define HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP

#include <Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp>
#include <catch2/catch.hpp>
#include <iostream>


namespace dev {
	template<template<std::integral auto> typename EntryTypeTemplate,
			 std::integral auto FIRST, std::integral auto LAST>
	class IntegralTemplatedTuple {
		static constexpr bool USE_SIGNED = FIRST < 0 or LAST < 0;
		using integral_type = std::conditional_t<USE_SIGNED, intmax_t, uintmax_t>;
		static constexpr integral_type MIN = std::min(FIRST, LAST);
		static constexpr integral_type MAX = std::max(FIRST, LAST);
		static constexpr uint64_t LENGTH = MAX + 1 - MIN;
		static constexpr enum class Direction : bool {
			up = true,
			down = false
		} DIRECTION = static_cast<const Direction>(FIRST <= LAST);

	public:
		template<integral_type N>
		using Entry = EntryTypeTemplate<N>;

	private:
		/** A helper struct to generate tuples of the Form (T<MIN>, T<MIN+1>,..., T<MAX>).
         * T uses an allocator to construct.
         * @tparam EntryTypeTemplate The type used in the tuple entries.
         * It must be a template of the Form T<INTEGER, ALLOCATOR>.
         * @tparam Allocator The allocator type to use for the entries.
         */
		struct TupleGenerator {
			/** Generates the tuple with gen_single based on entries of an integer_sequence.
             * @tparam IDS The indices itself.
             */
			template<integral_type... IDS>
			static auto gen_tuple(std::integer_sequence<integral_type, IDS...>) {
				if constexpr (DIRECTION == Direction::up)
					return std::make_tuple(Entry<integral_type(MIN + LENGTH - 1 - IDS)>{}...);
				else
					return std::make_tuple(Entry<integral_type(MAX - LENGTH + 1 + IDS)>{}...);
			}

			/** Wrapper for gen_tuple above.
             * @return See the other gen_tuple implementation.
             */
			static auto make_tuple() {
				return gen_tuple(std::make_integer_sequence<integral_type, LENGTH>());
			}

			using type = std::invoke_result_t<decltype(make_tuple)>;
		};

		typename TupleGenerator::type count_tuple_;

	public:
		explicit IntegralTemplatedTuple()
			: count_tuple_(TupleGenerator::make_tuple()) {}

	private:
		template<integral_type I>
		static constexpr size_t calcPos() {
			static_assert(MIN <= I && I <= MAX);
			constexpr size_t pos =
					(DIRECTION == Direction::up)
							? MAX - I
							: I - MIN;
			static_assert(0 <= pos and pos < LENGTH);
			return pos;
		}

	public:
		template<integral_type I>
		[[nodiscard]] constexpr Entry<I> &get() noexcept {
			return std::get<calcPos<I>()>(count_tuple_);
		}

		template<integral_type I>
		[[nodiscard]] constexpr const Entry<I> &get() const noexcept {
			return std::get<calcPos<I>()>(count_tuple_);
		}
	};





	template<template<std::integral auto, typename> typename EntryTypeTemplate,
			 std::integral auto FIRST, std::integral auto LAST, typename Allocator>
	class IntegralTemplatedTupleAlloc {
		static constexpr bool USE_SIGNED = FIRST < 0 or LAST < 0;
		using integral_type = std::conditional_t<USE_SIGNED, intmax_t, uintmax_t>;
		static constexpr integral_type MIN = std::min(FIRST, LAST);
		static constexpr integral_type MAX = std::max(FIRST, LAST);
		static constexpr uint64_t LENGTH = MAX + 1 - MIN;
		static constexpr enum class Direction : bool {
			up = true,
			down = false
		} DIRECTION = static_cast<const Direction>(FIRST <= LAST);

	public:
		template<integral_type N>
		using Entry = EntryTypeTemplate<N, Allocator>;

	private:
		/** A helper struct to generate tuples of the Form (T<MIN>, T<MIN+1>,..., T<MAX>).
         * T uses an allocator to construct.
         * @tparam EntryTypeTemplate The type used in the tuple entries.
         * It must be a template of the Form T<INTEGER, ALLOCATOR>.
         * @tparam Allocator The allocator type to use for the entries.
         */
		struct TupleGenerator {
			/** Generates the tuple with gen_single based on entries of an integer_sequence.
             * @tparam IDS The indices itself.
             */
			template<integral_type... IDS>
			static auto gen_tuple(Allocator const &alloc, std::integer_sequence<integral_type, IDS...>) {
				if constexpr (DIRECTION == Direction::up)
					return std::make_tuple(Entry<integral_type(MIN + LENGTH - 1 - IDS)>(alloc)...);
				else
					return std::make_tuple(Entry<integral_type(MAX - LENGTH + 1 + IDS)>(alloc)...);
			}

			/** Wrapper for gen_tuple above.
             * @return See the other gen_tuple implementation.
             */
			static auto make_tuple(Allocator const &alloc) {
				return gen_tuple(alloc, std::make_integer_sequence<integral_type, LENGTH>());
			}

			using type = std::invoke_result_t<decltype(make_tuple), Allocator>;
		};

		typename TupleGenerator::type count_tuple_;

	public:
		explicit IntegralTemplatedTupleAlloc(Allocator const &alloc)
			: count_tuple_(TupleGenerator::make_tuple(alloc)) {}

	private:
		template<integral_type I>
		static constexpr size_t calcPos() {
			static_assert(MIN <= I && I <= MAX);
			constexpr size_t pos =
					(DIRECTION == Direction::up)
							? MAX - I
							: I - MIN;
			static_assert(0 <= pos and pos < LENGTH);

			return pos;
		}

	public:
		template<integral_type I>
		[[nodiscard]] constexpr Entry<I> &get() noexcept {
			return std::get<calcPos<I>()>(count_tuple_);
		}

		template<integral_type I>
		[[nodiscard]] constexpr const Entry<I> &get() const noexcept {
			return std::get<calcPos<I>()>(count_tuple_);
		}
	};
}// namespace dev


namespace test_details {
    using dev::IntegralTemplatedTuple;
    using dev::IntegralTemplatedTupleAlloc;
    template<template<auto> typename T, auto FIRST, auto LAST>
	using old_IntegralTemplatedTuple = hypertrie::internal::util::IntegralTemplatedTuple<T, FIRST, LAST>;

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

	template<typename INTEGER, INTEGER MIN, typename ToPrint, INTEGER... IDS>
	std::ostream &printing(std::ostream &os, ToPrint const &toPrint, std::integer_sequence<INTEGER, IDS...>) {
		return (os << ... << toPrint.template get<MIN + IDS>());
	}

    template<template<auto> typename T, auto FIRST, auto LAST>
    std::ostream &operator<<(std::ostream &os, IntegralTemplatedTuple<T, FIRST, LAST> const &tup) {
        constexpr auto MIN = std::min(FIRST, LAST);
        constexpr auto MAX = std::max(FIRST, LAST);
        using INTEGER = std::remove_cv_t<decltype(MIN)>;
        return printing<INTEGER, MIN>(os, tup, std::make_integer_sequence<INTEGER, MAX - MIN + 1>{});
    }
    template<template<auto> typename T, auto FIRST, auto LAST>
    std::ostream &operator<<(std::ostream &os, old_IntegralTemplatedTuple<T, FIRST, LAST> const &tup) {
        constexpr auto MIN = std::min(FIRST, LAST);
        constexpr auto MAX = std::max(FIRST, LAST);
        using INTEGER = std::remove_cv_t<decltype(MIN)>;
        return printing<INTEGER, MIN>(os, tup, std::make_integer_sequence<INTEGER, MAX - MIN + 1>{});
    }
    template<template<auto N, typename Allocator> typename T,
            std::integral auto FIRST, std::integral auto LAST, typename Allocator>
    std::ostream &operator<<(std::ostream &os, IntegralTemplatedTupleAlloc<T, FIRST, LAST, Allocator> const &tup) {
        constexpr auto MIN = std::min(FIRST, LAST);
        constexpr auto MAX = std::max(FIRST, LAST);
        using INTEGER = std::remove_cv_t<decltype(MIN)>;
        return printing<INTEGER, MIN>(os, tup, std::make_integer_sequence<INTEGER, MAX - MIN + 1>{});
    }

}// namespace test_details
using namespace test_details;

template<size_t FIRST, size_t LAST>
void Unsigned_NonAllocVsAlloc(bool print = false) {
    old_IntegralTemplatedTuple<UnsignedNonAlloc, FIRST, LAST> old_na;
	IntegralTemplatedTuple<UnsignedNonAlloc, FIRST, LAST> na;
    IntegralTemplatedTupleAlloc<UnsignedAlloc, FIRST, LAST, std::allocator<int>> a(std::allocator<int>{});
    if (print) {
        std::cout << "allocate:  " << a << std::endl;
        std::cout << "non-alloc: " << na << std::endl;
        std::cout << "old-alloc: " << old_na << std::endl;
    }
    REQUIRE((equal<FIRST, LAST>(old_na, na) && equal<FIRST, LAST>(old_na, a)));
}
template<size_t FIRST, size_t LAST>
void Signed_NonAllocVsAlloc(bool print = false) {
    old_IntegralTemplatedTuple<UnsignedNonAlloc, FIRST, LAST> old_na;
    IntegralTemplatedTuple<SignedNonAlloc, FIRST, LAST> na;
    IntegralTemplatedTupleAlloc<SignedAlloc, FIRST, LAST, std::allocator<int>> a(std::allocator<int>{});
    if (print) {
        std::cout << "allocate:  " << a << std::endl;
        std::cout << "non-alloc: " << na << std::endl;
        std::cout << "old-alloc: " << old_na << std::endl;
    }
    REQUIRE((equal<FIRST, LAST>(old_na, na) && equal<FIRST, LAST>(old_na, a)));
}

TEST_CASE("Unsigned: Is same for single value at 0", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<0, 0>(); }
TEST_CASE("Unsigned: Is same for single value at 4", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<4, 4>(); }
TEST_CASE("Unsigned: Is same for multiple values starting at 0", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<0, 10>(); }
TEST_CASE("Unsigned: Is same for multiple values starting at 4", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<4, 20>(); }
TEST_CASE("Unsigned: Is same for multiple values counting down", "[TupleWithAllocator]") { Unsigned_NonAllocVsAlloc<4, 0>(); }

TEST_CASE("Signed: Is same for single value at 0", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<0, 0>(); }
TEST_CASE("Signed: Is same for single value at 4", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<4, 4>(); }
TEST_CASE("Signed: Is same for multiple values starting at 0", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<0, 10>(); }
TEST_CASE("Signed: Is same for multiple values starting at 4", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<4, 20>(); }
TEST_CASE("Signed: Is same for multiple values counting down", "[TupleWithAllocator]") { Signed_NonAllocVsAlloc<4, 0>(); }


#endif//HYPERTRIE_TESTINTEGRALTEMPLATEDTUPLEALLOC_HPP