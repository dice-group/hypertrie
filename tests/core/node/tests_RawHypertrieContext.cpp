#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <dice/hypertrie/internal/util/fmt_utils.hpp>
#include <utils/Node_test_configs.hpp>
#include <utils/RawEntryGenerator.hpp>
#include <utils/ValidationRawNodeContext.hpp>


#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>
#include <utils/DumpRawContext.hpp>

namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawNodeContext") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		TEST_CASE("problematic entries 1") {
			using T = bool_cfg<1>;
			constexpr auto depth = 1;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1}}, true},
												   SingleEntry_t{{{2}}, true},
												   SingleEntry_t{{{3}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<1, htt_t, allocator_type> context{alloc};
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<1, htt_t, std::allocator<std::byte>> validation_context_0{alloc, entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			fmt::print("B: {}", context);
			ValidationRawNodeContext<1, htt_t, std::allocator<std::byte>> validation_context{alloc, all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<1, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 12") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
			constexpr auto count = 3;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{3, 3, 3}}, true},
												   SingleEntry_t{{{4, 3, 3}}, true},
												   SingleEntry_t{{{4, 1, 5}}, true},
												   SingleEntry_t{{{5, 2, 1}}, true},
												   SingleEntry_t{{{4, 4, 4}}, true},
												   SingleEntry_t{{{4, 3, 5}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};

			std::cout << "insert 0:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);
			std::cout << "\nExpected insert 0:\n";
			dump_context(validation_context_0);
			dump_context_hash_translation_table(validation_context_0);
			std::cout.flush();
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());

			context.insert(nc, std::vector{entries_1});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_1{std::allocator<std::byte>(), all_entries};

			std::cout << "insert 1:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);
			std::cout << "\nExpected insert 1:\n";
			dump_context(validation_context_1);
			dump_context_hash_translation_table(validation_context_1);
			CHECK(validation_context_1 == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout.flush();

		};

		TEST_CASE("problematic entries 11") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
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
			RawHypertrieContext<5, htt_t, allocator_type> context{alloc};
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{alloc, entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{alloc, all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<5, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 10") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{{1, 2, 1, 1}}, true},
												   SingleEntry_t{{{1, 1, 1, 1}}, true},
												   SingleEntry_t{{{1, 2, 2, 1}}, true}};
			decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
			decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};
			std::cout << fmt::format("entries_0: {{ {} }}", fmt::join(entries_0, ", \n")) << std::endl;
			std::cout << fmt::format("entries_1: {{ {} }}", fmt::join(entries_1, ", \n")) << std::endl;
			std::cout << fmt::format("all_entries: {{ {} }}", fmt::join(all_entries, ", \n")) << std::endl;
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<5, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 9") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
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
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<5, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 8") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
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
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<5, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 7") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
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
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<5, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 6") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
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
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<5, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 5") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
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
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<5, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 4") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
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
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			;
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<5, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 3") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
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
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			;
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<5, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};

		TEST_CASE("problematic entries 2") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
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
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);

			std::cout << "insert 1:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);
			std::cout << "\nExpected insert 1:\n";
			dump_context(validation_context_0);
			dump_context_hash_translation_table(validation_context_0);
			std::cout.flush();

			context.insert(nc, std::vector{entries_1});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};

			std::cout << "insert 2:\n";
			dump_context(context);
			dump_context_hash_translation_table(context);
			std::cout << "\nExpected insert 2:\n";
			dump_context(validation_context_0);
			dump_context_hash_translation_table(validation_context_0);
			std::cout.flush();

			CHECK(validation_context == context);
		};


		TEST_CASE("problematic entries 1") {
			using T = bool_cfg<3>;
			constexpr auto depth = T::depth;
			using htt_t = typename T::htt_t;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using SingleEntry_t = SingleEntry<depth, htt_t>;
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
			RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodeContainer<depth, htt_t, allocator_type> nc{};
			context.insert(nc, std::vector{entries_0});
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
			CHECK(validation_context_0 == context);
			for (const auto &entry : entries_0)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 0: {}", nc.raw_identifier()) << std::endl;
			fmt::print("A: {}", context);

			context.insert(nc, std::vector{entries_1});
			;
			fmt::print("B: {}", context);
			ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
			fmt::print("V: {}", static_cast<RawHypertrieContext<5, htt_t, allocator_type> &>(validation_context));

			CHECK(validation_context == context);
			for (const auto &entry : all_entries)
				CHECK(context.get(nc, entry.key()) == entry.value());
			std::cout << fmt::format("result identifier 1: {}", nc.raw_identifier()) << std::endl;
		};
	};
};// namespace dice::hypertrie::tests::core::node