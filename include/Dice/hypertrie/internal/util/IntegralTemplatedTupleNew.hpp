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
         * You can give the constructor parameters to pass through to the constructors of
         * the elements. However all elements will get exactly the same parameter.
         * You can use the function make_integral_template_tuple to create an
         * IntegralTemplatedTuple without explicitly listing the Args parameters.
         * @tparam EntryTypeTemplate T in the text above.
         * @tparam FIRST
         * @tparam LAST
         * @tparam Args Types of the constructor parameters.
         */
		template<template<std::integral auto> typename EntryTypeTemplate,
				 std::integral auto FIRST, std::integral auto LAST, typename... Args>
		class IntegralTemplatedTuple {
			static constexpr bool USE_SIGNED = FIRST < 0 or LAST < 0;
			using integral_type = std::conditional_t<USE_SIGNED, intmax_t, uintmax_t>;
			static constexpr integral_type MIN = std::min(integral_type(FIRST), integral_type(LAST));
			static constexpr integral_type MAX = std::max(integral_type(FIRST), integral_type(LAST));
			static constexpr uint64_t LENGTH = MAX + 1 - MIN;
			static constexpr enum class Direction : bool {
				up = true,
				down = false
			} DIRECTION = static_cast<const Direction>(integral_type(FIRST) <= integral_type(LAST));

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
				static auto gen_tuple(std::integer_sequence<integral_type, IDS...>, Args &&...args) {
					if constexpr (DIRECTION == Direction::up)
						return std::make_tuple(Entry<integral_type(MIN + LENGTH - 1 - IDS)>{args...}...);
					else
						return std::make_tuple(Entry<integral_type(MAX - LENGTH + 1 + IDS)>{args...}...);
				}

				/** Wrapper for gen_tuple.
                 * @return The constructed tuple.
                 */


				static auto make_tuple(Args &&...args) {
					return gen_tuple(std::make_integer_sequence<integral_type, LENGTH>(), std::forward<Args>(args)...);
				}

				/* CAUTION: has to be __after__ the make_tuple function.
				 * Also make_tuple isn't allowed to have overloads.
				 */
				using type = std::invoke_result_t<decltype(make_tuple), Args &&...>;
			};

			typename TupleGenerator::type count_tuple_;

		public:
			explicit IntegralTemplatedTuple(Args &&...args)
				: count_tuple_(TupleGenerator::make_tuple(std::forward<Args>(args)...)) {}

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
		template<template<auto> typename EntryTypeTemplate,
				 std::integral auto FIRST, std::integral auto LAST, typename... Args>
		auto make_integral_template_tuple(Args &&...args) {
			return IntegralTemplatedTuple<EntryTypeTemplate, FIRST, LAST, Args...>(std::forward<Args>(args)...);
		}
	}// namespace new_impl
}// namespace hypertrie::internal::util

#endif//HYPERTRIE_INTEGRALTEMPLATEDTUPLENEW_HPP