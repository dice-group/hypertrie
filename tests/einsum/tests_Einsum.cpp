#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include <RawEntryGenerator.hpp>

#include <Dice/einsum.hpp>
#include <Dice/hypertrie/Hypertrie_default_traits.hpp>


namespace Dice::einsum::tests {

	TEST_SUITE("Testing of HypertrieContext") {

		TEST_CASE("write and read a single entry") {
			using tr = hypertrie::tagged_bool_Hypertrie_trait;
			hypertrie::Hypertrie<tr> hypertrie{3};
			hypertrie.set({{1, 2, 3}}, true);
			auto sc = std::make_shared<Subscript>(Subscript ::from_string("abc->abc"));

			einsum2map<size_t, tr>(sc, {hypertrie});
		};
	};
};// namespace Dice::einsum::tests