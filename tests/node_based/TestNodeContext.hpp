#ifndef HYPERTRIE_TESTNODECONTEXT_H
#define HYPERTRIE_TESTNODECONTEXT_H

#include <Dice/hypertrie/internal/node_based/NodeContext.hpp>

#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"
#include "TestTensor.hpp"


namespace hypertrie::tests::node_based::node_context {

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::node_based;

	template<size_t depth, typename key_part_type>
	using Key = hypertrie::internal::RawKey<depth, key_part_type>;

	template<HypertrieInternalTrait tr, pos_type depth>
	void basicUsage() {
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;

		SECTION(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
							depth, nameOfType<key_part_type>(), nameOfType<value_type>())) {
			utils::RawGenerator<depth, key_part_type, value_type> gen{};
			for (auto i : iter::range(5000)) {
				// create context
				NodeContext<depth, tr> context{};
				// create emtpy primary node
				UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();

				const auto entries = gen.entries(2);
				const auto [key, value] = *entries.begin();
				const auto [second_key, second_value] = *entries.rbegin();

				// insert value
				context.template set<depth>(nc, key, value);

				// get value
				{
					auto read_value = context.get(nc, key);
					REQUIRE(read_value == value);
				}

				// insert another value

				context.template set<depth>(nc, second_key, second_value);

				// get both values
				{
					auto read_value = context.get(nc, key);
					REQUIRE(read_value == value);
				}

				{
					auto read_value = context.get(nc, second_key);
					REQUIRE(read_value == second_value);
				}
			}
		}
	}

	TEST_CASE("Test setting independent keys", "[NodeContext]") {
		basicUsage<default_bool_Hypertrie_internal_t, 1>();
		basicUsage<default_long_Hypertrie_internal_t, 1>();
		basicUsage<default_double_Hypertrie_internal_t, 1>();
		basicUsage<default_bool_Hypertrie_internal_t, 2>();
		basicUsage<default_long_Hypertrie_internal_t, 2>();
		basicUsage<default_double_Hypertrie_internal_t, 2>();
		basicUsage<default_bool_Hypertrie_internal_t, 3>();
		basicUsage<default_long_Hypertrie_internal_t, 3>();
		basicUsage<default_double_Hypertrie_internal_t, 3>();
		basicUsage<default_bool_Hypertrie_internal_t, 5>();
		basicUsage<default_long_Hypertrie_internal_t, 5>();
		basicUsage<default_double_Hypertrie_internal_t, 5>();
	}

	TEST_CASE("Test Long valued", "[NodeContext]") {
		using tr = default_long_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;

		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();

		SECTION("write a single entry") {
			context.template set<depth>(nc, {1, 2, 3}, 1);
			tt.set({1, 2, 3}, 1);

			tt.checkContext(context);

			SECTION("twice") {
				context.template set<depth>(nc, {1, 2, 3}, 1);
				tt.set({1, 2, 3}, 1);

				tt.checkContext(context);
			}


			SECTION("change it") {
				//				std::cout << "before" << context.storage << std::endl;
				context.template set<depth>(nc, {1, 2, 3}, 2);
				tt.set({1, 2, 3}, 2);

				tt.checkContext(context);
			}
		}
	}

	TEST_CASE("Test specific case bulk long -> bool", "[NodeContext]") {
		using tr = default_bool_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		static std::vector<
				std::vector<Key>> test_entries{
				{
						{0, 0, 8},
						{1, 1, 5},
						{1, 4, 1}
				}
		};

		for(const auto &[case_, entries] : iter::enumerate(test_entries)) {
			SECTION("case {}"_format(case_)) {
				// print entries
				std::string print_entries{};
				for (auto &key : entries)
					print_entries += "{} → true\n"_format(key);
				WARN(print_entries);

				// insert entries into test tensor
				for (auto &key : entries) {
					tt.set(key, true);
				}

				// bulk insert keys
				context.template bulk_insert<depth>(nc, entries);
				WARN("\n\n\nresulting hypertrie \n{}\n\n"_format((std::string) context.storage));
				// check if they were inserted correctly
				tt.checkContext(context);
			}
		}
	}

	TEST_CASE("Test specific case long -> bool", "[NodeContext]") {
		using tr = default_bool_Hypertrie_internal_t;
		constexpr pos_type depth = 3;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using Key = typename tr::template RawKey<depth>;


		NodeContext<depth, tr> context{};
		// create emtpy primary node
		UncompressedNodeContainer<depth, tr> nc = context.template newPrimaryNode<depth>();
		auto tt = TestTensor<depth, tr>::getPrimary();


		static std::vector<
				std::vector<Key>> test_entries{
				{
					{10, 2, 2},
					{3, 2, 2},
					{1, 2, 0}
				},
				{
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
					{8, 0, 8}
				}
		};
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

		for(const auto &[case_, entries] : iter::enumerate(test_entries)) {
			SECTION("case {}"_format(case_)) {
				// print entries
				std::string print_entries{};
				for (auto &key : keys)
					print_entries += "{} → true\n"_format(key);
				WARN(print_entries);

				// insert entries
				int i = 0;
				WARN("state {} : {}"_format(i++, (std::string) context.storage));
				for (auto &key : keys) {

					context.template set<depth>(nc, key, true);
					tt.set(key, true);
					WARN("state {} : {}"_format(i++, (std::string) context.storage));

					tt.checkContext(context);
				}
			}
		}
	}

	TEST_CASE("Test specific cases long -> long", "[NodeContext]") {
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
		static std::vector<
		 std::vector<std::pair<Key, value_type>>> test_entries{
			{
				{{7, 1, 3}, {4}},
				{{7, 4, 3}, {1}}
			},
			{
				{{6, 6, 1}, {4}},
				{{6, 6, 2}, {5}}
			},
			{
				{{1, 8, 6}, {4}},
				{{6, 3, 6}, {4}},
				{{1, 8, 6}, {5}}
			},
			{
				{{6, 5, 7}, {2}},
				{{6, 5, 3}, {2}},
				{{3, 7, 5}, {2}},
				{{6, 5, 1}, {2}}
			},
			{
				{{3, 5, 8}, {2}},
				{{8, 4, 5}, {2}},
				{{8, 4, 8}, {3}},
				{{8, 4, 3}, {3}}
			},
			{
				{{7, 3, 7}, {2}},
				{{7, 4, 7}, {2}},
				{{7, 4, 7}, {5}}
			},
			{
				{{1, 1, 3}, {4}},
				{{1, 1, 1}, {4}},
				{{2, 3, 10}, {4}},
				{{8, 1, 6}, {5}}
			},
			{
				{{6, 0, 2}, {4}},
				{{10, 7, 0}, {2}},
				{{10, 10, 4}, {4}},
				{{7, 10, 0}, {2}}
			},
			{
				{{8, 7, 8}, {2}},
				{{7, 10, 10}, {2}},
				{{2, 3, 4}, {5}},
				{{6, 1, 3}, {1}},
				{{2, 10, 5}, {4}},
				{{7, 10, 10}, {4}}
			}
		};

		for(const auto &[case_, entries] : iter::enumerate(test_entries)){
			SECTION("case {}"_format(case_)) {
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
		}
	}


};// namespace hypertrie::tests::node_based::node_context

#endif//HYPERTRIE_TESTNODECONTEXT_H
