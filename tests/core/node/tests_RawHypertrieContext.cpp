#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

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

	TEST_SUITE("Testing of RawNodeContext") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		TEST_CASE("problematic entries 11") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 1, 2, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 2}}, true},
												   SingleEntry_t{{{1, 2, 1, 1}}, true},
												   SingleEntry_t{{{2, 1, 1, 1}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{alloc, entries_0};
			dump_context(validation_context_0, "expected");

			RawHypertrieContext<5, htt_t, allocator_type> context{alloc};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			dump_context(context, "actual");

			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;

			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{alloc, all_entries};
			dump_context(validation_context, "expected 2");

			context.insert(nc, entries_1);
			dump_context(context, "actual 2");

			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
		};

		TEST_CASE("problematic entries 10") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 2, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 1}}, true},
												   SingleEntry_t{{{1, 2, 2, 1}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			dump_context(validation_context_0, "expected 1");

			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			dump_context(context, "actual 1");

			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;

			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			dump_context(validation_context, "expected 2");

			context.insert(nc, entries_1);
			dump_context(context, "actual 2");

			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
		};

		TEST_CASE("problematic entries 9") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 1, 2, 1}}, true},
												   SingleEntry_t{{{2, 1, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 2}}, true},
												   SingleEntry_t{{{1, 2, 1, 1}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			 ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;
			dump_context(context, "A");

			context.insert(nc, entries_1);
			dump_context(context, "B");
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			dump_context(validation_context, "V");


			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 8") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{2, 1, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 1}}, true},
												   SingleEntry_t{{{2, 1, 1, 2}}, true},
												   SingleEntry_t{{{2, 1, 2, 1}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			 ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;
			dump_context(context, "A");

			context.insert(nc, entries_1);
			dump_context(context, "B");
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			dump_context(validation_context, "V");

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 7") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;
			constexpr auto count = 3;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{2, 1, 2}}, true},
												   SingleEntry_t{{{2, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 2}}, true},
												   SingleEntry_t{{{2, 2, 1}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			dump_context(validation_context_0, "EXPECTED 1");

			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			dump_context(context, "ACTUAL 1");

			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;

			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			dump_context(validation_context, "EXPECTED 2");

			context.insert(nc, entries_1);
			dump_context(context, "ACTUAL 2");

			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
		};

		TEST_CASE("problematic entries 6") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{4, 3, 1}}, true},
												   SingleEntry_t{{{2, 1, 1}}, true},
												   SingleEntry_t{{{4, 1, 3}}, true},
												   SingleEntry_t{{{1, 3, 1}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;


			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			dump_context(validation_context_0, "EXPECTED 1");

			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			dump_context(context, "ACTUAL 1");

			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;

			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			dump_context(validation_context, "EXPECTED 2");

			context.insert(nc, entries_1);
			dump_context(context, "ACTUAL 2");

			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
		};

		TEST_CASE("problematic entries 5") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{2, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1}}, true},
												   SingleEntry_t{{{1, 2, 1}}, true},
												   SingleEntry_t{{{3, 1, 1}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			 ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;
			dump_context(context, "A");

			context.insert(nc, entries_1);
			dump_context(context, "B");
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			dump_context(validation_context, "V");

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 4") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;
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


			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			dump_context(validation_context_0, "EXPECTED 1");

			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			dump_context(context, "ACTUAL 1");

			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;

			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			dump_context(validation_context, "EXPECTED 2");
			dump_context_hash_translation_table(validation_context);

			context.insert(nc, entries_1);
			dump_context(context, "ACTUAL 2");

			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
		};

		TEST_CASE("problematic entries 3") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;
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


			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			dump_context(validation_context_0, "EXPECTED 1");

			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			dump_context(context, "ACTUAL 1");

			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;

			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			dump_context(validation_context, "EXPECTED 2");

			context.insert(nc, entries_1);
			dump_context(context, "ACTUAL 2");

			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
		};

		TEST_CASE("problematic entries 2") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;
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
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			 ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;
			dump_context(context, "A");

			context.insert(nc, entries_1);
			dump_context(context, "B");

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			dump_context(validation_context, "V");

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;
		};


		TEST_CASE("problematic entries 1") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;
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

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			dump_context(validation_context_0, "EXPECTED 1");

			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};
			context.insert(nc, entries_0);
			dump_context(context, "ACTUAL 1");

			std::cout << fmt::format("result identifier 0: {}", nc.identifier()) << std::endl;

			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());

			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			dump_context(validation_context, "EXPECTED 2");

			context.insert(nc, entries_1);
			dump_context(context, "ACTUAL 2");

			std::cout << fmt::format("result identifier 1: {}", nc.identifier()) << std::endl;

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
		};

		TEST_CASE("simple cartesian 1") {
			using T = bool_cfg<2>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 3}}, true},
												   SingleEntry_t{{{1, 4}}, true},
												   SingleEntry_t{{{2, 3}}, true},
												   SingleEntry_t{{{2, 4}}, true}};

			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			dump_context(context);
		}

		TEST_CASE("simple prefix 1") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 2, 5}}, true},
												   SingleEntry_t{{{1, 2, 6}}, true},
												   SingleEntry_t{{{1, 4, 5}}, true},
												   SingleEntry_t{{{1, 4, 6}}, true},
												   SingleEntry_t{{{1, 5, 1}}, true}};

			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, all_entries);
			dump_context(context);
		}

		TEST_CASE("insert into fn to complete xn") {
			using T = bool_cfg<2>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance
			using SingleEntry_t = SingleEntry<depth,  htt_t>;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 3}}, true},
												   SingleEntry_t{{{1, 4}}, true},
												   SingleEntry_t{{{2, 3}}, true},
												   SingleEntry_t{{{2, 4}}, true}};

			auto const init_count = all_entries.size() - 1;
			std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
			std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};

			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodePtr<depth, htt_t, allocator_type> nc{};

			context.insert(nc, init_insert);
			dump_context(context);

			context.insert(nc, rest_insert);
			dump_context(context);
		}
	};

	TEST_CASE("problematic cartesian 1") {
		using T = bool_cfg<2>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 1}}, true},
											   SingleEntry_t{{{1, 3}}, true},
											   SingleEntry_t{{{1, 2}}, true},
											   SingleEntry_t{{{1, 4}}, true}};

		auto const init_count = 2;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 12") {
		using T = tagged_bool_cfg<1>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{13}}, true},
											   SingleEntry_t{{{3}}, true},
											   SingleEntry_t{{{2}}, true},
											   SingleEntry_t{{{7}}, true}};

		auto const init_count = 2;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 13") {
		using T = bool_cfg<3>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{3, 4, 3}}, true},
											   SingleEntry_t{{{4, 3, 3}}, true},
											   SingleEntry_t{{{3, 3, 1}}, true},
											   SingleEntry_t{{{5, 3, 3}}, true},
											   SingleEntry_t{{{4, 4, 4}}, true},
											   SingleEntry_t{{{3, 5, 3}}, true},
											   SingleEntry_t{{{2, 2, 4}}, true},
											   SingleEntry_t{{{5, 2, 5}}, true},
											   SingleEntry_t{{{4, 5, 5}}, true},
											   SingleEntry_t{{{5, 3, 6}}, true}};

		auto const init_count = 5;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 14") {
		using T = bool_cfg<2>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 1}}, true},
											   SingleEntry_t{{{1, 3}}, true},
											   SingleEntry_t{{{1, 2}}, true},
											   SingleEntry_t{{{1, 4}}, true}};

		auto const init_count = 2;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 15") {
		using T = long_cfg<3>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{3, 3, 2}}, 1},
											   SingleEntry_t{{{2, 2, 2}}, 2}};

		auto const init_count = 1;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("Problematic entries 16") {
		using T = long_cfg<5>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{ { 1, 2, 5, 5, 1 }, 1 },
											   SingleEntry_t{ { 4, 6, 3, 2, 6 }, 2 },
											   SingleEntry_t{ { 5, 4, 4, 4, 4 }, 2 },
											   SingleEntry_t{ { 3, 4, 5, 3, 4 }, 2 },
											   SingleEntry_t{ { 1, 2, 4, 3, 3 }, 2 },
											   SingleEntry_t{ { 2, 2, 4, 2, 5 }, 1 },
											   SingleEntry_t{ { 5, 3, 4, 5, 3 }, 1 },
											   SingleEntry_t{ { 1, 3, 6, 5, 3 }, 1 },
											   SingleEntry_t{ { 4, 3, 5, 1, 5 }, 1 },
											   SingleEntry_t{ { 6, 6, 4, 2, 1 }, 1 },
											   SingleEntry_t{ { 5, 5, 1, 5, 6 }, 2 },
											   SingleEntry_t{ { 6, 4, 2, 3, 4 }, 1 },
											   SingleEntry_t{ { 2, 5, 3, 4, 3 }, 1 },
											   SingleEntry_t{ { 2, 3, 4, 5, 4 }, 1 },
											   SingleEntry_t{ { 2, 3, 6, 5, 6 }, 1 },
											   SingleEntry_t{ { 6, 4, 4, 2, 5 }, 1 },
											   SingleEntry_t{ { 6, 2, 4, 3, 3 }, 2 },
											   SingleEntry_t{ { 1, 4, 2, 6, 2 }, 1 },
											   SingleEntry_t{ { 6, 4, 5, 5, 3 }, 1 },
											   SingleEntry_t{ { 1, 4, 5, 5, 6 }, 2 },
											   SingleEntry_t{ { 3, 4, 3, 5, 4 }, 2 },
											   SingleEntry_t{ { 2, 4, 3, 1, 4 }, 2 },
											   SingleEntry_t{ { 4, 1, 1, 1, 1 }, 1 },
											   SingleEntry_t{ { 3, 6, 4, 1, 4 }, 1 },
											   SingleEntry_t{ { 3, 3, 3, 2, 6 }, 2 },
											   SingleEntry_t{ { 6, 6, 1, 4, 5 }, 1 },
											   SingleEntry_t{ { 1, 2, 3, 5, 4 }, 1 },
											   SingleEntry_t{ { 6, 3, 4, 1, 3 }, 2 },
											   SingleEntry_t{ { 4, 5, 5, 6, 5 }, 1 },
											   SingleEntry_t{ { 6, 1, 4, 6, 4 }, 1 },
											   SingleEntry_t{ { 4, 3, 4, 4, 5 }, 1 },
											   SingleEntry_t{ { 4, 2, 2, 6, 5 }, 2 },
											   SingleEntry_t{ { 6, 5, 1, 3, 4 }, 2 },
											   SingleEntry_t{ { 3, 4, 5, 6, 6 }, 2 },
											   SingleEntry_t{ { 3, 4, 6, 3, 6 }, 2 },
											   SingleEntry_t{ { 2, 6, 5, 4, 4 }, 1 },
											   SingleEntry_t{ { 3, 6, 6, 6, 1 }, 2 },
											   SingleEntry_t{ { 2, 4, 6, 3, 6 }, 1 },
											   SingleEntry_t{ { 6, 2, 1, 2, 2 }, 1 },
											   SingleEntry_t{ { 3, 5, 4, 4, 5 }, 2 }};

		auto const init_count = 20;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("Problematic entries 17") {
		using T = long_cfg<5>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{ { 2, 2, 2, 1, 1 }, 1 },
											   SingleEntry_t{ { 2, 3, 1, 1, 1 }, 2 }};

		auto const init_count = 1;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("Problematic entries 18") {
		using T = long_cfg<3>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{{2, 3, 2}, 2},
											   SingleEntry_t{{2, 1, 2}, 1},
											   SingleEntry_t{{3, 3, 2}, 2},
											   SingleEntry_t{{2, 2, 2}, 2}};

		auto const init_count = 2;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("Problematic entries 19") {
		using T = long_cfg<5>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{{ { 1, 2, 5, 5, 1 }, 1 },
											   { { 4, 6, 3, 2, 6 }, 2 },
											   { { 5, 4, 4, 4, 4 }, 2 },
											   { { 3, 4, 5, 3, 4 }, 2 },
											   { { 1, 2, 4, 3, 3 }, 2 },
											   { { 2, 2, 4, 2, 5 }, 1 },
											   { { 5, 3, 4, 5, 3 }, 1 },
											   { { 1, 3, 6, 5, 3 }, 1 },
											   { { 4, 3, 5, 1, 5 }, 1 },
											   { { 6, 6, 4, 2, 1 }, 1 },
											   { { 5, 5, 1, 5, 6 }, 2 },
											   { { 6, 4, 2, 3, 4 }, 1 },
											   { { 2, 5, 3, 4, 3 }, 1 },
											   { { 2, 3, 4, 5, 4 }, 1 },
											   { { 2, 3, 6, 5, 6 }, 1 },
											   { { 6, 4, 4, 2, 5 }, 1 },
											   { { 6, 2, 4, 3, 3 }, 2 },
											   { { 1, 4, 2, 6, 2 }, 1 },
											   { { 6, 4, 5, 5, 3 }, 1 },
											   { { 1, 4, 5, 5, 6 }, 2 },
											   { { 3, 4, 3, 5, 4 }, 2 },
											   { { 2, 4, 3, 1, 4 }, 2 },
											   { { 4, 1, 1, 1, 1 }, 1 },
											   { { 3, 6, 4, 1, 4 }, 1 },
											   { { 3, 3, 3, 2, 6 }, 2 },
											   { { 6, 6, 1, 4, 5 }, 1 },
											   { { 1, 2, 3, 5, 4 }, 1 },
											   { { 6, 3, 4, 1, 3 }, 2 },
											   { { 4, 5, 5, 6, 5 }, 1 },
											   { { 6, 1, 4, 6, 4 }, 1 },
											   { { 4, 3, 4, 4, 5 }, 1 },
											   { { 4, 2, 2, 6, 5 }, 2 },
											   { { 6, 5, 1, 3, 4 }, 2 },
											   { { 3, 4, 5, 6, 6 }, 2 },
											   { { 3, 4, 6, 3, 6 }, 2 },
											   { { 2, 6, 5, 4, 4 }, 1 },
											   { { 3, 6, 6, 6, 1 }, 2 },
											   { { 2, 4, 6, 3, 6 }, 1 },
											   { { 6, 2, 1, 2, 2 }, 1 },
											   { { 3, 5, 4, 4, 5 }, 2 }};

		auto const init_count = 20;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 20") {
		using T = tagged_long_cfg<3>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{{ { 3, 2, 3 }, 1 },
											   { { 3, 1, 2 }, 1 }};

		auto const init_count = 1;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 21") {
		using T = tagged_long_cfg<2>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{{ { 4, 4 }, 2 },
											   { { 5, 4 }, 1 },
											   { { 1, 1 }, 2 },
											   { { 1, 3 }, 1 }};

		auto const init_count = 2;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 22") {
		using T = bool_cfg<3>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{ { 4, 2, 3 } },
											   SingleEntry_t{ { 1, 3, 5 } },
											   SingleEntry_t{ { 3, 3, 3 } },
											   SingleEntry_t{ { 2, 2, 2 } },
											   SingleEntry_t{ { 4, 4, 4 } },
											   SingleEntry_t{ { 2, 4, 2 } },
											   SingleEntry_t{ { 3, 4, 2 } },
											   SingleEntry_t{ { 2, 4, 3 } }};

		auto const init_count = 4;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 23") {
		using T = bool_cfg<2>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{ { 1, 2 }, true },
											   SingleEntry_t{ { 1, 4 }, true },
											   SingleEntry_t{ { 5, 3 }, true },
											   SingleEntry_t{ { 4, 3 }, true }};

		auto const init_count = 2;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 24") {
		using T = bool_cfg<2>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{ { 4, 9 }, true },
											   SingleEntry_t{ { 12, 2 }, true },
											   SingleEntry_t{ { 6, 6 }, true },
											   SingleEntry_t{ { 2, 4 }, true },
											   SingleEntry_t{ { 3, 3 }, true },
											   SingleEntry_t{ { 3, 6 }, true },
											   SingleEntry_t{ { 2, 3 }, true },
											   SingleEntry_t{ { 11, 5 }, true },
											   SingleEntry_t{ { 8, 6 }, true },
											   SingleEntry_t{ { 3, 11 }, true },
											   SingleEntry_t{ { 11, 12 }, true },
											   SingleEntry_t{ { 13, 4 }, true }};

		auto const init_count = 6;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 25") {
		using T = bool_cfg<3>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{ { 2, 2, 1 }, true },
											   SingleEntry_t{ { 2, 4, 3 }, true },
											   SingleEntry_t{ { 2, 2, 2 }, true },
											   SingleEntry_t{ { 2, 2, 3 }, true }};

		auto const init_count = 2;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 26") {
		using T = bool_cfg<2>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth, htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{ { 5, 1 }, true },
											   SingleEntry_t{ { 4, 2 }, true },
											   SingleEntry_t{ { 5, 2 }, true },
											   SingleEntry_t{ { 4, 1 }, true }};

		auto const init_count = 2;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 27") {
		using T = tagged_bool_cfg<2>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth, htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{{3, 1}, true },
											   SingleEntry_t{{2, 1}, true }};

		auto const init_count = 1;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("problematic entries 28") {
		using T = bool_cfg<2>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{}; // allocator instance
		using SingleEntry_t = SingleEntry<depth, htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{{1, 4}, true},
											   SingleEntry_t{{1, 7}, true},
											   SingleEntry_t{{9, 5}, true},
											   SingleEntry_t{{7, 4}, true},
											   SingleEntry_t{{1, 5}, true},
											   SingleEntry_t{{7, 7}, true},
											   SingleEntry_t{{9, 4}, true},
											   SingleEntry_t{{7, 5}, true}};

		auto const init_count = 4;
		std::vector<SingleEntry_t> init_insert{all_entries.begin(), all_entries.begin() + init_count};
		std::vector<SingleEntry_t> rest_insert{all_entries.begin() + init_count, all_entries.end()};


		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context_0{allocator_type{}, init_insert};
		dump_context(validation_context_0, "EXPECTED 1");
		dump_context_hash_translation_table(validation_context_0);

		RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, init_insert);
		dump_context(context, "ACTUAL 1");

		CHECK(validation_context_0 == context);
		for (const auto &entry : init_insert)
			CHECK(context.get(nc, entry.key()) == entry.value());

		ValidationRawNodeContext<5, htt_t, allocator_type> validation_context{allocator_type{}, all_entries};
		dump_context(validation_context, "EXPECTED 2");
		dump_context_hash_translation_table(validation_context);

		context.insert(nc, rest_insert);
		dump_context(context, "ACTUAL 2");

		CHECK(validation_context == context);
		for (const auto &entry : all_entries)
			CHECK(context.get(nc, entry.key()) == entry.value());
	}

	TEST_CASE("index proxy") {
		using T = bool_cfg<3>;
		constexpr auto depth = T::depth;
		using htt_t = typename T::htt_t;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{};
		using SingleEntry_t = SingleEntry<depth,  htt_t>;

		std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{3, 2, 2}}, true},
											   SingleEntry_t{{{2, 2, 2}}, true},
											   SingleEntry_t{{{3, 4, 4}}, true},
											   SingleEntry_t{{{2, 3, 2}}, true}};

		RawHypertrieContext<5, htt_t, allocator_type> context{alloc};
		NodePtr<depth, htt_t, allocator_type> nc{};
		context.insert(nc, all_entries);

		auto const get_proxy = [&](RawKey<depth, htt_t> const &key) {
			return RawIndexProxy<depth, 5, htt_t, allocator_type>{&context, &nc, key};
		};

		{
			auto proxy = get_proxy({{3, 2, 2}});
			CHECK(proxy == true);
			proxy = false;
			CHECK(proxy == false);
			proxy = true;
		}

		{
			auto proxy1 = get_proxy({{3, 2, 2}});
			auto proxy2 = get_proxy({{3, 2, 2}});
			auto proxy3 = get_proxy({{2, 2, 2}});

			CHECK(proxy1 == true);
			CHECK(proxy2 == true);
			CHECK(proxy3 == true);

			proxy1 = false;
			CHECK(proxy1 == false);
			CHECK(proxy2 == false);
			CHECK(proxy3 == true);
		}
	}

};// namespace dice::hypertrie::tests::core::node