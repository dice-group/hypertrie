#ifndef HYPERTRIE_RAWKEY_HPP
#define HYPERTRIE_RAWKEY_HPP

#include <array>
#include <fmt/ostream.h>
#include <optional>

namespace hypertrie::internal {
	template<size_t depth, typename key_part_type>
	using RawKey = std::array<key_part_type, depth>;

	template<size_t depth, typename key_part_type>
	using RawSliceKey = std::array<std::optional<key_part_type>, depth>;


}// namespace hypertrie::internal


template<size_t depth, typename key_part_type>
struct fmt::formatter<hypertrie::internal::RawKey<depth, key_part_type>> {
private:
	using raw_key_t = typename hypertrie::internal::RawKey<depth, key_part_type>;

public:
	auto parse(format_parse_context &ctx) {
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const raw_key_t &key, FormatContext &ctx) {
		if constexpr (depth == 0)
			ctx.out() << "[ ]";
		else
			fmt::format_to(ctx.out(), "[{}]", fmt::join(key, " "));
		return ctx.out();
	}
};

template<size_t depth, typename key_part_type>
struct fmt::formatter<hypertrie::internal::RawSliceKey<depth, key_part_type>> {
private:
	using raw_slice_key_t = typename hypertrie::internal::RawKey<depth, key_part_type>;

public:
	auto parse(format_parse_context &ctx) {
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const raw_slice_key_t &key, FormatContext &ctx) {
		if constexpr (depth == 0)
			ctx.out() << "[ ]";
		else {
			ctx.out() << "[ ";
			for (size_t i = 0; i < depth; ++i) {
				if (key[0].has_value())
					ctx.out() << key[0].value();
				else
					ctx.out() << ":";
				if (i != depth - 1)
					ctx.out() << ", ";
			}
			ctx.out() << "] ";
		}
		return ctx.out();
	}
};

#endif//HYPERTRIE_RAWKEY_HPP
