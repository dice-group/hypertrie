#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include <AssetGenerator.hpp>
#include <Dice/hypertrie/internal/util/name_of_type.hpp>
#include <Node_test_configs.hpp>

#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>


namespace hypertrie::tests::core::node {

	TEST_SUITE("NodeStorage") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		template<size_t depth, HypertrieCoreTrait tri>
		void create() {
			//			using key_part_type = typename tri::key_part_type;
			//			using value_type = typename tri::value_type;
		}

		TEST_CASE("storage") {
			using tri = typename tagged_bool_cfg<2>::tri;
			constexpr size_t depth = tagged_bool_cfg<2>::depth;
			hypertrie::tests::utils::RawGenerator<depth, tri> gen{0, 3};

			RawHypertrieContext<depth, tri> x{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			SUBCASE("insert nothing") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
				x.insert(nc, entries);
			}

			SUBCASE("insert one") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries = [&]() {
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
					for (auto &&entry : gen.entries(1))
						entries.emplace_back(entry.first, entry.second);
					return entries;
				}();
				x.insert(nc, entries);
			}

			SUBCASE("insert two") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries = [&]() {
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
					for (auto &&entry : gen.entries(2))
						entries.emplace_back(entry.first, entry.second);
					return entries;
				}();
				x.insert(nc, entries);
			}

			SUBCASE("insert 10") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries = [&]() {
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
					for (auto &&entry : gen.entries(10))
						entries.emplace_back(entry.first, entry.second);
					return entries;
				}();
				x.insert(nc, entries);
			}

			SUBCASE("insert 1 + 9") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries = [&]() {
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
					for (auto &&entry : gen.entries(10))
						entries.emplace_back(entry.first, entry.second);
					return entries;
				}();
				auto entries_1 = decltype(entries){entries[0]};
				auto entries_2_10 = decltype(entries){entries.begin() + 1, entries.end()};
				x.insert(nc, entries_1);
				x.insert(nc, entries_2_10);
			}

			SUBCASE("insert 5 + 5") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries = [&]() {
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
					for (auto &&entry : gen.entries(10))
						entries.emplace_back(entry.first, entry.second);
					return entries;
				}();
				auto entries_1_5 = decltype(entries){entries.begin(), entries.begin() + 5};
				auto entries_6_10 = decltype(entries){entries.begin() + 5, entries.end()};
				x.insert(nc, entries_1_5);
				x.insert(nc, entries_6_10);
			}
		}


		DOCTEST_TEST_CASE_TEMPLATE("allocate node", T,
								   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
								   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
								   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
								   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>) {
			create<T::depth, typename T::tri>();
		}
	}
};// namespace hypertrie::tests::core::node