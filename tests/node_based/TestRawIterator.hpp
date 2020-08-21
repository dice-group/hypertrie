#ifndef HYPERTRIE_TESTRAWITERATOR_HPP
#define HYPERTRIE_TESTRAWITERATOR_HPP


#include <Dice/hypertrie/internal/node_based/raw/storage/NodeContext.hpp>
#include <Dice/hypertrie/internal/node_based/interface/Hypertrie_traits.hpp>
#include <Dice/hypertrie/internal/node_based/raw/iterator/Iterator.hpp>
#include <Dice/hypertrie/internal/node_based/raw/iterator/Diagonal.hpp>
#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"
#include "TestTensor.hpp"


namespace hypertrie::tests::node_based::raw::node_context::iterator_test {

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::node_based::raw;

	using namespace hypertrie::internal::node_based;


	TEST_CASE("bool depth 1 uncompressed", "[raw iterator]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr const size_t depth = 1;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nc{};

		const auto entries = gen.entries(2);

		context.template set<depth>(nc, {1}, true);
		context.template set<depth>(nc, {2}, true);


		for(auto iter = iterator<depth, tri>(nc, context); iter != false; ++iter){
			auto key = *iter;
			fmt::print("[{}] -> {}\n", key[0], true);
//			fmt::print("[{} {} {}] -> {}", key[0], key[1], key[2], true);
		}
	}

	TEST_CASE("bool depth 1 compressed", "[raw iterator]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr const size_t depth = 1;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nc{};

		const auto entries = gen.entries(2);

		context.template set<depth>(nc, {5}, true);


		for(auto iter = iterator<depth, tri>(nc, context); iter != false; ++iter){
			auto key = *iter;
			fmt::print("[{}] -> {}\n", key[0], true);
//			fmt::print("[{} {} {}] -> {}", key[0], key[1], key[2], true);
		}
	}

	TEST_CASE("bool depth 2 uncompressed", "[raw iterator]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr const size_t depth = 2;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nc{};

		context.template set<depth>(nc, {1,3}, true);
		context.template set<depth>(nc, {1,4}, true);


		for(auto iter = iterator<depth, tri>(nc, context); iter != false; ++iter){
			auto key = *iter;
			fmt::print("[{} {} ] -> {}\n", key[0], key[1],true);
		}
	}

	TEST_CASE("long depth 2 uncompressed", "[raw iterator]") {
		using tri = default_long_Hypertrie_internal_t;
		constexpr const size_t depth = 2;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nc{};

		context.template set<depth>(nc, {2,3}, 7);
		context.template set<depth>(nc, {1,4}, 8);


		for(auto iter = iterator<depth, tri>(nc, context); iter != false; ++iter){
			auto key = iter->first;
			auto value = iter->second;
			fmt::print("[{} {} ] -> {}\n", key[0], key[1],value);
		}
	}

	TEST_CASE("bool depth 2 compressed", "[raw iterator]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr const size_t depth = 2;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nc{};

		const auto entries = gen.entries(2);

		context.template set<depth>(nc, {5,7}, true);


		for(auto iter = iterator<depth, tri>(nc, context); iter != false; ++iter){
			auto key = *iter;
			fmt::print("[{} {}] -> {}\n", key[0], key[1], true);
//			fmt::print("[{} {} {}] -> {}", key[0], key[1], key[2], true);
		}
	}

	TEST_CASE("long depth 2 compressed", "[raw iterator]") {
		using tri = default_long_Hypertrie_internal_t;
		constexpr const size_t depth = 2;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nc{};

		const auto entries = gen.entries(2);

		context.template set<depth>(nc, {5,7}, 3);


		for(auto iter = iterator<depth, tri>(nc, context); iter != false; ++iter){
			auto key = iter->first;
			auto value = iter->second;
			fmt::print("[{} {}] -> {}\n", key[0], key[1], value);
		}
	}


	template<HypertrieInternalTrait tri, size_t depth>
	void randomized_raw_iterator_test() {
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::Key;

		static utils::RawGenerator<depth, key_part_type, value_type> gen{};

		SECTION(fmt::format("depth = {}, key_part_type = {}, value_type = {}",
							depth, nameOfType<key_part_type>(), nameOfType<value_type>())) {

			for ([[maybe_unused]]auto number_of_entries : iter::range(0,500)) {
				SECTION(fmt::format("number of entries = {}", number_of_entries)) {

					for ([[maybe_unused]]auto run : iter::range(0,30)) {
						// create context
						NodeContext<depth, tri> context{};
						// create emtpy primary node
						UncompressedNodeContainer<depth, tri> nc{};

						const auto entries = gen.entries(number_of_entries);


						for (const auto &entry : entries)
							context.template set<depth>(nc, entry.first, entry.second);

						using IteratorEntry = std::conditional_t<(tri::is_bool_valued), Key, std::pair<Key, value_type>>;

						std::vector<IteratorEntry> iterator_entries;

						for (auto iter = iterator<depth, tri>(nc, context); iter != false; ++iter) {
							iterator_entries.push_back(*iter);
						}

						REQUIRE(entries.size() == iterator_entries.size());

						for (auto entry : entries) {
							IteratorEntry actual_entry = [&]() -> IteratorEntry {
								if constexpr (tri::is_bool_valued) return {entry.first.begin(), entry.first.end()};
								else
									return {Key{entry.first.begin(), entry.first.end()}, entry.second};
							}();
							REQUIRE(std::find(iterator_entries.begin(), iterator_entries.end(), actual_entry) != iterator_entries.end());
						}
					}
				}
			}
		}
	}

	template <size_t depth>
	void randomized_raw_iterator_tests() {
		randomized_raw_iterator_test<default_bool_Hypertrie_internal_t, depth>();
		randomized_raw_iterator_test<default_long_Hypertrie_internal_t, depth>();
		randomized_raw_iterator_test<default_double_Hypertrie_internal_t, depth>();
	}


	TEST_CASE("randomized tests", "[raw iterator]") {
		randomized_raw_iterator_tests<1>();
		randomized_raw_iterator_tests<2>();
		randomized_raw_iterator_tests<3>();
		randomized_raw_iterator_tests<4>();
	}
}

#endif//HYPERTRIE_TESTRAWITERATOR_HPP
