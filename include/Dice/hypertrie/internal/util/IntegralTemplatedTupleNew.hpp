#ifndef HYPERTRIE_INTEGRALTEMPLATEDTUPLENEW_HPP
#define HYPERTRIE_INTEGRALTEMPLATEDTUPLENEW_HPP

namespace hypertrie::internal::util {
	/* There is an older implementation at
	 * "include/Dice/hypertrie/internal/util/IntegralTemplatedTuple.hpp".
	 */
	namespace new_impl {
        /**
         * This class is a wrapper around a tuple std::tuple<T<FIRST> .. T<LAST>>.
         * FIRST is allowed to be smaller then LAST.
         * It allows access to the elements via get<i>() -> T<I>.
         * Elements are memory aligned from FIRST to LAST which means you can reinterpret
         * an IntegralTemplatedTuple<T,1,5> as an IntegralTemplatedTuple<T,1,3>
         * and will still be able to access the elements 1-3.
         * @tparam EntryTypeTemplate T in  the text above.
         * @tparam FIRST
         * @tparam LAST
         */
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
             */
			struct TupleGenerator {
				/** Generates the tuple based on entries of an integer_sequence.
                 * @tparam IDS The indices itself.
                 */
				template<integral_type... IDS>
				static auto gen_tuple(std::integer_sequence<integral_type, IDS...>) {
					if constexpr (DIRECTION == Direction::up)
						return std::make_tuple(Entry<integral_type(MIN + LENGTH - 1 - IDS)>{}...);
					else
						return std::make_tuple(Entry<integral_type(MAX - LENGTH + 1 + IDS)>{}...);
				}

				/** Wrapper for gen_tuple.
                 * @return The constructed tuple.
                 */
				static auto make_tuple() {
					return gen_tuple(std::make_integer_sequence<integral_type, LENGTH>());
				}

				/* CAUTION: has to be __after__ the make_tuple function.
				 * Also make_tuple isn't allowed to have overloads.
				 */
				using type = std::invoke_result_t<decltype(make_tuple)>;
			};

			typename TupleGenerator::type count_tuple_;

		public:
			explicit IntegralTemplatedTuple()
				: count_tuple_(TupleGenerator::make_tuple()) {}

		private:
			/** Because FIRST can be larger than LAST the indexing must change based on those values.
			 * This function does exactly that.
			 * @tparam I Index between FIRST and LAST or rather LAST and FIRST.
			 * @return The indexed value.
			 */
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
	}

    /** This is a more specialized version of IntegralTemplatedTuple that is able to handle an Allocator.
     * This class is a wrapper around a tuple std::tuple<T<FIRST> .. T<LAST>>.
     * FIRST is allowed to be smaller then LAST.
     * It allows access to the elements via get<i>() -> T<I>.
     * Elements are memory aligned from FIRST to LAST which means you can reinterpret
     * an IntegralTemplatedTuple<T,1,5> as an IntegralTemplatedTuple<T,1,3>
     * and will still be able to access the elements 1-3.
     * @tparam EntryTypeTemplate T in  the text above.
     * @tparam FIRST
     * @tparam LAST
     * @tparam Allocator
     */
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
            /** Generates the tuple based on entries of an integer_sequence.
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
        /** Because FIRST can be larger than LAST the indexing must change based on those values.
         * This function does exactly that.
         * @tparam I Index between FIRST and LAST or rather LAST and FIRST.
         * @return The indexed value.
         */
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

/* Possible improvements:
 * - Combine the two classes.
 * - Retype the second class to
 *     template<template<std::integral auto> typename EntryTypeTemplate,
 *       std::integral auto FIRST, std::integral auto LAST, typename Allocator>
 *   , because all entries get the same allocator type. So you could simply use an template alias before
 *   using this class.
 */

#endif//HYPERTRIE_INTEGRALTEMPLATEDTUPLENEW_HPP
