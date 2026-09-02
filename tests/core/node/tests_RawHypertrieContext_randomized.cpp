#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <utils/ValidationRawNodeContext.hpp>
#include <dice/hypertrie/internal/util/fmt_utils.hpp>
#include <utils/Node_test_configs.hpp>
#include <utils/RawEntryGenerator.hpp>
#include <utils/DumpRawContext.hpp>

#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("randomized testing of RawNodeContext") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		template<size_t depth, HypertrieTrait htt_t>
		void write_and_read() {
			CAPTURE(depth);
			CAPTURE(htt_t{});

			using key_part_type = typename htt_t::key_part_type;
			using value_type = typename htt_t::value_type;

			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance

			utils::RawEntryGenerator<depth, htt_t> gen{};

			for (size_t const count : iter::chain(iter::range(1, 10), iter::range(10, 30, 5), iter::range(300, 301))) {
				SUBCASE("insert {} entries "_format(count).c_str()) {
					auto const runs = (count != 300) ? 500 : 5;
					for (auto const run : iter::range(runs)) {
						SUBCASE("{}"_format(run).c_str()) {
							// TODO: reconsider -- bad for high count and low depth
							gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(count, 1.0 / depth))));
							gen.setValueMinMax(value_type(1), value_type(2));

							CAPTURE(gen.getKeyPartMax());
							CAPTURE(gen.getValueMax());

							gen.wind(run);

							auto const entries = gen.entries(count);
							CAPTURE(entries);

							ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> const validation_context{std::allocator<std::byte>(), entries};
							INFO("Expected after insert:\n", validation_context);

							RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
							NodePtr<depth, htt_t, allocator_type> nc{};
							context.insert(nc, entries);

							INFO("Actual after insert:\n", context);
							INFO("Actual result identifier after insert: ", nc.identifier());

							REQUIRE(validation_context == context);
							for (const auto &entry : entries) {
								REQUIRE(context.get(nc, entry.key()) == entry.value());
							}
						}
					}
				}

				SUBCASE("insert 2x{} entries "_format(count).c_str()) {
					auto const runs = (count != 300) ? 500 : 5;
					for (auto const run : iter::range(runs)) {
						SUBCASE("{}"_format(run).c_str()) {
							auto total_count = 2 * count;
							gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(total_count, 2.0 / depth))));
							gen.setValueMinMax(value_type(1), value_type(2));

							CAPTURE(gen.getKeyPartMax());
							CAPTURE(gen.getValueMax());

							gen.wind(run);

							auto const all_entries = gen.entries(total_count);
							decltype(all_entries) entries_0 = {all_entries.begin(), all_entries.begin() + count};
							decltype(all_entries) entries_1 = {all_entries.begin() + count, all_entries.end()};

							CAPTURE(entries_0);
							CAPTURE(entries_1);
							CAPTURE(all_entries);

							ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> const validation_context_0{std::allocator<std::byte>(), entries_0};
							INFO("Expected after first insert:\n", validation_context_0);

							RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
							NodePtr<depth, htt_t, allocator_type> nc{};
							context.insert(nc, entries_0);
							INFO("Actual after first insert:\n", context);
							INFO("Actual result identifier after first insert: ", nc.identifier());

							REQUIRE(validation_context_0 == context);
							for (const auto &entry : entries_0) {
								REQUIRE(context.get(nc, entry.key()) == entry.value());
							}

							ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> const validation_context{std::allocator<std::byte>(), all_entries};
							INFO("Expected after second insert:\n", validation_context);

							context.insert(nc, entries_1);
							INFO("Actual after second insert:\n", context);
							INFO("Actual result identifier after second insert: ", nc.identifier());

							REQUIRE(validation_context == context);
							for (const auto &entry : all_entries) {
								REQUIRE(context.get(nc, entry.key()) == entry.value());
							}
						}
					}
				}
			}
		}

		TEST_CASE_TEMPLATE("write and read", T,
						   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
						   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
						   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
						   tagged_long_cfg<1>, tagged_long_cfg<2>, tagged_long_cfg<3>, tagged_long_cfg<4>, tagged_long_cfg<5>,
						   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>,
						   tagged_double_cfg<1>, tagged_double_cfg<2>, tagged_double_cfg<3>, tagged_double_cfg<4>, tagged_double_cfg<5>
		) {
			write_and_read<T::depth, typename T::htt_t>();
		}
	}
};// namespace dice::hypertrie::tests::core::node
