#ifndef HYPERTRIE_TESTNODECONTEXTRANDOMIZED_H
#define HYPERTRIE_TESTNODECONTEXTRANDOMIZED_H

#include <Dice/hypertrie/internal/node_based/NodeContext.hpp>

#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"
#include "TestTensor.hpp"


namespace hypertrie::tests::node_based::node_context::randomized {

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::node_based;

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
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		for (size_t count : iter::range(5,10))
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
						// check if they were inserted correctly
						tt.checkContext(context);
					}
				}
			}
	}

	TEST_CASE("Test Randomized long -> bool", "[NodeContext]") {
		using tr = default_bool_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, 0, 10> gen{};

		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


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

						// insert entries
						for (auto &key : keys) {

							context.template set<depth>(nc, key, true);
							tt.set(key, true);

							tt.checkContext(context);
						}
					}
				}
			}
	}

	TEST_CASE("Test Randomized long -> long", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, 0, 10> gen{0, 5};

		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


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
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, 0, 10> gen{0, 5};

		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();

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

	TEST_CASE("Test specific case long -> bool 1", "[NodeContext]") {
		using tr = default_bool_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<Key> keys{
				{10, 2, 2},
				{3, 2, 2},
				{1, 2, 0}};

		// print entries
		std::string print_entries{};
		for (auto &key : keys)
			print_entries += "{} → true\n"_format(key);
		WARN(print_entries);

		// insert entries
		int i = 0;
		for (auto &key : keys) {

			context.template set<depth>(nc, key, true);
			tt.set(key, true);
			std::cout << "state " << i++ << " " << context.storage << std::endl;

			tt.checkContext(context);
		}
	}

	TEST_CASE("Test specific case long -> bool 2", "[NodeContext]") {
		using tr = default_bool_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<Key> keys{
				{9, 2, 4},
				{10, 4, 9},
				{5, 3, 4},
				{10, 7, 7},
				{6, 0, 0},
				{2, 5, 6},
				{10, 6, 1},
				{3, 1, 2},
				{1, 3, 3},
				{3, 10, 4},
				{10, 7, 5},
				{0, 3, 3},
				{5, 4, 3},
				{9, 6, 6},
				{5, 6, 8},
				{2, 3, 9},
				{8, 10, 9},
				{8, 0, 8}};

		// print entries
		std::string print_entries{};
		for (auto &key : keys)
			print_entries += "{} → true\n"_format(key);
		WARN(print_entries);

		// insert entries
		int i = 0;
		WARN("state {} : {}"_format( i++,(std::string)context.storage) );
		for (auto &key : keys) {

			context.template set<depth>(nc, key, true);
			tt.set(key, true);
			WARN("state {} : {}"_format( i++,(std::string)context.storage) );

			tt.checkContext(context);
		}
	}

	TEST_CASE("Test specific case 1", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<std::pair<Key, value_type>> entries{
				{{7, 1, 3}, {4}},
				{{7, 4, 3}, {1}}};

		// print entries
		std::string print_entries{};
		for (auto &[key, value] : entries)
			print_entries += "{} → {}\n"_format(key, value);
		WARN(print_entries);

		// insert entries
		int i = 0;
		for (auto &[key, value] : entries) {

			context.template set<depth>(nc, key, value);
			std::cout << "state " << i++ << " " << context.storage << std::endl;
			tt.set(key, value);

			tt.checkContext(context);
		}
	}

	TEST_CASE("Test specific case 2", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<std::pair<Key, value_type>> entries{
				{{6, 6, 1}, {4}},
				{{6, 6, 2}, {5}}};

		// print entries
		std::string print_entries{};
		for (auto &[key, value] : entries)
			print_entries += "{} → {}\n"_format(key, value);
		WARN(print_entries);

		// insert entries
		int i = 0;
		for (auto &[key, value] : entries) {

			context.template set<depth>(nc, key, value);
			std::cout << "state " << i++ << " " << context.storage << std::endl;
			tt.set(key, value);

			tt.checkContext(context);
		}
	}


	TEST_CASE("Test specific case 3", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<std::pair<Key, value_type>> entries{
				{{1, 8, 6}, {4}},
				{{6, 3, 6}, {4}},
				{{1, 8, 6}, {5}}};

		// print entries
		std::string print_entries{};
		for (auto &[key, value] : entries)
			print_entries += "{} → {}\n"_format(key, value);
		WARN(print_entries);

		// insert entries
		int i = 0;
		for (auto &[key, value] : entries) {

			context.template set<depth>(nc, key, value);
			std::cout << "state " << i++ << " " << context.storage << std::endl;
			tt.set(key, value);

			tt.checkContext(context);
		}
	}

	TEST_CASE("Test specific case 4", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<std::pair<Key, value_type>> entries{
				{{6, 5, 7}, {2}},
				{{6, 5, 3}, {2}},
				{{3, 7, 5}, {2}},
				{{6, 5, 1}, {2}}};

		// print entries
		std::string print_entries{};
		for (auto &[key, value] : entries)
			print_entries += "{} → {}\n"_format(key, value);
		WARN(print_entries);

		// insert entries
		int i = 0;
		for (auto &[key, value] : entries) {

			context.template set<depth>(nc, key, value);
			std::cout << "state " << i++ << " " << context.storage << std::endl;
			tt.set(key, value);

			tt.checkContext(context);
		}
	}

	TEST_CASE("Test specific case 5", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<std::pair<Key, value_type>> entries{
				{{3, 5, 8}, {2}},
				{{8, 4, 5}, {2}},
				{{8, 4, 8}, {3}},
				{{8, 4, 3}, {3}}};

		// print entries
		std::string print_entries{};
		for (auto &[key, value] : entries)
			print_entries += "{} → {}\n"_format(key, value);
		WARN(print_entries);

		// insert entries
		int i = 0;
		for (auto &[key, value] : entries) {

			context.template set<depth>(nc, key, value);
			std::cout << "state " << i++ << " " << context.storage << std::endl;
			tt.set(key, value);

			tt.checkContext(context);
		}
	}

	TEST_CASE("Test specific case 6", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<std::pair<Key, value_type>> entries{
				{{7, 3, 7}, {2}},
				{{7, 4, 7}, {2}},
				{{7, 4, 7}, {5}}};

		// TODO: prblem: 77->2 refcount should be 1 instead of 2
		// Problem here: A value change can actually mean that I need to create a new node if there is another entry, that uses the same structure but with the old value.

		// print entries
		std::string print_entries{};
		for (auto &[key, value] : entries)
			print_entries += "{} → {}\n"_format(key, value);
		WARN(print_entries);

		// insert entries
		int i = 0;
		for (auto &[key, value] : entries) {

			context.template set<depth>(nc, key, value);
			std::cout << "state " << i++ << " " << context.storage << std::endl;
			tt.set(key, value);

			tt.checkContext(context);
		}
	}

	TEST_CASE("Test specific case 7", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<std::pair<Key, value_type>> entries{
				{{1, 1, 3}, {4}},
				{{1, 1, 1}, {4}},
				{{2, 3, 10}, {4}},
				{{8, 1, 6}, {5}}};

		// print entries
		std::string print_entries{};
		for (auto &[key, value] : entries)
			print_entries += "{} → {}\n"_format(key, value);
		WARN(print_entries);

		// insert entries
		int i = 0;
		for (auto &[key, value] : entries) {

			context.template set<depth>(nc, key, value);
			std::cout << "state " << i++ << " " << context.storage << std::endl;
			tt.set(key, value);

			tt.checkContext(context);
		}
	}

	TEST_CASE("Test specific case 8", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<std::pair<Key, value_type>> entries{
				{{6, 0, 2}, {4}},
				{{10, 7, 0}, {2}},
				{{10, 10, 4}, {4}},
				{{7, 10, 0}, {2}}};

		// print entries
		std::string print_entries{};
		for (auto &[key, value] : entries)
			print_entries += "{} → {}\n"_format(key, value);
		WARN(print_entries);

		// insert entries
		int i = 0;
		for (auto &[key, value] : entries) {

			context.template set<depth>(nc, key, value);
			std::cout << "state " << i++ << " " << context.storage << std::endl;
			tt.set(key, value);

			tt.checkContext(context);
		}
	}

	TEST_CASE("Test specific case 9", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		// generate entries
		std::vector<std::pair<Key, value_type>> entries{
				{{8, 7, 8}, {2}},
				{{7, 10, 10}, {2}},
				{{2, 3, 4}, {5}},
				{{6, 1, 3}, {1}},
				{{2, 10, 5}, {4}},
				{{7, 10, 10}, {4}}
				//{{0, 0, 10}, {3}}
		};

		// print entries
		std::string print_entries{};
		for (auto &[key, value] : entries)
			print_entries += "{} → {}\n"_format(key, value);
		WARN(print_entries);

		// insert entries
		int i = 0;
		WARN("state {} : {}"_format( i++,(std::string)context.storage) );
		for (auto &[key, value] : entries) {

			context.template set<depth>(nc, key, value);
			WARN("state {} : {}"_format( i++,(std::string)context.storage) );
			tt.set(key, value);

			tt.checkContext(context);
		}
	}


};// namespace hypertrie::tests::node_based::node_context

#endif//HYPERTRIE_TESTNODECONTEXTRANDOMIZED_H
