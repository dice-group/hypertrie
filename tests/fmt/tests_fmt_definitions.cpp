#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <dice/hypertrie/fmt_Key.hpp>
#include <dice/hypertrie/internal/container/fmt_AllContainer.hpp>
#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/fmt_RawDiagonalPositions.hpp>
#include <dice/hypertrie/internal/raw/fmt_RawKey.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_AllocateNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

#include <dice/hypertrie/Hypertrie_default_traits.hpp>
#include <fmt/core.h>
#include <string>

namespace dice::hypertrie::tests::fmt {
	TEST_SUITE("fmt tests") {
		using namespace ::dice::hypertrie::internal::raw;

		TEST_CASE("hypertrie_trait") {
			std::string result = ::fmt::format("{}", default_bool_Hypertrie_trait{});
			REQUIRE(result == "<key_part = unsigned long, value = bool, "
							  "map = dice::sparse_map::sparse_map, "
							  "set = dice::sparse_map::sparse_set, key_part_tagging_bit = -1>");
		}

		TEST_CASE("RawKeyPositions") {
			RawKeyPositions<5> pos{std::vector<size_t>{0, 2, 4}};
			std::string result = ::fmt::format("{}", pos);
			REQUIRE(result == "[0, 2, 4]");
		}

		TEST_CASE("RawKey") {
			RawKey<5, default_bool_Hypertrie_trait> pos{{1, 2, 3, 4, 5}};
			std::string result = ::fmt::format("{}", pos);
			REQUIRE(result == "[1, 2, 3, 4, 5]");
		}

		TEST_CASE("RawSliceKey") {
			RawSliceKey<4, default_bool_Hypertrie_trait> pos{{1, 2, std::nullopt, 4, 5}};
			std::string result = ::fmt::format("{}", pos);
			REQUIRE(result == "[0 -> 1, 1 -> 2, 3 -> 4, 4 -> 5]");
		}

		template<size_t N, typename, typename>
		struct MyMapType {};
		TEST_CASE("AllocateNode") {
			AllocateNode<5, default_bool_Hypertrie_trait, MyMapType, std::allocator<std::byte>> alloc_node(std::allocator<std::byte>{});
			std::string result = ::fmt::format("{}", alloc_node);
			REQUIRE(result == "<depth = 5, trait = dice::hypertrie::Hypertrie_t (<key_part = unsigned long, value = bool, map = dice::sparse_map::sparse_map, set = dice::sparse_map::sparse_set, key_part_tagging_bit = -1>)>");
		}

		TEST_CASE("SingleEntryNode") {
			SingleEntryNode<5, default_bool_Hypertrie_trait, std::allocator<std::byte>> sen(SingleEntry<5, default_bool_Hypertrie_trait>{{0, 1, 2, 3, 4}}, 3);
			std::string result = ::fmt::format("{}", sen);
			std::cout << result << std::endl;
			REQUIRE(result == "{ [ref_count=3] <0, 1, 2, 3, 4> -> true }");
		}

		TEST_CASE("FullNode") {
			FullNode<3, default_bool_Hypertrie_trait, std::allocator<std::byte>> fn(10, {});
			std::string result = ::fmt::format("{}", fn);

			REQUIRE(result == "{ [size=0,ref_count=10]\n"
							  "0: [\n"
							  "]\n"
							  "1: [\n"
							  "]\n"
							  "2: [\n"
							  "]\n"
							  " }");
		}

		TEST_CASE("SpecificNodeStorage") {
			SpecificNodeStorage<2, default_bool_Hypertrie_trait, FullNode, std::allocator<std::byte>> fn_storage(std::allocator<std::byte>{});
			std::string result = ::fmt::format("{}", fn_storage);
			REQUIRE(result == "{}");
			SpecificNodeStorage<2, default_bool_Hypertrie_trait, FullNode, std::allocator<std::byte>> sn_storage(std::allocator<std::byte>{});
			result = ::fmt::format("{}", sn_storage);
			REQUIRE(result == "{}");
		}

		TEST_CASE("RawHypertrieContext") {
			RawHypertrieContext<2, default_bool_Hypertrie_trait, std::allocator<std::byte>> alloc_node(std::allocator<std::byte>{});
			std::string result = ::fmt::format("{}", alloc_node);
			std::cout << result << std::endl;
			REQUIRE(result == "<RawHypertrieContext\n"
							  "[2] FN\n"
							  "{}\n"
							  "[2] SEN\n"
							  "{}\n"
							  "[1] FN\n"
							  "{}\n"
							  "[1] SEN\n"
							  "{}\n"
							  ">");
		}

		/** TODO: std_set is only an alias for a special std::set. So std::set is also defined!
		 * Also it wasn't able to deduce the Allocator as a second template parameter (std_set rebinds).
		 */
		TEST_CASE("std_set") {
			::dice::hypertrie::internal::container::std_set<std::string> set = {"Hello", "World"};
			std::string result = ::fmt::format("{}", set);
			REQUIRE(result == "{Hello, World}");
		}

		/** TODO: std_map is only an alias for a special std::map. So std::map is also defined!
		 * Also it wasn't able to deduce the Allocator as a second template parameter (std_map rebinds).
		 */
		TEST_CASE("std_map") {
			::dice::hypertrie::internal::container::std_map<int, std::string> map = {{0, "Hello"}, {1, "World"}};
			std::string result = ::fmt::format("{}", map);
			REQUIRE(result == "{(0, Hello), (1, World)}");
		}

		/** TODO: dice_sparse_set is only an alias for a special dice::sparse_map::sparse_set. So dice::sparse_map::sparse_set is also defined!
		 * Also it wasn't able to deduce the Allocator as a second template parameter (dice::sparse_map::sparse_set rebinds).
		 */
		TEST_CASE("dice_sparse_set") {
			::dice::hypertrie::internal::container::dice_sparse_set<std::string> set = {"Hello", "World"};
			std::string result = ::fmt::format("{}", set);
			REQUIRE(result == "{World, Hello}");
		}

		/** TODO: dice_sparse_map is only an alias for a special dice::sparse_map::sparse_map. So dice::sparse_map::sparse_map is also defined!
		 * Also it wasn't able to deduce the Allocator as a second template parameter (dice_sparse_map rebinds).
		 */
		TEST_CASE("dice_sparse_map") {
			::dice::hypertrie::internal::container::dice_sparse_map<int, std::string> map = {{0, "Hello"}, {1, "World"}};
			std::string result = ::fmt::format("{}", map);
			REQUIRE(result == "{(0, Hello), (1, World)}");
		}

		TEST_CASE("Key") {
			Key<default_bool_Hypertrie_trait> key{{0, 1, 2}};
			std::string result = ::fmt::format("{}", key);
			REQUIRE(result == "<trait = dice::hypertrie::Hypertrie_t "
							  "(<key_part = unsigned long, value = bool, "
							  "map = dice::sparse_map::sparse_map, set = dice::sparse_map::sparse_set, key_part_tagging_bit = -1>)>: "
							  "{0, 1, 2}");
		}

		TEST_CASE("SliceKey") {
			SliceKey<default_bool_Hypertrie_trait> key{{0, std::nullopt, 2}};
			std::string result = ::fmt::format("{}", key);
			REQUIRE(result == "<trait = dice::hypertrie::Hypertrie_t "
							  "(<key_part = unsigned long, value = bool, "
							  "map = dice::sparse_map::sparse_map, set = dice::sparse_map::sparse_set, key_part_tagging_bit = -1>)>: "
							  "{0, -, 2}");
		}

		// TODO: add tests for the rest of the types
	}
}// namespace dice::hypertrie::tests::fmt