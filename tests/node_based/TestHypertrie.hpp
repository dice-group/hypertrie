#ifndef HYPERTRIE_TESTHYPERTRIE_H
#define HYPERTRIE_TESTHYPERTRIE_H

#include <Dice/hypertrie/internal/node_based/interface/Hypertrie.hpp>
#include <Dice/hypertrie/internal/node_based/interface/HypertrieContext.hpp>


#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"
#include "../utils/GenerateTriples.hpp"
#include "TestTensor.hpp"


namespace hypertrie::tests::node_based::node_context {
	using namespace fmt::literals;
	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::node_based;



	template<int depth>
	void test_single_write() {
		using tr = default_bool_Hypertrie_t;

		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		SECTION("Boolhypertrie depth {}"_format(depth)) {
			utils::EntryGenerator<depth, key_part_type, value_type> gen{};
			auto keys = gen.keys(500);
			for (const auto &key : keys) {
				Hypertrie<tr> t{depth};
				t.set(key, true);
				REQUIRE(t[key]);
			}
		}
	}

	TEST_CASE("test_single_write_read", "[BoolHypertrie]") {
		utils::resetDefaultRandomNumberGenerator();
		test_single_write<1>();
		test_single_write<2>();
		test_single_write<3>();
		test_single_write<4>();
		test_single_write<5>();
	}

	TEST_CASE("test_iterator", "[BoolHypertrie]") {
		using tr = default_bool_Hypertrie_t;
		constexpr const size_t depth = 4;
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;

		utils::EntryGenerator<depth, key_part_type, value_type> gen{};
		auto keys = gen.keys(5);

//		std::cout << keys << std::endl;

		Hypertrie<tr> t{depth};
		for (const auto &key : keys) {
			t.set(key, true);
			REQUIRE(t[key]);
		}

		for (const auto &key : t) {
			std::cout << key[0]  << ", " << key[1]  << ", " << key[2] << ", " << key[3] << std::endl;
		}

	}

};// namespace hypertrie::tests::node_based::node_context

#endif//HYPERTRIE_TESTHYPERTRIE_H
