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
				SUBCASE("Diagonal") {
					HashDiagonal<tr> hash_diagonal(slice_0, ::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{0, 1}});

					CHECK(hash_diagonal.find(2));
					CHECK(not hash_diagonal.find(3));
				}

				SUBCASE("Iterator") {
					for (const auto &entry : slice_0) {
						auto [key, value] = entry.tuple();
						fmt::print("{} -> {}\n", fmt::join(key, ", "), value);
					}
				}
			}
			SUBCASE("Diagonal") {
				HashDiagonal<tr> hash_diagonal(hypertrie, ::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{1, 2}});

				CHECK(hash_diagonal.find(2));
				CHECK(not hash_diagonal.find(3));
			}

			SUBCASE("Iterator") {
				for (const auto &entry : hypertrie) {
					auto [key, value] = entry.tuple();
					fmt::print("{} -> {}\n", fmt::join(key, ", "), value);
				}
			}

			SUBCASE("Hypertrie2") {
				Hypertrie<tr> hypertrie2{3};
				hypertrie.set({{1, 2, 3}}, true);
				auto slice_12 = std::get<0>(hypertrie[SliceKey<tr>{{std::nullopt, 2, 3}}]);
				for (const auto &entry : slice_12) {
					auto [key, value] = entry.tuple();
					fmt::print("{} -> {}\n", fmt::join(key, ", "), value);
				}
			}
		};
	};
};// namespace hypertrie::tests::core::node