#ifndef HYPERTRIE_FMT_KEY_HPP
#define HYPERTRIE_FMT_KEY_HPP

#include <dice/hypertrie/Key.hpp>
#include "dice/hypertrie/internal/util/fmt_utils.hpp"

namespace fmt {
	template <::dice::hypertrie::HypertrieTrait tri_t>
	struct formatter<::dice::hypertrie::Key<tri_t>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename T>
		static auto nameOfType() {
			return ::dice::hypertrie::internal::util::name_of_type<T>();
		}
		template <typename FormatContext>
		auto format(::dice::hypertrie::Key<tri_t> const& key, FormatContext &ctx) {
			auto out = format_to(ctx.out(),
							 "<trait = {} ({})>: ",
							 nameOfType<tri_t>(), tri_t{}
			);
			return ::dice::hypertrie::internal::util::format_vector(key, out);
		}
	};

	template <::dice::hypertrie::HypertrieTrait tri_t>
	struct formatter<::dice::hypertrie::SliceKey<tri_t>> : ::dice::hypertrie::internal::util::SimpleParsing {
		template<typename T>
		static auto nameOfType() {
			return ::dice::hypertrie::internal::util::name_of_type<T>();
		}
		template <typename FormatContext>
		auto format(::dice::hypertrie::SliceKey<tri_t> const& key, FormatContext &ctx) {
			auto out = format_to(ctx.out(),
								 "<trait = {} ({})>: ",
								 nameOfType<tri_t>(), tri_t{}
			);
			return ::dice::hypertrie::internal::util::format_vector(key, out);
		}
	};
}

#endif//HYPERTRIE_FMT_KEY_HPP
