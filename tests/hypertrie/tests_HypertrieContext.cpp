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

			SUBCASE("Add another entry") {
				hypertrie.set({{1, 2, 2}}, true);
				auto slice_0_var = hypertrie[SliceKey<tr>{{1, std::nullopt, std::nullopt}}];
				auto slice_0 = std::get<0>(slice_0_var);
				CHECK(slice_0[Key<tr>{{2, 2}}] == true);
				CHECK(slice_0[Key<tr>{{2, 3}}] == true);
				CHECK(slice_0[Key<tr>{{3, 2}}] == false);
			}
		};
	};
};// namespace hypertrie::tests::core::node