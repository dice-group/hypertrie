#ifndef HYPERTRIE_CONTEXT_HPP
#define HYPERTRIE_CONTEXT_HPP

#include "Dice/einsum/internal/Subscript.hpp"

namespace einsum::internal {

	using TimePoint = std::chrono::steady_clock::time_point;

	/**
	 * The context is passed to very operator. It helps to pass information into the operator graph and allows the
	 * operators to communicate during execution.
	 * It is also responsible for managing timeouts.
	 */
	class Context {
		constexpr static const uint max_counter = 500;
	public:
		/**
		 * The time after that the processing shall be stopped.
		 */
		TimePoint timeout;

		/**
		 * The time is only checked when counter hits max_counter.
		 */
		uint counter = 0;

		/**
		 * Indicates if the timeout was already reached.
		 */
		bool timed_out = false;

		Context(TimePoint const &timeout = TimePoint::max()) : timeout(timeout) {}

		/**
		 * Checks if the timeout is already reached. This method is intentionally unsynchronized.
		 * @return if the timeout was reached. If true, the timeout was reached for sure.
		 * If false, the timeout may already be reached.
		 * It will be reported monotonically starting approximately at one of the next max_counter calls of this method.
		 */
		inline bool hasTimedOut() {
			if (this->timed_out)
				return true;
			else if (this->counter > max_counter) {
				this->timed_out = this->timeout <= std::chrono::steady_clock::now();
				if (not this->timed_out) {
					this->counter = 0;
				}
				return this->counter;
			}

			++this->counter;
			return false;
		}
	};

}
#endif //HYPERTRIE_CONTEXT_HPP
