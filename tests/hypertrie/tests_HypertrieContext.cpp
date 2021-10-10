#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include <RawEntryGenerator.hpp>

#include <Dice/hypertrie/HypertrieContext.hpp>
#include <Dice/hypertrie/Hypertrie.hpp>



namespace hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawNodeContext") {
		using namespace ::hypertrie;

		TEST_CASE("problematic entries") {
				Hypertrie<tagged_bool_Hypertrie_trait> hypertrie;
		};
	};
};// namespace hypertrie::tests::core::node