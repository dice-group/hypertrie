#ifndef HYPERTRIE_TESTHYPERTRIE_H
#define HYPERTRIE_TESTHYPERTRIE_H

#include <Dice/hypertrie/internal/Hypertrie.hpp>
#include <Dice/hypertrie/internal/HypertrieContext.hpp>


#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"
#include "../utils/GenerateTriples.hpp"
#include "TestTensor.hpp"


namespace hypertrie::tests::node_context {
	using namespace fmt::literals;
	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal;


	template<HypertrieTrait tr, int depth>
	void test_single_write() {
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		SECTION("Boolhypertrie depth {}"_format(depth)) {
			utils::EntryGenerator<key_part_type, value_type, tr::lsb_unused> gen{};
			auto keys = gen.keys(500, depth);
			for (const auto &key : keys) {
				HypertrieContext<tr> context;
				Hypertrie<tr> t{depth, context};
				WARN(fmt::format("[ {} ]", fmt::join(key, ", ")));
				t.set(key, true);
				REQUIRE(t[key]);
			}
		}
	}

	TEMPLATE_TEST_CASE("test_single_write_read", "[Hypertrie]", only_uc_bool_Hypertrie_t, default_bool_Hypertrie_t, lsbunused_bool_Hypertrie_t, default_long_Hypertrie_t, default_double_Hypertrie_t) {
		using tr = TestType;
		test_single_write<tr, 1>();
		test_single_write<tr, 2>();
		test_single_write<tr, 3>();
		test_single_write<tr, 4>();
		test_single_write<tr, 5>();
	}

	TEMPLATE_TEST_CASE("test_iterator", "[Hypertrie]", only_uc_bool_Hypertrie_t, default_bool_Hypertrie_t,lsbunused_bool_Hypertrie_t) {
		using tr = TestType;
		constexpr const size_t depth = 4;
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;

		utils::EntryGenerator<key_part_type, value_type, tr::lsb_unused> gen{1, 15};
		auto keys = gen.keys(150, depth);

		//		std::cout << keys << std::endl;

		HypertrieContext<tr> context;
		Hypertrie<tr> t{depth, context};
		for (const auto &key : keys) {
			//			WARN(fmt::format("[ {} ]",fmt::join(key, ", ")));
			t.set(key, true);
			REQUIRE(t[key]);
		}

		std::vector<typename tr::Key> actual_keys;
		for (const auto &key : t) {
//			WARN(fmt::format("[ {} ]",fmt::join(key, ", ")));
			actual_keys.push_back(key);
		}

		REQUIRE(keys.size() == actual_keys.size());

		for (const auto &actual_key : actual_keys) {
			REQUIRE(keys.count(actual_key));
		}
	}

	TEMPLATE_TEST_CASE("test_diagonal", "[Hypertrie]", only_uc_bool_Hypertrie_t, default_bool_Hypertrie_t,lsbunused_bool_Hypertrie_t) {
		using tr = TestType;
		constexpr const size_t depth = 4;
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;

		utils::EntryGenerator<key_part_type, value_type, tr::lsb_unused> gen{1, 15};
		auto keys = gen.keys(150, depth);

		//		std::cout << keys << std::endl;
		{
			HypertrieContext<tr> context;
			Hypertrie<tr> t{depth, context};
			t.set({2, 4, 6, 8}, true);
			HashDiagonal d{t, {2}};
			d.begin();
			REQUIRE(d != d.end());
			std::cout << d.currentKeyPart() << " → " << std::string(d.currentHypertrie()) << std::endl;
			++d;
			REQUIRE(d == d.end());
		}

		{
			HypertrieContext<tr> context;
			Hypertrie<tr> t{depth, context};
			t.set({2, 10, 6, 8}, true);
			t.set({2, 4, 6, 8}, true);
			HashDiagonal d{t, {1}};
			d.begin();
			REQUIRE(d != d.end());
			std::cout << d.currentKeyPart() << " → " << std::string(d.currentHypertrie()) << std::endl;
			++d;
			REQUIRE(d != d.end());
			std::cout << d.currentKeyPart() << " → " << std::string(d.currentHypertrie()) << std::endl;
			++d;
			REQUIRE(d == d.end());
		}

		{
			HypertrieContext<tr> context;
			Hypertrie<tr> t{depth, context};
			t.set({2, 10, 6, 8}, true);
			t.set({2, 4, 6, 8}, true);
			HashDiagonal d{t, {3}};
			d.begin();
			REQUIRE(d != d.end());
			std::cout << d.currentKeyPart() << " → " << std::string(d.currentHypertrie()) << std::endl;
			++d;
			REQUIRE(d == d.end());
		}
	}

	TEMPLATE_TEST_CASE("test_slice", "[Hypertrie]", only_uc_bool_Hypertrie_t, default_bool_Hypertrie_t,lsbunused_bool_Hypertrie_t) {
		using tr = lsbunused_bool_Hypertrie_t;
		constexpr const size_t depth = 4;

		HypertrieContext<tr> context;
		Hypertrie<tr> t{depth, context};
		//			WARN(fmt::format("[ {} ]",fmt::join(key, ", ")));
		t.set({2, 4, 6, 8}, true);
		t.set({2, 10, 12, 14}, true);
		t.set({2, 16, 18, 20}, true);
		WARN((std::string) t);

		typename tr::SliceKey slice_key = {2, 4, 6, {}};

		std::variant<const_Hypertrie<tr>, bool> result = t[slice_key];

		const_Hypertrie sliced_hypertrie = std::get<0>(result);

		WARN((std::string) sliced_hypertrie);
	}

	TEMPLATE_TEST_CASE("test_depth_0", "[Hypertrie]", only_uc_bool_Hypertrie_t, default_bool_Hypertrie_t,lsbunused_bool_Hypertrie_t) {
		using tr = lsbunused_bool_Hypertrie_t;
		constexpr const size_t depth = 0;

		using Key = tr::Key;

		HypertrieContext<tr> context;
		Hypertrie<tr> t{depth, context};
		auto value = t[Key{}];
		REQUIRE(value == tr::value_type(0));
		t.set({}, tr::value_type(1));

		value = t[Key{}];
		REQUIRE(value == tr::value_type(1));
		WARN((std::string) t);

		t.set({}, tr::value_type(0));

		value = t[Key{}];
		REQUIRE(value == tr::value_type(0));

		WARN((std::string) t);
	}

};// namespace hypertrie::tests::node_context

#endif//HYPERTRIE_TESTHYPERTRIE_H
