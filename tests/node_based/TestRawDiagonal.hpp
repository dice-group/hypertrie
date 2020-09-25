#ifndef HYPERTRIE_TESTRAWDIAGONAL_HPP
#define HYPERTRIE_TESTRAWDIAGONAL_HPP

#include <Dice/hypertrie/internal/Hypertrie_traits.hpp>
#include <Dice/hypertrie/internal/raw/iterator/Diagonal.hpp>
#include <Dice/hypertrie/internal/raw/storage/NodeContext.hpp>
#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/DiagonalTestDataGenerator.hpp"
#include "../utils/NameOfType.hpp"
#include "TestTensor.hpp"
namespace hypertrie::tests::raw::node_context::diagonal_test {

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::raw;

	using namespace hypertrie::internal;

	// TODO: that will be helpful for non-raw diagonals
	std::vector<std::vector<size_t>> getAllSubsets(size_t depth) {
		std::vector<std::vector<size_t>> subsets;
		for (auto subset_r : iter::powerset(iter::range(depth))) {
			std::vector<size_t> subset{subset_r.begin(), subset_r.end()};
			if (subset.size() > 0)
				subsets.push_back(std::move(subset));
		}
		return subsets;
	}


	template<HypertrieInternalTrait tri, size_t depth>
	void randomized_diagonal_test() {
		using tr = typename tri::tr;
		using IteratorEntry = typename tr::IteratorEntry;
		using iter_funcs = typename tr::iterator_entry;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::template RawKey<depth>;

		static utils::DiagonalTestDataGenerator<depth, key_part_type, value_type, size_t(tri::is_lsb_unused)> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nodec{};

		for (size_t count : iter::range(0, 30)) {
			static auto all_diagonal_positions = getAllSubsets(depth);
			for (const auto &diagonal_positions : all_diagonal_positions) {
				// TODO: this approach works only for non-raw diagonals -> port it there
				SECTION("diagonal positions: {}"_format(fmt::join(diagonal_positions, ","))) {
					for (auto diagonal_size : iter::chain(iter::range(0, 5), iter::range(50, 51))) {
						SECTION("diagonal entries: {}"_format(fmt::join(diagonal_positions, ","))) {
							for (auto total_size :
								 iter::imap([&](double x) { return size_t(diagonal_size * x); },
											{1.0, 1.2, 2.0, 5.0})) {
								SECTION("entries: {}"_format(fmt::join(total_size, ","))) {
									auto diag_data = gen.diag_data(total_size, diagonal_size, diagonal_positions);


									for (const auto &[key, value] : diag_data.tensor_entries)
										context.template set<depth>(nodec, key, value);

									std::set<key_part_type> found_key_parts{};
									for (auto iter = HashDiagonal<depth, tri>(nodec, context); iter != false; ++iter) {
										IteratorEntry entry = *iter;
										auto actual_key = iter_funcs::key(entry);
										auto actual_rawkey = [&]() {
											Key raw_key;
											for (auto [raw_key_part, non_raw_key_part] : iter::zip(raw_key, actual_key))
												raw_key_part = non_raw_key_part;
											return raw_key;
										}();
										auto actual_value = iter_funcs::value(entry);
										// check if the key is valid
										REQUIRE(entries.count(actual_rawkey));
										// check if the value is valid
										REQUIRE(entries[actual_rawkey] == actual_value);
										// check that the entry was not already found
										REQUIRE(not found_keys.count(actual_rawkey));
										found_keys.insert(actual_rawkey);

										WARN("[{}] -> {}\n"_format(fmt::join(actual_key, ", "), actual_value));
									}
									REQUIRE(found_keys.size() == entries.size());
								}
							}
						}
					}
				}
			}
		}
	}
}
}
}

TEMPLATE_TEST_CASE_SIG("iterating hypertrie entries [bool]", "[RawDiagonal]", ((size_t depth), depth), 1, 2) {
	randomized_iterator_test<default_bool_Hypertrie_internal_t, depth>();
}

TEST_CASE("bool depth 1 uncompressed d", "[raw diagonal]") {
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


		typename tri::DiagonalPositions<depth> diag_poss {true};

		auto diag = hypertrie::internal::raw::HashDiagonal<1,depth, NodeCompression::uncompressed, tri>(nc, diag_poss, context);

		for(auto res : diag){
			key_part_type k =  res.first;
			fmt::print("{}\n", k);
		}
	}

	TEST_CASE("bool depth 2 uncompressed d", "[raw diagonal]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr const size_t depth = 2;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nc{};

		const auto entries = gen.entries(2);

		context.template set<depth>(nc, {1,1}, true);
		context.template set<depth>(nc, {1,2}, true);
		context.template set<depth>(nc, {2,1}, true);
		context.template set<depth>(nc, {2,2}, true);


		typename tri::DiagonalPositions<depth> diag_poss {};
		diag_poss[0] = true;
		diag_poss[1] = false;

		std::cout << "0:" << diag_poss[0] << std::endl;
		std::cout << "1:" << diag_poss[1] << std::endl;

		auto diag = hypertrie::internal::raw::HashDiagonal<1,depth, NodeCompression::uncompressed, tri>(nc, diag_poss, context);

		for(auto res : diag){
			key_part_type k =  res.first;
			fmt::print("{}\n", k);
		}
	}

	TEST_CASE("bool depth 2 compressed d", "[raw diagonal]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr const size_t depth = 2;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		NodeContainer<depth, tri> nc{};

		const auto entries = gen.entries(2);

		context.template set<depth>(nc, {1,1}, true);

		typename tri::DiagonalPositions<depth> diag_poss {};
		diag_poss[0] = true;
		diag_poss[1] = false;

		std::cout << "0:" << diag_poss[0] << std::endl;
		std::cout << "1:" << diag_poss[1] << std::endl;

		auto c_nc = nc.compressed();
		auto diag = hypertrie::internal::raw::HashDiagonal<1,depth, NodeCompression::compressed, tri>(c_nc, diag_poss);

		for(auto res : diag){
			key_part_type k =  res.first;
			fmt::print("{}\n", k);
		}
	}
};

#endif//HYPERTRIE_TESTRAWDIAGONAL_HPP
