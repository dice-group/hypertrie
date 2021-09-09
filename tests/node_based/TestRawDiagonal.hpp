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
		using IteratorEntry [[maybe_unused]] = typename tr::IteratorEntry;
		using iter_funcs [[maybe_unused]] = typename tr::iterator_entry;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key [[maybe_unused]] = typename tri::template RawKey<depth>;

		static utils::DiagonalTestDataGenerator<diag_depth, depth, key_part_type, value_type, size_t(tri::is_lsb_unused)> gen{};

		NodeContext<depth, tri> context{};
		NodeContainer<depth, tri> nodec{};

		static auto all_diagonal_positions = getAllCombinations(depth, diag_depth);
		for (const auto &diagonal_positions : all_diagonal_positions) {
			SECTION("diagonal positions: {}"_format(fmt::join(diagonal_positions, ","))) {
				for (size_t diagonal_size : iter::chain(iter::range(2, 5), iter::range(50, 51))) {
					SECTION("diagonal size: {}"_format(diagonal_size)) {
						for (size_t total_size :
							 iter::unique_everseen(iter::imap([&](double x) { return size_t(diagonal_size * x); },
															  std::vector<double>{1.0, 1.2, 2.0, 5.0}))) {
							SECTION("tensor size: {}"_format(total_size)) {
								if (depth == 1 and diag_depth == 1 and total_size != diagonal_size)
									continue;

								for (auto i : iter::range(50)) {
									SECTION("{}"_format(i)) {
										auto diag_data = gen.diag_data(total_size, diagonal_size, diagonal_positions);


										std::string print_entries{};
										for (const auto &[key, value] : diag_data.tensor_entries)
											print_entries += "{} → {}\n"_format(key, value);
										WARN("entries:\n" + print_entries);

										auto raw_diag_poss = tri::template rawDiagonalPositions<depth>(diagonal_positions);

										for (const auto &[raw_key, value] : diag_data.tensor_entries)
											context.template set<depth>(nodec, raw_key, value);

										std::set<key_part_type> found_key_parts{};

										auto unodec = nodec.uncompressed();

										::hypertrie::internal::raw::HashDiagonal<diag_depth, depth, NodeCompression::uncompressed, tri> diag(unodec, raw_diag_poss, context);

										INFO((std::string) context.storage);

										for (auto iter = diag.begin(); iter != false; ++iter) {
											auto actual_key_part = iter.currentKeyPart();

											// check if the key_part is valid
											REQUIRE(diag_data.diagonal_entries.count(actual_key_part));

											// check that the key_part was not already found
											REQUIRE(not found_key_parts.count(actual_key_part));
											found_key_parts.insert(actual_key_part);

											INFO("diagonal key part: {}\n"_format(actual_key_part));

											if constexpr (depth != diag_depth) {
												auto actual_iter_entry = iter.currentValue();

												auto expected_entries = diag_data.diagonal_entries[actual_key_part];
												for (auto &[raw_key, expected_value] : expected_entries) {
													auto actual_value = context.template get(actual_iter_entry.nodec, raw_key);
													REQUIRE(actual_value == expected_value);
												}

												size_t size = context.template size(actual_iter_entry.nodec);
												REQUIRE(size == expected_entries.size());

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
		}
	}

	template<HypertrieInternalTrait tri, size_t depth, size_t diag_depth>
	void randomized_diagonal_compressed_test() {
		using tr = typename tri::tr;
		using IteratorEntry [[maybe_unused]] = typename tr::IteratorEntry;
		using iter_funcs [[maybe_unused]] = typename tr::iterator_entry;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key [[maybe_unused]] = typename tri::template RawKey<depth>;

		static utils::DiagonalTestDataGenerator<diag_depth, depth, key_part_type, value_type, size_t(tri::is_lsb_unused)> gen{};

		NodeContext<depth, tri> context{};
		NodeContainer<depth, tri> nodec{};

		static auto all_diagonal_positions = getAllCombinations(depth, diag_depth);
		for (const auto &diagonal_positions : all_diagonal_positions) {
			SECTION("diagonal positions: {}"_format(fmt::join(diagonal_positions, ","))) {
				for (auto diagonal_size : iter::range(0,2)) {
					if (diag_depth == 1 and diagonal_size == 0)
						continue;
					SECTION("diagonal size: {}"_format(diagonal_size)) {
						size_t total_size = 1;
						for (auto i : iter::range(50)) {
							SECTION("{}"_format(i)) {
								auto diag_data = gen.diag_data(total_size, diagonal_size, diagonal_positions);


								std::string print_entries{};
								for (const auto &[key, value] : diag_data.tensor_entries)
									print_entries += "{} → {}\n"_format(key, value);
								WARN("entries:\n" + print_entries);

								auto raw_diag_poss = tri::template rawDiagonalPositions<depth>(diagonal_positions);

								for (const auto &[raw_key, value] : diag_data.tensor_entries)
									context.template set<depth>(nodec, raw_key, value);

								std::set<key_part_type> found_key_parts{};

								auto cnodec = nodec.compressed();

								::hypertrie::internal::raw::HashDiagonal<diag_depth, depth, NodeCompression::compressed, tri> diag(cnodec, raw_diag_poss);

								INFO((std::string) context.storage);

								for (auto iter = diag.begin(); iter != false; ++iter) {
									auto actual_key_part = iter.currentKeyPart();

									// check if the key_part is valid
									REQUIRE(diag_data.diagonal_entries.count(actual_key_part));

									// check that the key_part was not already found
									REQUIRE(not found_key_parts.count(actual_key_part));
									found_key_parts.insert(actual_key_part);

									INFO("diagonal key part: {}\n"_format(actual_key_part));

									if constexpr (depth != diag_depth) {
										auto actual_iter_entry = iter.currentValue();

										auto expected_entries = diag_data.diagonal_entries[actual_key_part];
										for (auto &[raw_key, expected_value] : expected_entries) {
											auto actual_value = context.template get(actual_iter_entry.nodec, raw_key);
											REQUIRE(actual_value == expected_value);
										}

										size_t size = context.template size(actual_iter_entry.nodec);
										REQUIRE(size == expected_entries.size());

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

	TEMPLATE_TEST_CASE_SIG("Depth 1 diagonals [bool]", "[RawDiagonal]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_diagonal_compressed_test<default_bool_Hypertrie_internal_t, depth, 1>();
		randomized_diagonal_test<default_bool_Hypertrie_internal_t, depth, 1>();
	}

	TEMPLATE_TEST_CASE_SIG("Depth 1 diagonals [bool lsb-unused]", "[RawDiagonal]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_diagonal_compressed_test<lsbunused_bool_Hypertrie_internal_t, depth, 1>();
		randomized_diagonal_test<lsbunused_bool_Hypertrie_internal_t, depth, 1>();
	}

	TEMPLATE_TEST_CASE_SIG("Depth 2 diagonals [bool]", "[RawDiagonal]", ((size_t depth), depth), 2, 3, 4, 5) {
		randomized_diagonal_compressed_test<default_bool_Hypertrie_internal_t, depth, 2>();
		randomized_diagonal_test<default_bool_Hypertrie_internal_t, depth, 2>();
	}

	TEMPLATE_TEST_CASE_SIG("Depth 2 diagonals [bool lsb-unused]", "[RawDiagonal]", ((size_t depth), depth), 2, 3, 4, 5) {
		randomized_diagonal_compressed_test<lsbunused_bool_Hypertrie_internal_t, depth, 2>();
		randomized_diagonal_test<lsbunused_bool_Hypertrie_internal_t, depth, 2>();
	}

	TEMPLATE_TEST_CASE_SIG("Depth 3 diagonals [bool]", "[RawDiagonal]", ((size_t depth), depth), 3, 4, 5) {
		randomized_diagonal_compressed_test<default_bool_Hypertrie_internal_t, depth, 3>();
		randomized_diagonal_test<default_bool_Hypertrie_internal_t, depth, 3>();
	}

	TEMPLATE_TEST_CASE_SIG("Depth 3 diagonals [bool lsb-unused]", "[RawDiagonal]", ((size_t depth), depth), 3, 4, 5) {
		randomized_diagonal_compressed_test<lsbunused_bool_Hypertrie_internal_t, depth, 3>();
		randomized_diagonal_test<lsbunused_bool_Hypertrie_internal_t, depth, 3>();
	}

	TEMPLATE_TEST_CASE_SIG("Depth 4 diagonals [bool]", "[RawDiagonal]", ((size_t depth), depth), 4, 5) {
		randomized_diagonal_compressed_test<default_bool_Hypertrie_internal_t, depth, 4>();
		randomized_diagonal_test<default_bool_Hypertrie_internal_t, depth, 4>();
	}

	TEMPLATE_TEST_CASE_SIG("Depth 4 diagonals [bool lsb-unused]", "[RawDiagonal]", ((size_t depth), depth), 4, 5) {
		randomized_diagonal_compressed_test<lsbunused_bool_Hypertrie_internal_t, depth, 4>();
		randomized_diagonal_test<lsbunused_bool_Hypertrie_internal_t, depth, 4>();
	}

	// TODO: test long valued diagonals

};

#endif//HYPERTRIE_TESTRAWDIAGONAL_HPP
