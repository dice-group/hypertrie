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
		HashDiagonal diagonal(ht, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{0}});
		CHECK(diagonal.find(1));
		CHECK(diagonal.current_diagonal().to_scalar());

		CHECK(not diagonal.find(2));
		// must not access diagonal.current_diagonal() if find returned false
	}

	TEST_CASE("Diagonal Test FN depth 1") {
		Hypertrie<htt_t, allocator_type> ht{1};
		ht.set({1}, true);
		ht.set({3}, true);
		HashDiagonal diagonal(ht, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{0}});
		CHECK(diagonal.find(1));
		CHECK(diagonal.current_diagonal().to_scalar());
		CHECK(not diagonal.find(2));
		// must not access diagonal.current_diagonal() if find returned false
		CHECK(diagonal.find(3));
		CHECK(diagonal.current_diagonal().to_scalar());
	}

	TEST_CASE("Diagonal Test XN depth 2") {
		Hypertrie<htt_t, allocator_type> ht{2};
		ht.set({1, 1}, true);
		ht.set({2, 1}, true);

		HashDiagonal diagonal(ht, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{0}});
		CHECK(diagonal.find(1));
		CHECK(diagonal.current_diagonal()[Key<htt_t>{1}]);

		CHECK(diagonal.find(2));
		CHECK(diagonal.current_diagonal()[Key<htt_t>{1}]);

		CHECK(!diagonal.find(3));
	}

	TEST_CASE("depth 0 results") {
		Hypertrie<default_long_Hypertrie_trait, allocator_type> hyp{2};
		hyp.set({1, 1}, 1);
		hyp.set({2, 2}, 2);
		hyp.set({3, 3}, 3);

		HashDiagonal diagonal{hyp, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{0, 1}}};
		CHECK(diagonal.find(1));
		CHECK(diagonal.current_diagonal() == Hypertrie<default_long_Hypertrie_trait, allocator_type>::from_scalar(1));

		CHECK(diagonal.find(2));
		CHECK(diagonal.current_diagonal() == Hypertrie<default_long_Hypertrie_trait, allocator_type>::from_scalar(2));

		CHECK(diagonal.find(3));
		CHECK(diagonal.current_diagonal() == Hypertrie<default_long_Hypertrie_trait, allocator_type>::from_scalar(3));
	}
}// namespace dice::hypertrie::tests