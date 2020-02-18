#ifndef HYPERTRIE_CARDINALITYESTIMATION_HPP
#define HYPERTRIE_CARDINALITYESTIMATION_HPP

#include <cmath>
#include "Dice/einsum/internal/Subscript.hpp"
#include "Dice/einsum/internal/Entry.hpp"

namespace einsum::internal {


	template<typename key_part_type, template<typename, typename> class map_type,
			template<typename> class set_type>
	struct CardinalityEstimation {
		using const_BoolHypertrie_t = const_BoolHypertrie<key_part_type, map_type, set_type>;

		/**
		 *
		 * @param operands
		 * @param label_candidates
		 * @param sc
		 * @return
		 */
		static Label getMinCardLabel(const std::vector<const_BoolHypertrie_t> &operands,
									 const std::shared_ptr<Subscript> &sc,
									 const std::shared_ptr<Context> &context) {
			const tsl::hopscotch_set<Label> &operandsLabelSet = sc->getOperandsLabelSet();
			const tsl::hopscotch_set<Label> &lonely_non_result_labels = sc->getLonelyNonResultLabelSet();
			for (const auto &label : context->label_order) {
				if (operandsLabelSet.count(label) and not lonely_non_result_labels.count(label))
					return label;
			}
			throw std::logic_error{"There were no labels left to process via a join, but a join operator was issued."};
		}
	};
}
#endif //HYPERTRIE_CARDINALITYESTIMATION_HPP
