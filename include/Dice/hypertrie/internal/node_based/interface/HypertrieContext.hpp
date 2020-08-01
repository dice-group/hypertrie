#ifndef HYPERTRIE_HYPERTRIECONTEXT_HPP
#define HYPERTRIE_HYPERTRIECONTEXT_HPP


#include "Dice/hypertrie/internal/node_based/raw/storage/NodeContext.hpp"

static constexpr const std::size_t depth = 5;

namespace hypertrie::internal::node_based {

	template<typename tr = default_bool_Hypertrie_t>
	class HypertrieContext {
		const size_t depth_;

		HypertrieContext(size_t depth) : depth_(depth) {

		}

	};
}

#endif //HYPERTRIE_HYPERTRIECONTEXT_HPP
