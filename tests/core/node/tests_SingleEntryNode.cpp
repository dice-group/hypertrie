#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <Dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/Hypertrie_default_traits.hpp>

namespace hypertrie::tests::core::node {

	TEST_SUITE("SingleEntryNode") {
		using namespace ::hypertrie::internal::raw;

		TEST_CASE("remove me later") {
			SingleEntryNode<3, Hypertrie_core_t<default_bool_Hypertrie_trait >> x;
			REQUIRE(x.size() == 1);

		}
	}
};// namespace hypertrie::tests::core::node