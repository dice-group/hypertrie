#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cmath>
#include <cstddef>
#include <memory>
#include <random>
#include <vector>

#include <fmt/format.h>

#include <utils/DumpRawContext.hpp>
#include <utils/Node_test_configs.hpp>
#include <utils/RawEntryGenerator.hpp>
#include <utils/ValidationRawNodeContext.hpp>

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/NodeContainer.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>

#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/util/fmt_utils.hpp>

namespace dice::hypertrie::tests::core::node {
	bool debug = false;

	template<size_t depth, HypertrieTrait htt_t>
	void insert_remove_read() {
		SUBCASE("{}"_format(htt_t{}).c_str()) {

			SUBCASE("hypertrie depth = {}"_format(depth).c_str()) {

				using key_part_type = typename htt_t::key_part_type;
				using value_type = typename htt_t::value_type;

				using allocator_type = std::allocator<std::byte>;
				allocator_type const alloc{};// allocator instance

				utils::RawEntryGenerator<depth, htt_t> gen{};

				for (size_t const count : iter::chain(iter::range(1, 10), iter::range(10, 30, 5), iter::range(300, 301))) {
					SUBCASE("insert {} entries then remove half"_format(count).c_str()) {
						auto runs = (count != 300) ? 500 : 5;
						for (const auto i : iter::range(runs)) {
							SUBCASE("{}"_format(i).c_str()) {
								// TODO: reconsider -- bad for high count and low depth
								gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(count, 1.0 / depth))));
								gen.setValueMinMax(value_type(1), value_type(2));

								// std::cout << "key_part_max " << gen.getKeyPartMax() << "\n"
								// 		     << "value_max " << gen.getValueMax() << std::endl;
								gen.wind(i);

								std::vector<SingleEntry<depth, htt_t>> const entries = gen.entries(count);
								std::vector<SingleEntry<depth, htt_t>> const to_remove{entries.begin(), entries.begin() + (entries.size() / 2)};
								std::vector<SingleEntry<depth, htt_t>> const diff{entries.begin() + (entries.size() / 2), entries.end()};

								std::cout << fmt::format("entries: {{ {} }}", fmt::join(entries, ", \n")) << std::endl;
								std::cout << fmt::format("to_remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
								std::cout << fmt::format("diff: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

								ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), diff};

								RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
								NodeContainer<depth, htt_t, allocator_type> nc{};
								context.insert(nc, std::vector{entries});

								if (debug) {
									std::cout << "Before remove:\n";
									dump_context(context);
									dump_context_hash_translation_table(context);
									std::cout << "\nExpected after remove:\n";
									dump_context(validation_context);
									dump_context_hash_translation_table(validation_context);
								}

								context.remove(nc, std::vector{to_remove});

								if (debug) {
									std::cout << "\nActual after remove:\n";
									dump_context(context);
									dump_context_hash_translation_table(context);

									std::cout << fmt::format("result identifier: {}", nc.raw_identifier()) << std::endl;
								}

								REQUIRE(validation_context == context);
								for (const auto &entry : diff) {
									REQUIRE(context.get(nc, entry.key()) == entry.value());
								}
							}
						}
					}

					SUBCASE("insert {} entries then remove all or all but one"_format(count).c_str()) {
						auto runs = (count != 300) ? 500 : 5;
						std::random_device seed_rng;
						std::default_random_engine rng{seed_rng()};

						for (const auto i : iter::range(runs)) {
							SUBCASE("{}"_format(i).c_str()) {
								// TODO: reconsider -- bad for high count and low depth
								gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(count, 1.0 / depth))));
								gen.setValueMinMax(value_type(1), value_type(2));
								// std::cout << "key_part_max " << gen.getKeyPartMax() << "\n"
								// 		     << "value_max " << gen.getValueMax() << std::endl;
								gen.wind(i);

								std::vector<SingleEntry<depth, htt_t>> const entries = gen.entries(count);

								std::uniform_int_distribution<size_t> dist{entries.size() - 1, entries.size()};
								size_t const remove_count = dist(rng);

								std::vector<SingleEntry<depth, htt_t>> const to_remove{entries.begin(), entries.begin() + remove_count};
								std::vector<SingleEntry<depth, htt_t>> const diff{entries.begin() + remove_count, entries.end()};

								std::cout << fmt::format("entries: {{ {} }}", fmt::join(entries, ", \n")) << std::endl;
								std::cout << fmt::format("to_remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
								std::cout << fmt::format("diff: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

								ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), diff};

								RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
								NodeContainer<depth, htt_t, allocator_type> nc{};
								context.insert(nc, std::vector{entries});

								if (debug) {
									std::cout << "Before remove:\n";
									dump_context(context);
									dump_context_hash_translation_table(context);
									std::cout << "\nExpected after remove:\n";
									dump_context(validation_context);
									dump_context_hash_translation_table(validation_context);
								}

								context.remove(nc, std::vector{to_remove});

								if (debug) {
									std::cout << "\nActual after remove:\n";
									dump_context(context);
									dump_context_hash_translation_table(context);
								}

								std::cout << fmt::format("result identifier: {}", nc.raw_identifier()) << std::endl;

								REQUIRE(validation_context == context);
								for (const auto &entry : diff) {
									REQUIRE(context.get(nc, entry.key()) == entry.value());
								}
							}
						}
					}

					SUBCASE("insert 2x{} entries then remove random amount"_format(count).c_str()) {
						auto runs = (count != 300) ? 500 : 5;
						auto const total_count = 2 * count;

						std::random_device seed_rng;
						std::default_random_engine rng{seed_rng()};

						for (const auto i : iter::range(runs)) {
							SUBCASE("{}"_format(i).c_str()) {
								// TODO: reconsider -- bad for high count and low depth
								gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(total_count, 1.0 / depth))));
								gen.setValueMinMax(value_type(1), value_type(2));
								// std::cout << "key_part_max " << gen.getKeyPartMax() << "\n"
								// 		     << "value_max " << gen.getValueMax() << std::endl;
								gen.wind(i);

								std::vector<SingleEntry<depth, htt_t>> const entries = gen.entries(total_count);

								std::uniform_int_distribution<size_t> dist{0, entries.size()};
								size_t const remove_count = dist(rng);

								std::vector<SingleEntry<depth, htt_t>> const to_remove{entries.begin(), entries.begin() + remove_count};
								std::vector<SingleEntry<depth, htt_t>> const diff{entries.begin() + remove_count, entries.end()};

								std::cout << fmt::format("entries: {{ {} }}", fmt::join(entries, ", \n")) << std::endl;
								std::cout << fmt::format("to_remove: {{ {} }}", fmt::join(to_remove, ", \n")) << std::endl;
								std::cout << fmt::format("diff: {{ {} }}", fmt::join(diff, ", \n")) << std::endl;

								ValidationRawNodeContext<5, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), diff};

								RawHypertrieContext<5, htt_t, allocator_type> context{std::allocator<std::byte>()};
								NodeContainer<depth, htt_t, allocator_type> nc{};
								context.insert(nc, std::vector{entries});

								if (debug) {
									std::cout << "Before remove:\n";
									dump_context(context);
									dump_context_hash_translation_table(context);
									std::cout << "\nExpected after remove:\n";
									dump_context(validation_context);
									dump_context_hash_translation_table(validation_context);
								}

								context.remove(nc, std::vector{to_remove});

								if (debug) {
									std::cout << "\nActual after remove:\n";
									dump_context(context);
									dump_context_hash_translation_table(context);

									std::cout << fmt::format("result identifier: {}", nc.raw_identifier()) << std::endl;
								}

								REQUIRE(validation_context == context);
								for (const auto &entry : diff) {
									REQUIRE(context.get(nc, entry.key()) == entry.value());
								}
							}
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
					   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>
					   //
	) {
		insert_remove_read<T::depth, typename T::htt_t>();
	}

}// namespace dice::hypertrie::tests::core::node
