#ifndef HYPERTRIE_TESTNODECONTEXTRANDOMIZED_H
#define HYPERTRIE_TESTNODECONTEXTRANDOMIZED_H

#include <iterator>

#include <Dice/hypertrie/internal/raw/storage/NodeContext.hpp>

#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"
#include "TestTensor.hpp"


namespace hypertrie::tests::raw::node_context::randomized {

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::raw;


	template<HypertrieInternalTrait tri, size_t depth>
	void randomized_bulk_load() {
		SECTION("{}"_format(tri::to_string())) {
			static_assert(std::is_same_v<typename tri::value_type, bool>, "bulk-loading is only supported for boolean valued hypertries yet.");
			using key_part_type = typename tri::key_part_type;
			using value_type = typename tri::value_type;
			using Key = typename tri::template RawKey<depth>;

			static utils::RawGenerator<depth, key_part_type, value_type, size_t(tri::is_lsb_unused)> gen{};

			NodeContext<depth, tri> context{};
			// create emtpy primary node
			NodeContainer<depth, tri> nc{};
			auto tt = TestTensor<depth, tri>::getPrimary();



			for (size_t count : iter::range(3, 25)) {
				gen.setKeyPartMinMax(0,size_t((count/depth + 1) * 1.5));
				SECTION("insert {} keys "_format(count)) {
					for (const auto i : iter::range(10)) {
						SECTION("{}"_format(i)) {
							// generate entries
							auto temp_keys = gen.keys(count);
							auto middle_it = temp_keys.begin();
							std::advance(middle_it, count / 2);
							std::vector<std::vector<Key>> keyss{
									{temp_keys.begin(), middle_it},
									{middle_it, temp_keys.end()}};

							for (auto [no, keys] : iter::enumerate(keyss)) {
								SECTION("insert key set {}"_format(no + 1)) {
									// print entries
									std::string print_entries{};
									for (auto &key : keys)
										print_entries += "{} → true\n"_format(key);
									WARN(print_entries);

									// insert entries into test tensor
									for (auto &key : keys) {
										tt.set(key, true);
									}

									// bulk insert keys
									context.template bulk_insert<depth>(nc, keys);
									INFO("\n\n\nresulting hypertrie \n{}\n\n"_format((std::string) context.storage));
									// check if they were inserted correctly
									tt.checkContext(context);
								}
							}
						}
					}
				}
			}
		}
	}

	TEMPLATE_TEST_CASE_SIG("Randomized double bulk loading [bool]", "[NodeContext]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_bulk_load<default_bool_Hypertrie_internal_t, size_t(depth)>();
	}

	TEMPLATE_TEST_CASE_SIG("Randomized double bulk loading [bool lsb-unused]", "[NodeContext]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_bulk_load<lsbunused_bool_Hypertrie_internal_t, size_t(depth)>();
	}

	template<HypertrieInternalTrait tri, size_t depth>
	void randomized_insert_and_read() {
		SECTION("{}"_format(tri::to_string())) {

			using key_part_type = typename tri::key_part_type;
			using value_type = typename tri::value_type;
			using Key = typename tri::template RawKey<depth>;

			static utils::RawGenerator<depth, key_part_type, value_type, size_t(tri::is_lsb_unused)> gen{};

			NodeContext<depth, tri> context{};
			// create emtpy primary node
			UncompressedNodeContainer<depth, tri> nc{};
			auto test_tensor = TestTensor<depth, tri>::getPrimary();


			for (size_t count : iter::chain(iter::range(1, 10), iter::range(10, 30, 5), iter::range(300, 301))) {
				gen.setKeyPartMinMax(0, count / depth + 1);
				gen.setValueMinMax(value_type(0), value_type(count/10 + 1));
				SECTION("insert {} keys "_format(count)) {
					auto runs = (count != 300) ? 25 : 1;
					for (const auto i : iter::range(runs)) {
						SECTION("{}"_format(i)) {
							// generate entries
							using entry_type = std::pair<Key, value_type>;
							std::vector<entry_type> entries(count);
							for (auto &entry : entries) {
								entry = gen.entry();
								// TODO: remove when deleting entries is supported.
								if (entry.second == value_type(0))
									entry.second = value_type(1);
							}

							// print entries
							std::string print_entries{};
							for (const auto &[key, value] : entries)
								print_entries += "{} → {}\n"_format(key, value);
							WARN(print_entries);

							auto i = 1;
							// insert entries

							std::map<Key, value_type> expected_entries{};
							for (auto &[key, value] : entries) {

								// set value on node_container
								context.template set<depth>(nc, key, value);

								// track changes
								expected_entries[key] = value;
								test_tensor.set(key, value);

								// print current state
								INFO("state {} : {}"_format(i++, (std::string) context.storage));

								// check if everything is alight
								if (not (count == 300 and i != 300)) // check only the final result for inserting 500 entries
									test_tensor.checkContext(context);

								// check if for keys which were set so far, the right value is returned.
								for (const auto &[expected_key, expected_value] : expected_entries)
									REQUIRE(context.template get(nc, expected_key) == expected_value);
							}
						}
					}
				}
			}
		}
	}

	TEMPLATE_TEST_CASE_SIG("Randomized insert and read [bool]", "[NodeContext]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_insert_and_read<default_bool_Hypertrie_internal_t, size_t(depth)>();
	}

	TEMPLATE_TEST_CASE_SIG("Randomized insert and read [bool lsb-unused]", "[NodeContext]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_insert_and_read<lsbunused_bool_Hypertrie_internal_t, size_t(depth)>();
	}

	TEMPLATE_TEST_CASE_SIG("Randomized insert and read [long]", "[NodeContext]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_insert_and_read<default_long_Hypertrie_internal_t, size_t(depth)>();
	}

	TEMPLATE_TEST_CASE_SIG("Randomized insert and read [double]", "[NodeContext]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_insert_and_read<default_double_Hypertrie_internal_t, size_t(depth)>();
	}

};// namespace hypertrie::tests::node_context

#endif//HYPERTRIE_TESTNODECONTEXTRANDOMIZED_H
