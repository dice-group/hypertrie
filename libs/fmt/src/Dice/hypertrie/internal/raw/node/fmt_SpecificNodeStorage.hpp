#ifndef HYPERTRIE_FMT_SPECIFICNODESTORAGE_HPP
#define HYPERTRIE_FMT_SPECIFICNODESTORAGE_HPP

#include <Dice/hypertrie/internal/raw/node/SpecificNodeStorage.hpp>
#include <Dice/hypertrie/internal/util/fmt_utils.hpp>

namespace fmt {
	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri_t, template<size_t, typename> typename node_type>
	struct formatter<::hypertrie::internal::raw::SpecificNodeStorage<depth, tri_t, node_type>> : hypertrie::internal::util::SimpleParsing {
		template<typename FormatContext>
		auto format(::hypertrie::internal::raw::SpecificNodeStorage<depth, tri_t, node_type> const &sns, FormatContext &ctx) {
			return ::hypertrie::internal::util::format_map_ptr_val(sns.nodes(), ctx.out());
		}
	};
}// namespace fmt

#endif//HYPERTRIE_FMT_SPECIFICNODESTORAGE_HPP
