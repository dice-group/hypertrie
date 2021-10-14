#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include "ValidationRawNodeContext.hpp"
#include <Node_test_configs.hpp>
#include <RawEntryGenerator.hpp>


#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <Dice/hypertrie/internal/raw/iteration/RawHashDiagonal.hpp>

namespace hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawNodeContext") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		TEST_CASE("problematic entries 11") {
			using T = bool_cfg<4>;
			constexpr auto depth = T::depth;
			using tri = typename T::tri;
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;
			constexpr auto count = 2;

			std::vector<SingleEntry_t> all_entries{SingleEntry_t{{1, 1, 2, 1}, true},
												   SingleEntry_t{{1, 1, 1, 1}, true},
												   SingleEntry_t{{1, 1, 1, 2}, true},
												   SingleEntry_t{{1, 2, 1, 1}, true},
												   SingleEntry_t{{2, 1, 1, 1}, true}
			};

		};
	};
};// namespace hypertrie::tests::core::node