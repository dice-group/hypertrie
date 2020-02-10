#ifndef HYPERTRIE_CONTEXT_HPP
#define HYPERTRIE_CONTEXT_HPP

#include "Dice/einsum/internal/Subscript.hpp"

namespace einsum::internal {
	/**
	 * The context is passed to very operator. It helps to pass information into the operator graph and allows the
	 * operators to communicate during execution.
	 */

	using TimePoint = std::chrono::steady_clock::time_point;

	inline bool hasTimedOut(TimePoint timeout) {
		return timeout <= std::chrono::steady_clock::now();
	}

	class Context {
	public:

		/**
		 * The time after that the processing shall be stopped.
		 */
		TimePoint timeout{};

		/**
		 * A fixed label order in which the labels should be processed.
		 * This is an exemplary way of passing in information.
		 */
		std::vector<Label> label_order{};

		Context(std::shared_ptr<Subscript> subscript, TimePoint const &timeout) :
				timeout(timeout),
				label_order(sortedLabels(subscript)) {}

		/**
		 * Reads all labels from a Subscripts and returns a Vector with the Labels sorted lexicographically.
		 * @param subscript the subscript that defines the execution
		 * @return vector of Labels
		 */
		[[nodiscard]] static std::vector<Label> sortedLabels(std::shared_ptr<Subscript> const &subscript) {
			const auto &label_set = subscript->getOperandsLabelSet();
			std::vector<Label> label_order_{};
			label_order_.resize(label_set.size());
			std::copy(label_set.begin(), label_set.end(), label_order_.begin());
			std::sort(label_order_.begin(), label_order_.end());
			return label_order_;
		}


	};

}
#endif //HYPERTRIE_CONTEXT_HPP
