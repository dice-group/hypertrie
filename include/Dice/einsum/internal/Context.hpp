#ifndef HYPERTRIE_CONTEXT_HPP
#define HYPERTRIE_CONTEXT_HPP

#include "Dice/einsum/internal/Subscript.hpp"

namespace einsum::internal {
	/**
	 * The context is passed to very operator. It helps to pass information into the operator graph and allows the
	 * operators to communicate during execution.
	 */
	struct Context {
		/**
		 * A fixed label order in which the labels should be processed.
		 * This is an exemplary way of passing in information.
		 */
		std::vector<Label> label_order;

		/**
		 * This creates a context that has label_order set to ascending sorted labels.
		 * @param subscript the subscript that definies the execution
		 * @return a Context with the labels ordered by name
		 */
		static Context orderByLabelName(std::shared_ptr<Subscript> subscript) {
			Context context{};
			const auto &label_set = subscript->getOperandsLabelSet();
			context.label_order.resize(label_set.size());
			std::copy(label_set.begin(), label_set.end(), context.label_order.begin());
			std::sort(context.label_order.begin(), context.label_order.end());
			return context;
		}
	};

}
#endif //HYPERTRIE_CONTEXT_HPP
