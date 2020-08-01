#ifndef HYPERTRIE_HYPERTRIECONTEXT_HPP
#define HYPERTRIE_HYPERTRIECONTEXT_HPP


#include "Dice/hypertrie/internal/node_based/raw/storage/NodeContext.hpp"
#include "Dice/hypertrie/internal/util/SwitchTemplateFunctions.hpp"
#include <fmt/format.h>


namespace hypertrie::internal::node_based {

	static constexpr const std::size_t max_depth = 5;

	template<typename tr = default_bool_Hypertrie_t>
	class HypertrieContext {
		const size_t depth_;
		void *raw_context;

		HypertrieContext(size_t depth) : depth_(depth) {
			compiled_switch<max_depth, 1>::switch_(
					depth,
					[&](auto depth_arg) -> void * { return new raw::NodeContext<depth_arg, raw::Hypertrie_internal_t<tr>>{}; },
					[&]() -> void * { throw std::logic_error{fmt::format("Hypertrie depth {} invalid. allowed range: [1,{})", depth, max_depth)}; });
		}
	};
}

#endif //HYPERTRIE_HYPERTRIECONTEXT_HPP
