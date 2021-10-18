#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include "ValidationRawNodeContext.hpp"
#include <Dice/hypertrie/internal/util/fmt_utils.hpp>
#include <Node_test_configs.hpp>
#include <RawEntryGenerator.hpp>


#include <Dice/hypertrie/internal/raw/fmt_Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <Dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

namespace hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawNodeContext") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		TEST_CASE("problematic entries 11") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 1, 2, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 2}}, true},
												   SingleEntry_t{{{1, 2, 1, 1}}, true},
												   SingleEntry_t{{{2, 1, 1, 1}}, true}
			};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 10") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 2, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 1}}, true},
												   SingleEntry_t{{{1, 2, 2, 1}}, true}
			};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 9") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 1, 2, 1}}, true},
												   SingleEntry_t{{{2, 1, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 2}}, true},
												   SingleEntry_t{{{1, 2, 1, 1}}, true}
			};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 8") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{2, 1, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 1}}, true},
												   SingleEntry_t{{{2, 1, 1, 2}}, true},
												   SingleEntry_t{{{2, 1, 2, 1}}, true}
			};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 7") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 3;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{2, 1, 2}}, true},
												   SingleEntry_t{{{2, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 2}}, true},
												   SingleEntry_t{{{2, 2, 1}}, true}
			};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 6") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{4, 3, 1}}, true},
												   SingleEntry_t{{{2, 1, 1}}, true},
												   SingleEntry_t{{{4, 1, 3}}, true},
												   SingleEntry_t{{{1, 3, 1}}, true}
			};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 5") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{2, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1}}, true},
												   SingleEntry_t{{{1, 2, 1}}, true},
												   SingleEntry_t{{{3, 1, 1}}, true}
												   };
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 4") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{2, 3, 3, 1}}, true},
												   SingleEntry_t{{{1, 3, 3, 2}}, true},
												   SingleEntry_t{{{2, 3, 1, 1}}, true},
												   SingleEntry_t{{{3, 1, 3, 1}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 3") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{2, 3, 2}}, true},
												   SingleEntry_t{{{2, 1, 2}}, true},
												   SingleEntry_t{{{3, 3, 2}}, true},
												   SingleEntry_t{{{2, 2, 2}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 2") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{3, 3, 4}}, true},
												   SingleEntry_t{{{3, 3, 2}}, true},
												   SingleEntry_t{{{2, 1, 2}}, true},
												   SingleEntry_t{{{6, 6, 1}}, true},
												   SingleEntry_t{{{2, 2, 3}}, true},
												   SingleEntry_t{{{3, 4, 6}}, true},
												   SingleEntry_t{{{6, 2, 2}}, true},
												   SingleEntry_t{{{3, 3, 5}}, true},
												   SingleEntry_t{{{4, 5, 4}}, true},
												   SingleEntry_t{{{5, 5, 1}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};


		TEST_CASE("problematic entries 1") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{3, 2, 2}}, true},
												   SingleEntry_t{{{2, 2, 2}}, true},
												   SingleEntry_t{{{3, 4, 4}}, true},
												   SingleEntry_t{{{2, 3, 2}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			context.insert(nc, entries_0);
			ValidationRawNodeContext<5, tri> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, entries_1);
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, tri> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", (RawHypertrieContext<5, tri> &) validation_context);

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};
	};
};// namespace hypertrie::tests::core::node