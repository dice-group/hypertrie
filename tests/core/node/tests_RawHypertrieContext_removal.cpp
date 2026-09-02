#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <dice/hypertrie/internal/util/fmt_utils.hpp>
#include <utils/Node_test_configs.hpp>
#include <utils/RawEntryGenerator.hpp>
#include <utils/ValidationRawNodeContext.hpp>
#include <utils/DumpRawContext.hpp>


#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>


namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("raw hypertrie context removal") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		TEST_CASE("simple check removed entries") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> const all_entries{SingleEntry_t{{{1, 1, 2}}, true},
												   SingleEntry_t{{{1, 1, 1}}, true},
												   SingleEntry_t{{{1, 2, 1}}, true},
												   SingleEntry_t{{{2, 1, 1}}, true}};

			std::vector<SingleEntry_t> const to_remove{ all_entries.begin(), all_entries.begin() + count };
			std::vector<SingleEntry_t> const diff{ all_entries.begin() + count, all_entries.end() };


			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{alloc};
			NodePtr<depth, htt_t, allocator_type> nc{};

			ValidationRawNodeContext<depth, htt_t, allocator_type> valid_context{alloc, diff};

			context.insert(nc, all_entries);

			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			context.remove(nc, to_remove);

			std::cout << "\nAfter remove:\n";
			dump_context(context);

			dump_context_hash_translation_table(context);

			std::cout << "\nExpected after remove:\n";
			dump_context(valid_context);

			std::cout << fmt::format("result identifier: {}", nc.identifier()) << std::endl;

			CHECK(context == valid_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		};

		TEST_CASE("simple check no single entry full nodes") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
			constexpr auto count = 4;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{{{1, 8, 3}}, true},
					SingleEntry_t{{{2, 4, 3}}, true},
					SingleEntry_t{{{9, 4, 2}}, true},
					SingleEntry_t{{{8, 5, 3}}, true},
					SingleEntry_t{{{4, 2, 7}}, true},
					SingleEntry_t{{{1, 7, 1}}, true},
					SingleEntry_t{{{4, 2, 5}}, true},
					SingleEntry_t{{{5, 2, 1}}, true},
					SingleEntry_t{{{3, 9, 1}}, true},
					SingleEntry_t{{{8, 3, 4}}, true},
					SingleEntry_t{{{1, 1, 5}}, true},
					SingleEntry_t{{{7, 5, 8}}, true},
					SingleEntry_t{{{4, 3, 7}}, true},
					SingleEntry_t{{{7, 9, 1}}, true}};

			std::vector<SingleEntry_t> const to_remove{ all_entries.end() - count, all_entries.end() };
			std::vector<SingleEntry_t> const diff{ all_entries.begin(), all_entries.end() - count };

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "Before remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			context.remove(nc, to_remove);
			std::cout << "After remove:\n";
			dump_context(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "Expected after remove:\n";
			dump_context(validation_context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		};

		TEST_CASE("edge case remove all but one") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{{{1, 8, 3}}, true},
					SingleEntry_t{{{2, 4, 3}}, true},
					SingleEntry_t{{{9, 4, 2}}, true},
					SingleEntry_t{{{8, 5, 3}}, true},
					SingleEntry_t{{{4, 2, 7}}, true},
					SingleEntry_t{{{1, 7, 1}}, true},
					SingleEntry_t{{{4, 2, 5}}, true},
					SingleEntry_t{{{5, 2, 1}}, true},
					SingleEntry_t{{{3, 9, 1}}, true},
					SingleEntry_t{{{8, 3, 4}}, true},
					SingleEntry_t{{{1, 1, 5}}, true},
					SingleEntry_t{{{7, 5, 8}}, true},
					SingleEntry_t{{{4, 3, 7}}, true},
					SingleEntry_t{{{7, 9, 1}}, true}};

			std::vector<SingleEntry_t> const to_remove{ all_entries.begin(), all_entries.end() - 1 };
			std::vector<SingleEntry_t> const diff{ all_entries.end() - 1, all_entries.end() };

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "Before remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			context.remove(nc, to_remove);
			std::cout << "After remove:\n";
			dump_context(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "Expected after remove:\n";
			dump_context(validation_context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		};

		TEST_CASE("edge case remove all") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{{{1, 8, 3}}, true},
					SingleEntry_t{{{2, 4, 3}}, true},
					SingleEntry_t{{{9, 4, 2}}, true},
					SingleEntry_t{{{8, 5, 3}}, true},
					SingleEntry_t{{{4, 2, 7}}, true},
					SingleEntry_t{{{1, 7, 1}}, true},
					SingleEntry_t{{{4, 2, 5}}, true},
					SingleEntry_t{{{5, 2, 1}}, true},
					SingleEntry_t{{{3, 9, 1}}, true},
					SingleEntry_t{{{8, 3, 4}}, true},
					SingleEntry_t{{{1, 1, 5}}, true},
					SingleEntry_t{{{7, 5, 8}}, true},
					SingleEntry_t{{{4, 3, 7}}, true},
					SingleEntry_t{{{7, 9, 1}}, true}};

			std::vector<SingleEntry_t> const to_remove{ all_entries.begin(), all_entries.end() };
			std::vector<SingleEntry_t> const diff{};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "Before remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			context.remove(nc, to_remove);
			std::cout << "After remove:\n";
			dump_context(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "Expected after remove:\n";
			dump_context(validation_context);

			CHECK(context == validation_context);
		};

		TEST_CASE("problematic entries move") {
			using T = bool_cfg<2>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 3, 4 }, true },
					SingleEntry_t{ { 4, 1 }, true },
					SingleEntry_t{ { 3, 2 }, true },
					SingleEntry_t{ { 4, 2 }, true },
					SingleEntry_t{ { 3, 1 }, true },
					SingleEntry_t{ { 2, 4 }, true },
					SingleEntry_t{ { 4, 4 }, true },
					SingleEntry_t{ { 1, 3 }, true }};

			std::vector<SingleEntry_t> const to_remove{ all_entries.begin(), all_entries.begin() + 3 };
			std::vector<SingleEntry_t> const diff{ all_entries.begin() + 3, all_entries.end() };

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
		}

		TEST_CASE("problematic entries after entry subset optim") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 2, 2, 2 }, true },
					SingleEntry_t{ { 2, 3, 2 }, true },
					SingleEntry_t{ { 1, 3, 1 }, true },
					SingleEntry_t{ { 3, 2, 3 }, true },
					SingleEntry_t{ { 2, 3, 3 }, true },
					SingleEntry_t{ { 1, 1, 2 }, true }};

			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + 3};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + 3, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries after sen check optim") {
			using T = tagged_bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					 SingleEntry_t{ { 1, 1, 3 }, true },
					 SingleEntry_t{ { 3, 3, 2 }, true },
					 SingleEntry_t{ { 3, 2, 3 }, true },
					 SingleEntry_t{ { 1, 1, 1 }, true } };

			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + 2};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + 2, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 1") {
			using T = bool_cfg<2>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 2, 2 }, true },
					SingleEntry_t{ { 2, 1 }, true },
					SingleEntry_t{ { 1, 2 }, true },
					SingleEntry_t{ { 1, 1 }, true } };

			auto const count = 3;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 2") {
			using T = bool_cfg<2>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 1, 2 }, true },
					SingleEntry_t{ { 1, 1 }, true },
					SingleEntry_t{ { 1, 3 }, true } };

			auto const count = 1;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 3") {
			using T = bool_cfg<2>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 2, 3 }, true },
					SingleEntry_t{ { 3, 1 }, true },
					SingleEntry_t{ { 2, 1 }, true },
					SingleEntry_t{ { 3, 3 }, true },
					SingleEntry_t{ { 1, 3 }, true },
					SingleEntry_t{ { 1, 1 }, true } };

			auto const count = 5;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 4") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 3, 2, 1 }, true },
					SingleEntry_t{ { 2, 2, 3 }, true },
					SingleEntry_t{ { 1, 3, 2 }, true },
					SingleEntry_t{ { 1, 2, 1 }, true } };

			auto const count = 2;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 5") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 1, 2, 1 }, true },
					SingleEntry_t{ { 3, 1, 3 }, true },
					SingleEntry_t{ { 1, 2, 2 }, true },
					SingleEntry_t{ { 2, 3, 2 }, true } };

			auto const count = 2;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 6") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 1, 2, 2 }, true },
					SingleEntry_t{ { 2, 3, 2 }, true },
					SingleEntry_t{ { 3, 2, 2 }, true },
					SingleEntry_t{ { 3, 3, 2 }, true } };

			auto const count = 2;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context_hash_translation_table(validation_context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 7") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 1, 2, 3 }, true },
					SingleEntry_t{ { 1, 3, 2 }, true },
					SingleEntry_t{ { 2, 3, 3 }, true },
					SingleEntry_t{ { 3, 2, 3 }, true } };

			auto const count = 1;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 8") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 2, 3, 2 }, true },
					SingleEntry_t{ { 2, 2, 1 }, true },
					SingleEntry_t{ { 2, 1, 3 }, true },
					SingleEntry_t{ { 3, 3, 3 }, true },
					SingleEntry_t{ { 3, 2, 2 }, true },
					SingleEntry_t{ { 2, 2, 2 }, true } };

			auto const count = 3;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 9") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 1, 2, 1, 3 }, true },
					SingleEntry_t{ { 2, 2, 1, 2 }, true },
					SingleEntry_t{ { 2, 2, 1, 1 }, true },
					SingleEntry_t{ { 3, 1, 3, 1 }, true },
					SingleEntry_t{ { 1, 2, 1, 2 }, true },
					SingleEntry_t{ { 2, 1, 1, 2 }, true } };

			auto const count = 2;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 10") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 1, 2, 1 }, true },
					SingleEntry_t{ { 2, 2, 3 }, true },
					SingleEntry_t{ { 1, 1, 1 }, true },
					SingleEntry_t{ { 3, 1, 2 }, true },
					SingleEntry_t{ { 1, 1, 3 }, true },
					SingleEntry_t{ { 3, 2, 1 }, true },
					SingleEntry_t{ { 1, 2, 3 }, true },
					SingleEntry_t{ { 2, 1, 3 }, true } };

			auto const count = 6;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 11") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 3, 3, 3, 1 }, true },
					SingleEntry_t{ { 2, 2, 2, 1 }, true },
					SingleEntry_t{ { 2, 3, 1, 1 }, true } };

			auto const count = 1;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 12") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 1, 3, 1, 1 }, true },
					SingleEntry_t{ { 3, 3, 2, 1 }, true },
					SingleEntry_t{ { 1, 2, 3, 1 }, true },
					SingleEntry_t{ { 2, 1, 3, 1 }, true } };

			auto const count = 2;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 13") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 2, 2, 3, 2 }, true },
					SingleEntry_t{ { 2, 2, 2, 2 }, true },
					SingleEntry_t{ { 3, 2, 3, 1 }, true },
					SingleEntry_t{ { 1, 1, 2, 3 }, true },
					SingleEntry_t{ { 2, 3, 2, 1 }, true },
					SingleEntry_t{ { 2, 3, 1, 2 }, true } };

			auto const count = 1;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 14") {
			using T = bool_cfg<5>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 3, 3, 1, 3, 1 }, true },
					SingleEntry_t{ { 2, 2, 3, 1, 1 }, true },
					SingleEntry_t{ { 2, 3, 1, 3, 1 }, true },
					SingleEntry_t{ { 1, 3, 3, 2, 2 }, true },
					SingleEntry_t{ { 3, 3, 2, 1, 2 }, true },
					SingleEntry_t{ { 1, 2, 2, 3, 3 }, true },
					SingleEntry_t{ { 3, 2, 2, 2, 2 }, true },
					SingleEntry_t{ { 3, 3, 1, 3, 2 }, true },
					SingleEntry_t{ { 3, 1, 1, 1, 1 }, true },
					SingleEntry_t{ { 3, 3, 1, 1, 1 }, true },
					SingleEntry_t{ { 2, 2, 3, 3, 3 }, true },
					SingleEntry_t{ { 2, 2, 2, 2, 2 }, true },
					SingleEntry_t{ { 2, 2, 3, 2, 2 }, true },
					SingleEntry_t{ { 3, 1, 3, 2, 3 }, true },
					SingleEntry_t{ { 1, 3, 3, 1, 1 }, true },
					SingleEntry_t{ { 1, 1, 3, 1, 1 }, true },
					SingleEntry_t{ { 1, 1, 2, 1, 1 }, true },
					SingleEntry_t{ { 2, 3, 2, 3, 3 }, true },
					SingleEntry_t{ { 3, 3, 2, 2, 1 }, true },
					SingleEntry_t{ { 3, 1, 3, 1, 1 }, true } };

			auto const count = 6;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 15") {
			using T = long_cfg<1>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 2 }, 2 },
					SingleEntry_t{ { 3 }, 1 }};

			auto const count = 1;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 16") {
			using T = bool_cfg<5>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 3, 2, 2, 2, 3 }, true },
					SingleEntry_t{ { 2, 1, 2, 1, 2 }, true },
					SingleEntry_t{ { 3, 2, 1, 1, 2 }, true },
					SingleEntry_t{ { 2, 3, 1, 2, 1 }, true },
					SingleEntry_t{ { 2, 1, 2, 2, 1 }, true },
					SingleEntry_t{ { 1, 2, 1, 1, 1 }, true },
					SingleEntry_t{ { 2, 1, 1, 2, 2 }, true },
					SingleEntry_t{ { 2, 2, 2, 1, 3 }, true },
					SingleEntry_t{ { 3, 3, 3, 2, 2 }, true },
					SingleEntry_t{ { 1, 1, 1, 1, 2 }, true },
					SingleEntry_t{ { 2, 1, 3, 1, 2 }, true },
					SingleEntry_t{ { 1, 1, 3, 1, 3 }, true },
					SingleEntry_t{ { 1, 1, 3, 3, 2 }, true },
					SingleEntry_t{ { 2, 1, 1, 1, 1 }, true },
					SingleEntry_t{ { 1, 3, 3, 2, 2 }, true },
					SingleEntry_t{ { 3, 1, 1, 2, 2 }, true },
					SingleEntry_t{ { 3, 1, 3, 1, 2 }, true },
					SingleEntry_t{ { 2, 2, 2, 2, 3 }, true },
					SingleEntry_t{ { 2, 2, 1, 2, 3 }, true },
					SingleEntry_t{ { 3, 2, 1, 2, 3 }, true }};

			auto const count = 10;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 17") {
			using T = tagged_bool_cfg<1>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{{3}, true},
					SingleEntry_t{{1}, true}};

			auto const count = 1;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 18") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 1, 2, 1 }, true },
					SingleEntry_t{ { 2, 1, 3 }, true },
					SingleEntry_t{ { 3, 1, 1 }, true },
					SingleEntry_t{ { 1, 2, 3 }, true }};

			auto const count = 2;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 19") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{{1, 2, 2}, true},
					SingleEntry_t{{3, 2, 2}, true},
					SingleEntry_t{{1, 2, 1}, true},
					SingleEntry_t{{1, 3, 2}, true},
					SingleEntry_t{{2, 1, 2}, true},
					SingleEntry_t{{2, 2, 1}, true},
					SingleEntry_t{{1, 3, 1}, true},
					SingleEntry_t{{2, 1, 1}, true}};

			auto const count = 2;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}

		TEST_CASE("problematic entries 20") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> const all_entries{
					SingleEntry_t{ { 2, 2, 3 }, true },
					SingleEntry_t{ { 2, 2, 1 }, true },
					SingleEntry_t{ { 2, 3, 3 }, true },
					SingleEntry_t{ { 1, 2, 3 }, true },
					SingleEntry_t{ { 1, 2, 1 }, true }};

			auto const count = 2;
			std::vector<SingleEntry_t> const to_remove{all_entries.begin(), all_entries.begin() + count};
			std::vector<SingleEntry_t> const diff{all_entries.begin() + count, all_entries.end()};

			std::cout << fmt::format("entries to insert: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			std::cout << fmt::format("entries to remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
			std::cout << fmt::format("difference: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			std::cout << "\nBefore remove:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);

			ValidationRawNodeContext<depth, htt_t, allocator_type> validation_context{allocator_type{}, diff};
			std::cout << "\nExpected after remove:\n";
			dump_context(validation_context);
			dump_context_hash_translation_table(validation_context);

			std::cout << "\nAfter remove:\n";
			context.remove(nc, to_remove);
			dump_context(context);

			CHECK(context == validation_context);
			for (const auto &entry : diff)
				REQUIRE(context.get(nc, entry.key()) == entry.value());
		}
	}
}
