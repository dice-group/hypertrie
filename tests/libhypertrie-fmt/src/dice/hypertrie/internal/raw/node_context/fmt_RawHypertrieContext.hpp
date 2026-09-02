#ifndef HYPERTRIE_FMT_RAWHYPERTRIECONTEXT_HPP
#define HYPERTRIE_FMT_RAWHYPERTRIECONTEXT_HPP

#include <dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SpecificNodeStorage.hpp>

#include <dice/hypertrie/internal/container/fmt_SparseMap.hpp>
#include <dice/hypertrie/internal/container/fmt_SparseSet.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <utils/DumpRawContext.hpp>

#include <dice/template-library/for.hpp>

namespace dice::hypertrie::internal::raw {
	template<size_t max_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	std::ostream &operator<<(std::ostream &os, RawHypertrieContext<max_depth, htt_t, allocator_type> const &context) {
		tests::core::node::dump_context(context, "", os);
		tests::core::node::dump_context_hash_translation_table(context, os);
		return os;
	}
} // namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_FMT_RAWHYPERTRIECONTEXT_HPP
