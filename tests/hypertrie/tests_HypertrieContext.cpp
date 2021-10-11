#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include <RawEntryGenerator.hpp>

#include <Dice/hypertrie/Hypertrie.hpp>
#include <Dice/hypertrie/HypertrieContext.hpp>
#include <Dice/hypertrie/internal/Hypertrie_default_traits.hpp>


namespace hypertrie::tests::core::node {

	TEST_SUITE("Testing of HypertrieContext") {
		using namespace ::hypertrie;

		TEST_CASE("write and read a single entry") {
			using tr = tagged_bool_Hypertrie_trait;
			Hypertrie<tr> hypertrie{3};
			hypertrie.set({{1, 2, 3}}, true);

			auto result = hypertrie[SliceKey<tr>{{1, 2, 3}}];
			CHECK(std::get<bool>(result) == true);
			CHECK(hypertrie[Key<tr>{{1, 2, 3}}] == true);
			CHECK(hypertrie[Key<tr>{{0, 0, 0}}] == false);
		};
	};
};// namespace hypertrie::tests::core::node