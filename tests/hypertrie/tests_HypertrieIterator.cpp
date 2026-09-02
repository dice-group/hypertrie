#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <dice/hypertrie.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>

#include <algorithm>

namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("Testing of Hypertrie' Iterator") {
		using namespace ::dice::hypertrie;

		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{};// allocator instance
		using htt_t = tagged_bool_Hypertrie_trait;


		TEST_CASE("iterate empty hypertrie, depth 3") {
			Hypertrie<htt_t, allocator_type> op_0(3);

			for ([[maybe_unused]] auto entry : op_0)
				FAIL_CHECK("It's empty. This must not be hit.");

			SUBCASE("Iterate slice of empty hypertrie") {
				auto slice_1 = op_0[{1, std::nullopt, std::nullopt}];

				for ([[maybe_unused]] auto entry : slice_1)
					FAIL_CHECK("It's empty. This must not be hit.");
			}
		}
		TEST_CASE("iterate empty slice of non-empty depth 3 hypertrie") {
			Hypertrie<htt_t, allocator_type> op_0(3);
			op_0.set({1, 2, 3}, true);

			auto empty_slice = op_0[{3, std::nullopt, 1}];

			for ([[maybe_unused]] auto entry : empty_slice)
				FAIL_CHECK("It's empty. This must not be hit.");
		};

		TEST_CASE("iterate depth 1 single entry slice of single entry node") {
			Hypertrie<htt_t, allocator_type> op_0(3);
			op_0.set({1, 2, 3}, true);

			auto empty_slice = op_0[{1, std::nullopt, 3}];


			for ([[maybe_unused]] auto entry : empty_slice)
				CHECK(entry.key()[0] == 2);
		};

		TEST_CASE("iterate depth 2 single entry slice of single entry node") {
			Hypertrie<htt_t, allocator_type> op_0(3);
			op_0.set({1, 2, 3}, true);

			auto slice_0 = op_0[{1, std::nullopt, std::nullopt}];

			for ([[maybe_unused]] auto entry : slice_0) {
				CHECK(entry.key()[0] == 2);
				CHECK(entry.key()[1] == 3);
			}

			SUBCASE("Reslice contextless hypertrie to [1,3,:]") {
				auto slice_1 = slice_0[{3, std::nullopt}];

				for (auto entry : slice_1)
					CHECK(entry.key()[1] == 3);
			}
		};

		TEST_CASE("iterate depth 3 node with 8 entries and check entry count") {
			Hypertrie<htt_t, allocator_type> op_0(3);
			op_0.set({1, 3, 5}, true);
			op_0.set({2, 3, 5}, true);
			op_0.set({1, 4, 5}, true);
			op_0.set({2, 4, 5}, true);
			op_0.set({1, 3, 6}, true);
			op_0.set({2, 3, 6}, true);
			op_0.set({1, 4, 6}, true);
			op_0.set({2, 4, 6}, true);

			SUBCASE("The whole hypertrie") {
				size_t count_0 = 0;
				for ([[maybe_unused]] auto entry : op_0)
					count_0++;
				CHECK(count_0 == 8);
			}

			SUBCASE("A full slice") {
				auto slice_0 = op_0[{std::nullopt, std::nullopt, std::nullopt}];

				size_t count_0 = 0;
				for ([[maybe_unused]] auto entry : slice_0)
					count_0++;
				CHECK(count_0 == 8);
			}


			SUBCASE("Slice [1,:,:]") {
				auto slice_1 = op_0[{1, std::nullopt, std::nullopt}];

				size_t count_1 = 0;
				for ([[maybe_unused]] auto entry : slice_1)
					count_1++;
				CHECK(count_1 == 4);
			}

			SUBCASE("Empty slice [5,:,:]") {
				auto slice_1_empty = op_0[{5, std::nullopt, std::nullopt}];

				for ([[maybe_unused]] auto entry : slice_1_empty)
					FAIL_CHECK("It's empty. This must not be hit.");
			}

			SUBCASE("Slice [1,3,:]") {
				auto slice_2 = op_0[{1, 3, std::nullopt}];

				size_t count_2 = 0;
				for ([[maybe_unused]] auto entry : slice_2)
					count_2++;
				CHECK(count_2 == 2);
			}

			SUBCASE("Empty slice [1,1,:]") {
				auto slice_2_empty = op_0[{1, 1, std::nullopt}];

				for ([[maybe_unused]] auto entry : slice_2_empty)
					FAIL_CHECK("It's empty. This must not be hit.");
			}
		};

		TEST_CASE("iterate depth 0") {
			Hypertrie<htt_t, allocator_type> hyp{0};
			for ([[maybe_unused]] auto const &e : hyp) {
				FAIL_CHECK("empty hypertrie is expected to have no entries");
			}

			hyp.set({}, true);

			size_t count = 0;
			for ([[maybe_unused]] auto const &e : hyp) {
				CHECK(e.size() == 0);
				CHECK(e.value() == true);
				count += 1;
			}

			CHECK(count == 1);
		}

		// TODO
		/*TEST_CASE("iterate depth 0 valued") {
			Hypertrie<default_long_Hypertrie_trait, allocator_type> hyp{0};
			for ([[maybe_unused]] auto const &e : hyp) {
				FAIL_CHECK("empty hypertrie is expected to have no entries");
			}

			hyp.set({}, 89);

			size_t count = 0;
			for ([[maybe_unused]] auto const &e : hyp) {
				CHECK(e.size() == 0);
				CHECK(e.value() == 89);
				count += 1;
			}

			CHECK(count == 1);
		}*/

		TEST_CASE("std algorithms") {
			Hypertrie<htt_t, allocator_type> hyp(3);
			hyp.set({1, 2, 3}, true);
			hyp.set({4, 5, 6}, true);

			SUBCASE("distance") {
				CHECK(std::ranges::distance(hyp.begin(), hyp.end()) == 2);
			}

			SUBCASE("copy and equal") {
				std::vector<NonZeroEntry<htt_t>> entries;
				entries.resize(2);
				std::ranges::copy(hyp, entries.begin());

				CHECK(std::ranges::equal(hyp, entries));
			}
		}
	};
};// namespace dice::hypertrie::tests::core::node