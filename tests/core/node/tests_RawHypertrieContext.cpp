#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include "ValidationRawNodeContext.hpp"
#include <AssetGenerator.hpp>
#include <Dice/hypertrie/internal/util/name_of_type.hpp>
#include <Node_test_configs.hpp>


#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>


namespace hypertrie::tests::core::node {

	TEST_SUITE("NodeStorage") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		using tri = typename tagged_bool_cfg<3>::tri;
		constexpr size_t depth = tagged_bool_cfg<3>::depth;

		std::vector<SingleEntry<3, tri_with_stl_alloc<tri>>> _2M_triples = []() {
			hypertrie::tests::utils::RawGenerator<depth, tri> gen{1, 158};
			std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
			for (auto &&entry : gen.entries(2000000))
				entries.emplace_back(entry.first, entry.second);
			return entries;
		}();

		auto _1M_triples_a = decltype(_2M_triples){_2M_triples.begin(), _2M_triples.begin() + 2000000 / 2};
		auto _1M_triples_b = decltype(_2M_triples){_2M_triples.begin() + 2000000 / 2, _2M_triples.end()};

		TEST_CASE("storage") {
			using tri = typename tagged_bool_cfg<3>::tri;
			constexpr size_t depth = tagged_bool_cfg<3>::depth;
			hypertrie::tests::utils::RawGenerator<depth, tri> gen{1, 3};

			RawHypertrieContext<depth, tri> context{std::allocator<std::byte>()};
			NodeContainer<depth, tri> nc{};
			SUBCASE("insert nothing") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
				context.insert(nc, entries);
				ValidationRawNodeContext<depth, tri> validation_context{std::allocator<std::byte>(), entries};
				REQUIRE(validation_context == context);
			}

			SUBCASE("insert one") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries = [&]() {
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
					for (auto &&entry : gen.entries(1))
						entries.emplace_back(entry.first, entry.second);
					return entries;
				}();
				context.insert(nc, entries);
				ValidationRawNodeContext<depth, tri> validation_context{std::allocator<std::byte>(), entries};
				REQUIRE(validation_context == context);
			}

			SUBCASE("insert two") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries = [&]() {
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
					for (auto &&entry : gen.entries(2))
						entries.emplace_back(entry.first, entry.second);
					return entries;
				}();
				context.insert(nc, entries);
				ValidationRawNodeContext<depth, tri> validation_context{std::allocator<std::byte>(), entries};
				REQUIRE(validation_context == context);
			}

			SUBCASE("insert 10") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries = [&]() {
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
					for (auto &&entry : gen.entries(10))
						entries.emplace_back(entry.first, entry.second);
					return entries;
				}();
				context.insert(nc, entries);
				ValidationRawNodeContext<depth, tri> validation_context{std::allocator<std::byte>(), entries};
				REQUIRE(validation_context == context);
				for (const auto &entry : entries)
					REQUIRE(context.get(nc, entry.key()) == entry.value());
			}

			SUBCASE("insert 1 + 9") {
				std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries = [&]() {
					std::vector<SingleEntry<depth, tri_with_stl_alloc<tri>>> entries;
					for (auto &&entry : gen.entries(2))
						entries.emplace_back(entry.first, entry.second);
					return entries;
				}();
				auto entries_1 = decltype(entries){entries[0]};
				auto entries_2_10 = decltype(entries){entries.begin() + 1, entries.end()};
				context.insert(nc, entries_1);
				ValidationRawNodeContext<depth, tri> validation_context{std::allocator<std::byte>(), entries_1};
				REQUIRE(validation_context == context);
				context.insert(nc, entries_2_10);
				ValidationRawNodeContext<depth, tri> validation_context2{std::allocator<std::byte>(), entries};
				REQUIRE(validation_context2 == context);
				for (const auto &entry : entries)
					REQUIRE(context.get(nc, entry.key()) == entry.value());
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
				context.insert(nc, entries_1_5);
				ValidationRawNodeContext<depth, tri> validation_context{std::allocator<std::byte>(), entries_1_5};
				REQUIRE(validation_context == context);
				context.insert(nc, entries_6_10);
				ValidationRawNodeContext<depth, tri> validation_context2{std::allocator<std::byte>(), entries};
				REQUIRE(validation_context2 == context);
				for (const auto &entry : entries)
					REQUIRE(context.get(nc, entry.key()) == entry.value());
			}
			//
			SUBCASE("insert 1M") {
				context.insert(nc, _1M_triples_a);
				ValidationRawNodeContext<depth, tri> validation_context{std::allocator<std::byte>(), _1M_triples_a};
				REQUIRE(validation_context == context);
				context.insert(nc, _1M_triples_b);
				ValidationRawNodeContext<depth, tri> validation_context2{std::allocator<std::byte>(), _2M_triples};
				REQUIRE(validation_context2 == context);
				for (const auto &entry : _2M_triples)
					REQUIRE(context.get(nc, entry.key()) == entry.value());
			}
		}
	}
}// namespace hypertrie::tests::core::node