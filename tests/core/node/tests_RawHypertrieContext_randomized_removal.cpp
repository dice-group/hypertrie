#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <random>

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

namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("randomized testing of RawHypertrieContext remove") {

		template<size_t depth, HypertrieTrait htt_t>
		void insert_remove_read() {
			using key_part_type = typename htt_t::key_part_type;
			using value_type = typename htt_t::value_type;

			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance

			utils::RawEntryGenerator<depth, htt_t> gen{};

			for (size_t const count : iter::chain(iter::range(1, 10), iter::range(10, 30, 5), iter::range(300, 301))) {
				SUBCASE("insert {} entries then remove half"_format(count).c_str()) {
					auto const runs = (count != 300) ? 500 : 5;
					for (auto const run : iter::range(runs)) {
						SUBCASE("{}"_format(run).c_str()) {
							// TODO: reconsider -- bad for high count and low depth
							gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(count, 1.0 / depth))));
							gen.setValueMinMax(value_type(1), value_type(2));
							CAPTURE(gen.getKeyPartMax());
							CAPTURE(gen.getValueMax());

							gen.wind(run);

							std::vector<SingleEntry<depth, htt_t>> const entries = gen.entries(count);
							std::vector<SingleEntry<depth, htt_t>> const to_remove{entries.begin(), entries.begin() + (entries.size() / 2)};
							std::vector<SingleEntry<depth, htt_t>> const diff{entries.begin() + (entries.size() / 2), entries.end()};

							CAPTURE(entries);
							CAPTURE(to_remove);
							CAPTURE(diff);

							ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> const validation_context_0{std::allocator<std::byte>{}, entries};
							INFO("Before remove:\n", validation_context_0);

							RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
							NodePtr<depth, htt_t, allocator_type> nc{};
							context.insert(nc, entries);
							REQUIRE(context == validation_context_0);

							ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> const validation_context{std::allocator<std::byte>(), diff};
							INFO("Expected after remove:\n", validation_context);

							context.remove(nc, to_remove);
							INFO("Actual after remove:\n", context);
							INFO("Actual result identifier:\n", nc.identifier());

							REQUIRE(validation_context == context);
							for (const auto &entry : diff) {
								REQUIRE(context.get(nc, entry.key()) == entry.value());
							}
						}
					}
				}

				SUBCASE("insert {} entries then remove all or all but one"_format(count).c_str()) {
					auto const runs = (count != 300) ? 500 : 5;
					std::random_device seed_rng;
					std::default_random_engine rng{seed_rng()};

					for (auto const run : iter::range(runs)) {
						SUBCASE("{}"_format(run).c_str()) {
							// TODO: reconsider -- bad for high count and low depth
							gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(count, 1.0 / depth))));
							gen.setValueMinMax(value_type(1), value_type(2));

							CAPTURE(gen.getKeyPartMax());
							CAPTURE(gen.getValueMax());

							gen.wind(run);

							std::vector<SingleEntry<depth, htt_t>> const entries = gen.entries(count);

							std::uniform_int_distribution<size_t> dist{entries.size() - 1, entries.size()};
							size_t const remove_count = dist(rng);

							std::vector<SingleEntry<depth, htt_t>> const to_remove{entries.begin(), entries.begin() + remove_count};
							std::vector<SingleEntry<depth, htt_t>> const diff{entries.begin() + remove_count, entries.end()};

							CAPTURE(entries);
							CAPTURE(to_remove);
							CAPTURE(diff);

							ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> const validation_context_0{std::allocator<std::byte>(), entries};
							INFO("Before remove:\n", validation_context_0);

							RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
							NodePtr<depth, htt_t, allocator_type> nc{};
							context.insert(nc, entries);
							REQUIRE(context == validation_context_0);

							ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> const validation_context{std::allocator<std::byte>(), diff};
							INFO("Expected after remove:\n", validation_context);

							context.remove(nc, to_remove);

							INFO("Actual after remove:\n", context);
							INFO("Actual result identifier after remove: ", nc.identifier());

							REQUIRE(validation_context == context);
							for (const auto &entry : diff) {
								REQUIRE(context.get(nc, entry.key()) == entry.value());
							}
						}
					}
				}

				SUBCASE("insert 2x{} entries then remove random amount"_format(count).c_str()) {
					auto const runs = (count != 300) ? 500 : 5;
					auto const total_count = 2 * count;

					std::random_device seed_rng;
					std::default_random_engine rng{seed_rng()};

					for (auto const run : iter::range(runs)) {
						SUBCASE("{}"_format(run).c_str()) {
							// TODO: reconsider -- bad for high count and low depth
							gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(total_count, 1.0 / depth))));
							gen.setValueMinMax(value_type(1), value_type(2));

							CAPTURE(gen.getKeyPartMax());
							CAPTURE(gen.getValueMax());

							gen.wind(run);

							std::vector<SingleEntry<depth, htt_t>> const entries = gen.entries(total_count);

							std::uniform_int_distribution<size_t> dist{0, entries.size()};
							size_t const remove_count = dist(rng);

							std::vector<SingleEntry<depth, htt_t>> const to_remove{entries.begin(), entries.begin() + remove_count};
							std::vector<SingleEntry<depth, htt_t>> const diff{entries.begin() + remove_count, entries.end()};

							CAPTURE(entries);
							CAPTURE(to_remove);
							CAPTURE(diff);

							ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> const validation_context_0{std::allocator<std::byte>(), entries};
							INFO("Before remove:\n", validation_context_0);

							RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
							NodePtr<depth, htt_t, allocator_type> nc{};
							context.insert(nc, entries);
							REQUIRE(context == validation_context_0);

							ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> const validation_context{std::allocator<std::byte>(), diff};
							INFO("Expected after remove:\n", validation_context);

							context.remove(nc, to_remove);
							INFO("Actual after remove:\n", context);
							INFO("Actual result identifier after remove: ", nc.identifier());

							REQUIRE(validation_context == context);
							for (const auto &entry : diff) {
								REQUIRE(context.get(nc, entry.key()) == entry.value());
							}
						}
					}
				}
			}
		}

		TEST_CASE_TEMPLATE("insert then remove then read", T,
						   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
						   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
						   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
						   tagged_long_cfg<1>, tagged_long_cfg<2>, tagged_long_cfg<3>, tagged_long_cfg<4>, tagged_long_cfg<5>,
						   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>,
						   tagged_double_cfg<1>, tagged_double_cfg<2>, tagged_double_cfg<3>, tagged_double_cfg<4>, tagged_double_cfg<5>
						   //
		) {
			insert_remove_read<T::depth, typename T::htt_t>();
		}
	}

} // namespace dice::hypertrie::tests::core::node
