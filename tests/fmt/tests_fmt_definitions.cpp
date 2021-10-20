#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <Dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <Dice/hypertrie/internal/raw/fmt_Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/fmt_RawDiagonalPositions.hpp>
#include <Dice/hypertrie/internal/raw/fmt_RawKey.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_AllocateNode.hpp>
#include <Dice/hypertrie/internal/container/fmt_AllContainer.hpp>
#include <Dice/hypertrie/fmt_Key.hpp>

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

		TEST_CASE("RawKeyPositions") {
			RawKeyPositions<5> pos{std::vector<size_t>{0,2,4}};
			std::string result = ::fmt::format("{}", pos);
			REQUIRE(result == "[1, 0, 1, 0, 1]");
		}

		TEST_CASE("RawKey") {
			RawKey<5, Hypertrie_core_t<default_bool_Hypertrie_trait>> pos{{1, 2, 3, 4, 5}};
			std::string result = ::fmt::format("{}", pos);
			REQUIRE(result == "[1, 2, 3, 4, 5]");
		}

		TEST_CASE("RawSliceKey") {
			RawSliceKey<5, Hypertrie_core_t<default_bool_Hypertrie_trait>> pos{{1,2,std::nullopt,4,5}};
			std::string result = ::fmt::format("{}", pos);
			REQUIRE(result == "[1, 2, -, 4, 5]");
		}

		template <size_t N, typename> struct MyMapType {};
		TEST_CASE("AllocateNode") {
			AllocateNode<5, Hypertrie_core_t<default_bool_Hypertrie_trait>, MyMapType> alloc_node(std::allocator<std::byte>{});
			std::string result = ::fmt::format("{}", alloc_node);
			REQUIRE(result == "<depth = 5, trait = hypertrie::internal::raw::Hypertrie_core_t "
							  "(<key_part = unsigned long, value = bool, "
							  "allocator = std::allocator, map = tsl::sparse_map, set = tsl::sparse_set, "
							  "key_part_tagging_bit = -1>)>");
		}

		/** TODO: std_set is only an alias for a special std::set. So std::set is also defined!
		 * Also it wasn't able to deduce the Allocator as a second template parameter (std_set rebinds).
		 */
		TEST_CASE("std_set") {
			::hypertrie::internal::container::std_set<std::string> set = {"Hello", "World"};
			std::string result = ::fmt::format("{}", set);
			REQUIRE(result == "{Hello, World}");
		}

		/** TODO: std_map is only an alias for a special std::map. So std::map is also defined!
		 * Also it wasn't able to deduce the Allocator as a second template parameter (std_map rebinds).
		 */
		TEST_CASE("std_map") {
			::hypertrie::internal::container::std_map<int, std::string> map = {{0, "Hello"}, {1, "World"}};
			std::string result = ::fmt::format("{}", map);
			REQUIRE(result == "{(0, Hello), (1, World)}");
		}

		/** TODO: tsl_sparse_set is only an alias for a special tsl::sparse_set. So tsl::sparse_set is also defined!
		 * Also it wasn't able to deduce the Allocator as a second template parameter (tsl::sparse_set rebinds).
		 */
		TEST_CASE("tsl_sparse_set") {
			::hypertrie::internal::container::tsl_sparse_set<std::string> set = {"Hello", "World"};
			std::string result = ::fmt::format("{}", set);
			REQUIRE(result == "{World, Hello}");
		}

		/** TODO: tsl_sparse_map is only an alias for a special tsl::sparse_map. So tsl::sparse_map is also defined!
		 * Also it wasn't able to deduce the Allocator as a second template parameter (tsl_sparse_map rebinds).
		 */
		TEST_CASE("tsl_sparse_map") {
			::hypertrie::internal::container::tsl_sparse_map<int, std::string> map = {{0, "Hello"}, {1, "World"}};
			std::string result = ::fmt::format("{}", map);
			REQUIRE(result == "{(0, Hello), (1, World)}");
		}

		TEST_CASE("Key") {
			Key<default_bool_Hypertrie_trait> key {{0,1,2}};
			std::string result = ::fmt::format("{}", key);
			REQUIRE(result == "<trait = hypertrie::internal::Hypertrie_t "
							  "(<key_part = unsigned long, value = bool, allocator = std::allocator, "
							  "map = tsl::sparse_map, set = tsl::sparse_set, key_part_tagging_bit = -1>)>: "
							  "{0, 1, 2}");
		}

		TEST_CASE("SliceKey") {
			SliceKey<default_bool_Hypertrie_trait> key {{0,std::nullopt,2}};
			std::string result = ::fmt::format("{}", key);
			REQUIRE(result == "<trait = hypertrie::internal::Hypertrie_t "
							  "(<key_part = unsigned long, value = bool, allocator = std::allocator, "
							  "map = tsl::sparse_map, set = tsl::sparse_set, key_part_tagging_bit = -1>)>: "
							  "{0, -, 2}");
		}

	}
}