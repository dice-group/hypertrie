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

	std::vector<std::vector<uint8_t>> getAllCombinations(size_t depth, size_t comb_length) {
		std::vector<std::vector<uint8_t>> subsets;
		for (auto subset_r : iter::combinations(iter::range(uint8_t(depth)), comb_length)) {
			std::vector<uint8_t> subset{subset_r.begin(), subset_r.end()};
			if (subset.size() > 0)
				subsets.push_back(std::move(subset));
		}
		return subsets;
	}


	template<HypertrieInternalTrait tri, size_t depth, size_t diag_depth>
	void randomized_diagonal_test() {
		using tr = typename tri::tr;
		using IteratorEntry = typename tr::IteratorEntry;
		using iter_funcs = typename tr::iterator_entry;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::template RawKey<depth>;

		static utils::DiagonalTestDataGenerator<diag_depth, depth, key_part_type, value_type, size_t(tri::is_lsb_unused)> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nodec{};

		static auto all_diagonal_positions = getAllCombinations(depth, diag_depth);
		for (const auto &diagonal_positions : all_diagonal_positions) {
			SECTION("diagonal positions: {}"_format(fmt::join(diagonal_positions, ","))) {
				for (auto diagonal_size : iter::chain(iter::range(2, 5), iter::range(50, 51))) {
					SECTION("diagonal entries: {}"_format(fmt::join(diagonal_positions, ","))) {
						for (auto total_size :
							 iter::imap([&](double x) { return size_t(diagonal_size * x); },
										std::vector<double>{1.0, 1.2, 2.0, 5.0})) {
							SECTION("entries: {}"_format(total_size)) {
								auto diag_data = gen.diag_data(total_size, diagonal_size, diagonal_positions);


								std::string print_entries{};
								for (const auto &[key, value] : diag_data.tensor_entries)
									print_entries += "{} → {}\n"_format(key, value);
								WARN("entries:\n"+print_entries);

								auto raw_diag_poss = tri::template rawDiagonalPositions<depth>(diagonal_positions);

								for (const auto &[raw_key, value] : diag_data.tensor_entries)
									context.template set<depth>(nodec, raw_key, value);

								std::set<key_part_type> found_key_parts{};

								HashDiagonal<diag_depth, depth, NodeCompression::uncompressed, tri> diag(nodec, raw_diag_poss, context) ;

								for (auto iter = diag.begin(); iter != false; ++iter) {
									auto actual_key_part = iter.currentKeyPart();

									// check if the key_part is valid
									REQUIRE(diag_data.diagonal_entries.count(actual_key_part));

									// check that the key_part was not already found
									REQUIRE(not found_key_parts.count(actual_key_part));
									found_key_parts.insert(actual_key_part);

									WARN("diagonal key part: {}\n"_format(actual_key_part));

									if constexpr (depth != diag_depth) {
										auto actual_iter_entry = iter.currentValue();

										auto expected_entries = diag_data.diagonal_entries[actual_key_part];
										for (auto &[raw_key, expected_value] : expected_entries) {
											auto actual_value = context.template get(actual_iter_entry.nodec, raw_key);
											REQUIRE(actual_value == expected_value);
										}

										REQUIRE(context.template size(actual_iter_entry.nodec) == expected_entries.size());

										REQUIRE(actual_iter_entry.is_managed == true);
									}
								}
								REQUIRE(found_key_parts.size() == diagonal_size);
							}
						}
					}
				}
			}
		}
	}

	TEMPLATE_TEST_CASE_SIG("iterating hypertrie entries [bool]", "[RawDiagonal]", ((size_t depth), depth), 3) {
		randomized_diagonal_test<default_bool_Hypertrie_internal_t, depth, 2>();
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
