#ifndef HYPERTRIE_FMT_KEY_HPP
#define HYPERTRIE_FMT_KEY_HPP

#include <Dice/hypertrie/Key.hpp>
#include "Dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <::hypertrie::internal::HypertrieTrait tri_t>
	struct formatter<::hypertrie::Key<tri_t>> : hypertrie::internal::util::SimpleParsing {
		template<typename T>
		static auto nameOfType() {
			return ::hypertrie::internal::util::name_of_type<T>();
		}
		template <typename FormatContext>
		auto format(::hypertrie::Key<tri_t> const& key, FormatContext &ctx) {
			auto out = format_to(ctx.out(),
							 "<trait = {} ({})>: ",
							 nameOfType<tri_t>(), tri_t{}
			);
			return ::hypertrie::internal::util::format_vector(key, out);
		}
	};

	template <::hypertrie::internal::HypertrieTrait tri_t>
	struct formatter<::hypertrie::SliceKey<tri_t>> : hypertrie::internal::util::SimpleParsing {
		template<typename T>
		static auto nameOfType() {
			return ::hypertrie::internal::util::name_of_type<T>();
		}
		template <typename FormatContext>
		auto format(::hypertrie::SliceKey<tri_t> const& key, FormatContext &ctx) {
			auto out = format_to(ctx.out(),
								 "<trait = {} ({})>: ",
								 nameOfType<tri_t>(), tri_t{}
			);
			return ::hypertrie::internal::util::format_vector(key, out);
		}
	};
}

#endif//HYPERTRIE_FMT_KEY_HPP
