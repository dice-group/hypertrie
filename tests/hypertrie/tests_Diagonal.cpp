#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <dice/hypertrie.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>


namespace dice::hypertrie::tests {
	using allocator_type = std::allocator<std::byte>;
	using htt_t = tagged_bool_Hypertrie_trait;

	TEST_CASE("Diagonal Test SEN depth 1") {
		Hypertrie<htt_t, allocator_type> ht{1};
		ht.set({1}, true);
		auto diagonal = HashDiagonal(ht, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{0}});
		CHECK(diagonal.find(1));
		CHECK(diagonal.current_scalar());

		CHECK(not diagonal.find(2));
		// must not access diagonal.current_scalar() if find returned false
	}
	// FullNode
	TEST_CASE("Diagonal Test FN depth 1") {
		Hypertrie<htt_t, allocator_type> ht{1};
		ht.set({1}, true);
		ht.set({3}, true);
		auto diagonal = HashDiagonal(ht, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{0}});
		CHECK(diagonal.find(1));
		CHECK(diagonal.current_scalar());
		CHECK(not diagonal.find(2));
		// must not access diagonal.current_scalar() if find returned false
		CHECK(diagonal.find(3));
		CHECK(diagonal.current_scalar());
	}
}// namespace dice::hypertrie::tests