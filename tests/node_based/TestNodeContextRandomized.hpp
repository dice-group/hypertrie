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

	template<size_t depth, typename key_part_type>
	using Key = hypertrie::internal::RawKey<depth, key_part_type>;

	TEST_CASE("Test Randomized bulk long -> bool", "[NodeContext]") {
		using tr = default_bool_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, 0, 10> gen{};

		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc{};
		auto tt = TestTensor<depth, tr>::getPrimary();


		for (size_t count : iter::range(3,10))
			SECTION("insert {} key "_format(count)) {
				for (const auto i : iter::range(10)) {
					SECTION("{}"_format(i)) {
						// generate entries
						auto temp_keys = gen.keys(count);
						std::vector<Key> keys{temp_keys.begin(), temp_keys.end()};

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
						WARN("\n\n\nresulting hypertrie \n{}\n\n"_format((std::string) context.storage));
						// check if they were inserted correctly
						tt.checkContext(context);
					}
				}
			}
	}

	TEST_CASE("Test Randomized double bulk long -> bool", "[NodeContext]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, 0, 10> gen{};

		NodeContext<depth, tri> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tri> nc{};
		auto tt = TestTensor<depth, tri>::getPrimary();


		for (size_t count : iter::range(3,50))
			SECTION("insert {} key "_format(count)) {
				for (const auto i : iter::range(10)) {
					SECTION("{}"_format(i)) {
						// generate entries
						auto temp_keys = gen.keys(2*count);
						auto middle_it = temp_keys.begin();
						std::advance(middle_it,count);
						std::vector<std::vector<Key>> keyss{
								{temp_keys.begin(), middle_it},
								{middle_it, temp_keys.end()}};

						for (std::vector<Key> &keys: keyss){
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
							WARN("\n\n\nresulting hypertrie \n{}\n\n"_format((std::string) context.storage));
							// check if they were inserted correctly
							tt.checkContext(context);
						}
					}
				}
			}
	}

	TEST_CASE("Test Randomized double bulk long -> bool, unused_lsb", "[NodeContext]") {
		using tri = Hypertrie_internal_t<Hypertrie_t<unsigned long,
				bool,
				hypertrie::internal::container::std_map,
				hypertrie::internal::container::std_set,
				true>>;
		constexpr pos_type depth = 3;

		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, 0, 10> gen{};

		NodeContext<depth, tri> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tri> nc{};
		auto tt = TestTensor<depth, tri>::getPrimary();


		for (size_t count : iter::range(3,50))
			SECTION("insert {} key "_format(count)) {
				for (const auto i : iter::range(10)) {
					SECTION("{}"_format(i)) {
						// generate entries
						auto temp_keys = gen.keys(2*count);
						std::vector<Key> temp_keys_vec{temp_keys.begin(), temp_keys.end()};
						for (auto &key : temp_keys_vec){
							key[0] <<= 1;
							key[1] <<= 1;
							key[2] <<= 1;
						}
						// generate entries
						auto middle_it = temp_keys_vec.begin();
						std::advance(middle_it,count);
						std::vector<std::vector<Key>> keyss{
								{temp_keys_vec.begin(), middle_it},
								{middle_it, temp_keys_vec.end()}};

						for (std::vector<Key> &keys: keyss){
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
							WARN("\n\n\nresulting hypertrie \n{}\n\n"_format((std::string) context.storage));
							// check if they were inserted correctly
							tt.checkContext(context);
						}
					}
				}
			}
	}

	TEST_CASE("Test Randomized long -> bool", "[NodeContext]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, 0, 10> gen{};

		NodeContext<depth, tri> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tri> nc{};
		auto tt = TestTensor<depth, tri>::getPrimary();


		for (size_t count : iter::range(1,500))
			SECTION("insert {} key "_format(count)) {
				for (const auto i : iter::range(200)) {
					SECTION("{}"_format(i)) {
						// generate entries
						std::vector<Key> keys(count);
						for (auto &key : keys)
							key = gen.key();

						// print entries
						std::string print_entries{};
						for (auto &key : keys)
							print_entries += "{} → true\n"_format(key);
						WARN(print_entries);

						auto i = 1;
						// insert entries
						for (auto &key : keys) {

							context.template set<depth>(nc, key, true);
							tt.set(key, true);
							WARN("state {} : {}"_format(i++, (std::string) context.storage));

							tt.checkContext(context);
						}
					}
				}
			}
	}

	TEST_CASE("Test Randomized long -> bool, unused_lsb", "[NodeContext]") {
		using tri = Hypertrie_internal_t<Hypertrie_t<unsigned long,
				bool,
				hypertrie::internal::container::std_map,
				hypertrie::internal::container::std_set,
													  true>>;
		constexpr pos_type depth = 3;

		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, 0, 10> gen{};

		NodeContext<depth, tri> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tri> nc{};
		auto tt = TestTensor<depth, tri>::getPrimary();


		for (size_t count : iter::range(1,500))
			SECTION("insert {} key "_format(count)) {
				for (const auto i : iter::range(200)) {
					SECTION("{}"_format(i)) {
						// generate entries
						std::vector<Key> keys(count);
						for (auto &key : keys){
							key = gen.key();
							key[0] <<= 1;
							key[1] <<= 1;
							key[2] <<= 1;
						}

						// print entries
						std::string print_entries{};
						for (auto &key : keys)
							print_entries += "{} → true\n"_format(key);
						WARN(print_entries);

						auto i = 1;
						// insert entries
						for (auto &key : keys) {

							context.template set<depth>(nc, key, true);
							tt.set(key, true);
							WARN("state {} : {}"_format(i++, (std::string) context.storage));

							tt.checkContext(context);
						}
					}
				}
			}
	}

	TEST_CASE("Test Randomized long -> long", "[NodeContext]") {
		using tri = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, 0, 10> gen{0, 5};

		NodeContext<depth, tri> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tri> nc{};
		auto tt = TestTensor<depth, tri>::getPrimary();


		for (size_t count : iter::range(1, 500))
			SECTION("insert {} key "_format(count)) {
				for (const auto i : iter::range(200)) {
					SECTION("{}"_format(i)) {
						// generate entries
						std::vector<std::pair<Key, value_type>> entries(count);
						for (auto &entry : entries)
							entry = gen.entry();

						// print entries
						std::string print_entries{};
						for (auto &[key, value] : entries)
							print_entries += "{} → {}\n"_format(key, value);
						WARN(print_entries);

						// insert entries
						for (auto &[key, value] : entries) {

							context.template set<depth>(nc, key, value);
							tt.set(key, value);

							tt.checkContext(context);
						}
					}
				}
			}
	}

	TEST_CASE("Test Randomized long -> long many keys", "[NodeContext]") {
		using tri = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, 0, 10> gen{0, 5};

		NodeContext<depth, tri> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tri> nc{};
		auto tt = TestTensor<depth, tri>::getPrimary();

		auto count = 500;
		SECTION("insert {} key "_format(count)) {
			for (const auto i : iter::range(1000)) {
				SECTION("{}"_format(i)) {
					// generate entries
					std::vector<std::pair<Key, value_type>> entries(count);
					for (auto &entry : entries)
						entry = gen.entry();

					// print entries
					std::string print_entries{};
					for (auto &[key, value] : entries)
						print_entries += "{} → {}\n"_format(key, value);
					WARN(print_entries);

					// insert entries
					for (auto &[key, value] : entries) {

						context.template set<depth>(nc, key, value);
						tt.set(key, value);

						tt.checkContext(context);
					}
				}
			}
		}
	}
};// namespace hypertrie::tests::node_context

#endif//HYPERTRIE_TESTNODECONTEXTRANDOMIZED_H
