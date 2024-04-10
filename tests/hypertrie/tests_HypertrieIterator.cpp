#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <dice/hypertrie.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>


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
				auto slice_1 = std::get<0>(op_0[{1, std::nullopt, std::nullopt}]);

				for ([[maybe_unused]] auto entry : slice_1)
					FAIL_CHECK("It's empty. This must not be hit.");
			}
		}
		TEST_CASE("iterate empty slice of non-empty depth 3 hypertrie") {
			Hypertrie<htt_t, allocator_type> op_0(3);
			op_0.set({1, 2, 3}, true);

			auto empty_slice = std::get<0>(op_0[{3, std::nullopt, 1}]);

			for ([[maybe_unused]] auto entry : empty_slice)
				FAIL_CHECK("It's empty. This must not be hit.");
		};

		TEST_CASE("iterate depth 1 single entry slice of single entry node") {
			Hypertrie<htt_t, allocator_type> op_0(3);
			op_0.set({1, 2, 3}, true);

			auto empty_slice = std::get<0>(op_0[{1, std::nullopt, 3}]);


			for ([[maybe_unused]] auto entry : empty_slice)
				CHECK(entry[0] == 2);
		};

		TEST_CASE("iterate depth 2 single entry slice of single entry node") {
			Hypertrie<htt_t, allocator_type> op_0(3);
			op_0.set({1, 2, 3}, true);

			auto slice_0 = std::get<0>(op_0[{1, std::nullopt, std::nullopt}]);

			for ([[maybe_unused]] auto entry : slice_0) {
				CHECK(entry[0] == 2);
				CHECK(entry[1] == 3);
			}

			SUBCASE("Reslice contextless hypertrie to [1,3,:]") {
				auto slice_1 = std::get<0>(slice_0[{3, std::nullopt}]);

				size_t count_1 = 0;
				for ([[maybe_unused]] auto entry : slice_1)
					CHECK(entry[1] == 3);
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
				auto slice_0 = std::get<0>(op_0[{std::nullopt, std::nullopt, std::nullopt}]);

				size_t count_0 = 0;
				for ([[maybe_unused]] auto entry : slice_0)
					count_0++;
				CHECK(count_0 == 8);
			}


			SUBCASE("Slice [1,:,:]") {
				auto slice_1 = std::get<0>(op_0[{1, std::nullopt, std::nullopt}]);

				size_t count_1 = 0;
				for ([[maybe_unused]] auto entry : slice_1)
					count_1++;
				CHECK(count_1 == 4);
			}

			SUBCASE("Empty slice [5,:,:]") {
				auto slice_1_empty = std::get<0>(op_0[{5, std::nullopt, std::nullopt}]);

				for ([[maybe_unused]] auto entry : slice_1_empty)
					FAIL_CHECK("It's empty. This must not be hit.");
			}

			SUBCASE("Slice [1,3,:]") {
				auto slice_2 = std::get<0>(op_0[{1, 3, std::nullopt}]);

				size_t count_2 = 0;
				for ([[maybe_unused]] auto entry : slice_2)
					count_2++;
				CHECK(count_2 == 2);
			}

			SUBCASE("Empty slice [1,1,:]") {
				auto slice_2_empty = std::get<0>(op_0[{1, 1, std::nullopt}]);

				for ([[maybe_unused]] auto entry : slice_2_empty)
					FAIL_CHECK("It's empty. This must not be hit.");
			}
		};
	};
};// namespace dice::hypertrie::tests::core::node