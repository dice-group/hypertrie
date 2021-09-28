#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <Dice/hypertrie/internal/raw/fmt_Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>

#include <string>
#include <fmt/core.h>
#include <Dice/hypertrie/internal/Hypertrie_default_traits.hpp>

namespace hypertrie::tests::fmt {
	TEST_SUITE("fmt tests") {
		using namespace ::hypertrie::internal::raw;

		TEST_CASE("hypertrie_trait") {
			std::string result = ::fmt::format("{}", default_bool_Hypertrie_trait{});
			REQUIRE(result == "<key_part = unsigned long, value = bool, "
							  "allocator = std::allocator, map = tsl::sparse_map, "
							  "set = tsl::sparse_set, key_part_tagging_bit = -1>");
		}

		TEST_CASE("hypertrie_core_trait") {
			std::string result = ::fmt::format("{}", Hypertrie_core_t<default_bool_Hypertrie_trait>{});
			REQUIRE(result == "<key_part = unsigned long, value = bool, "
							  "allocator = std::allocator, map = tsl::sparse_map, "
							  "set = tsl::sparse_set, key_part_tagging_bit = -1>");
		}
	}
}