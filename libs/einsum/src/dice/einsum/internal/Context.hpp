#ifndef HYPERTRIE_CONTEXT_HPP
#define HYPERTRIE_CONTEXT_HPP

#include "dice/einsum/Commons.hpp"
#include "dice/einsum/Subscript.hpp"
#include "dice/einsum/TimeoutException.hpp"

#include <chrono>

namespace dice::einsum::internal {

	/**
	 * The context is passed to very operator. It helps to pass information into the operator graph and allows the
	 * operators to communicate during execution.
	 * It is also responsible for managing timeouts.
	 */
	class Context {
		static constexpr uint max_counter_ = 512;

	public:
		using clock = std::chrono::steady_clock;
		using time_point = clock::time_point;
		using duration = clock::duration;

	private:
		time_point start_time_;
		time_point end_time_;
		duration time_out_duration_;
		bool has_time_out_;


		/**
		 * The time is only checked when counter hits max_counter.
		 */
		uint counter_ = 0;

	public:
		explicit Context(time_point const &end_time = time_point::max()) noexcept
			: start_time_(clock::now()),
			  end_time_(end_time),
			  time_out_duration_(end_time_ - start_time_),
			  has_time_out_(end_time_ != time_point::max()) {}

		/**
		 * Checks if the timeout is already reached. If the timeout is reached it throws a TimeoutException.
		 */
		void check_time_out() {
			if (has_time_out_ and counter_++ > max_counter_) {
				if (clock::now() < end_time_) [[likely]] {
					counter_ = 0;
				} else {
					throw TimeoutException(time_out_duration_);
				}
			}
		}
	};

}// namespace dice::einsum::internal
#endif//HYPERTRIE_CONTEXT_HPP
