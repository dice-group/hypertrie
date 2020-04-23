#include <catch2/catch.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <set>
#include <unordered_set>

#include <Dice/hypertrie/boolhypertrie.hpp>
#include <Dice/hypertrie/internal/Join.hpp>

#include "utils/GenerateTriples.hpp"
#include "diagonal/DiagonalTestData.hpp"


namespace hypertrie::tests::hashjoin {
	namespace {
		using namespace fmt::literals;
		using namespace hypertrie;
		using BH = hypertrie::boolhypertrie<>::CompressedBoolHypertrie;
		using const_BH = hypertrie::boolhypertrie<>::const_CompressedBoolHypertrie;
		using Join = hypertrie::boolhypertrie<>::CompressedHashJoin;
		using Key = BH::Key;
		using SliceKey = BH::SliceKey;
		using pos_type = hypertrie::boolhypertrie<>::pos_type;
	}


	TEST_CASE("Compiles at least", "[HashJoin]"
	) {
		BH tensor_0{1};
		tensor_0.set({0}, true);
		tensor_0.set({1}, true);
		tensor_0.set({2}, true);

		BH tensor_1{2};
		tensor_1.set({2, 7}, true);
		tensor_1.set({3, 8}, true);
		tensor_1.set({4, 9}, true);
		BH tensor_2{1};
		tensor_2.set({6}, true);
		tensor_2.set({2}, true);


		std::vector<std::vector<pos_type>> op_sc{{1},
		                                         {1, 0},
		                                         {1}};
		std::vector<const_BH> operands = {tensor_0, tensor_1, tensor_2};
		Join x{operands, {{0}, {0}, {0}}};

		Join a = x;
	}


	std::size_t run = 0;

	TEST_CASE("test many combinations", "[HashJoin]") {

		std::size_t const max = 15;
		std::vector<std::pair<pos_type, std::vector<pos_type >>> op_poss_pool;
		// @TODO change so to control the slicing positions
		for (auto depth : ::iter::range(1, 4)) {

			std::vector<pos_type> positions{iter::range(depth).begin(), iter::range(depth).end()};
			for (const auto &op_poss : iter::powerset(positions)) {
				op_poss_pool.push_back({depth, {op_poss.begin(), op_poss.end()}});
			}
		}


		const std::size_t diagonal_count = 50;
		std::vector<DiagonalTestData<>> diagonal_pool;
		std::uniform_int_distribution<std::size_t> rand_op_poss{0, diagonal_count - 1};
		diagonal_pool.reserve(diagonal_count);
		for (auto rand_op_pos : utils::gen_random<std::size_t>(diagonal_count, 0, op_poss_pool.size() - 1)) {
			auto &[depth, op_poss] = op_poss_pool[rand_op_pos];
			diagonal_pool.emplace_back(DiagonalTestData<>{op_poss, std::size_t(depth) * max, depth, max});
		}

		for (auto n : ::iter::range(8)) {
			std::size_t op_number = std::uniform_int_distribution<std::size_t>{1, 5}(
					utils::defaultRandomNumberGenerator);

			std::vector<std::reference_wrapper<DiagonalTestData<>>> diagonals;
			for (auto d : utils::gen_random<std::size_t>(op_number, 0, diagonal_count - 1))
				diagonals.emplace_back(diagonal_pool[d]);

			std::vector<Join::poss_type> join_positions;
			std::vector<const_BH> operands;

			for (const DiagonalTestData<> &diagonal : diagonals) {
				join_positions.push_back(diagonal.join_positions);
				BH op{diagonal.depth};
				for (const auto &key : diagonal.entries)
					op.set(key, true);

				operands.push_back(std::move(op));
			}

			bool all_join_poss_empty = true;
			for (const DiagonalTestData<> &diagonal : diagonals) {
				if (not diagonal.join_positions.empty()) {
					all_join_poss_empty = false;
					break;
				}
			}

			if (all_join_poss_empty)
				continue;

			std::vector<std::string> position_strings;
			for (auto &poss: join_positions)
				position_strings.push_back(fmt::format("({})", fmt::join(poss.begin(), poss.end(), ", ")));


			std::vector<std::size_t> sizes;
			for (const DiagonalTestData<> &diagonal : diagonals)
				sizes.push_back(diagonal.depth);
			{
				SECTION("diagonal positions {}; operand depths {}; n {}"_format(
						fmt::join(position_strings.begin(), position_strings.end(), ", "),
						fmt::join(sizes.begin(), sizes.end(), ", "), n)) {
					Join join{operands, join_positions};

					UNSCOPED_INFO("run {}"_format(run++));
					//std::vector<const_BoolHypertrie>, key_part_type
					std::size_t result_count = 0;
					for (auto[next_ops, key_part] : join) {
						++result_count;
						auto next_op_iter = next_ops.begin();
						for (DiagonalTestData<> &diagonal_data : diagonals) {
							if (diagonal_data.depth == diagonal_data.join_positions.size())
								continue;
							if (diagonal_data.join_positions.size() == 0) {
								for (const auto &key : diagonal_data.entries)
									REQUIRE(next_op_iter->operator[](key) == true);
							} else if (diagonal_data.join_positions.size() == diagonal_data.depth) {
								REQUIRE(diagonal_data.expected_entries.count(key_part));
							} else {
								for (const auto &key : diagonal_data.expected_entries.at(key_part))
									REQUIRE(next_op_iter->operator[](key) == true);
							}
							++next_op_iter;
						}
					}

					DiagonalTestData<> &first_diag = diagonals[0];
					tsl::sparse_set<std::size_t> actual_key_parts;
					if (first_diag.join_positions.empty()) {
                        std::vector<std::size_t> temp = {iter::range(max +1).begin(), iter::range(max +1).end()};
                        for (auto &x: temp) x *= 8;
						actual_key_parts = {temp.begin(), temp.end()};
					} else {
						for (auto &[key_part, _] : first_diag.expected_entries)
							actual_key_parts.insert(key_part);
					}


					for (DiagonalTestData<> &diagonal_data : hypertrie::internal::util::skip<1>(diagonals)) {
						tsl::sparse_set<std::size_t> current_key_parts;
						if (diagonal_data.join_positions.empty()) {
                            std::vector<std::size_t> temp = {iter::range(max +1).begin(), iter::range(max +1).end()};
                            for (auto &x: temp) x *= 8;
							current_key_parts = {temp.begin(), temp.end()}; // TODO: continue
						} else {
							for (auto &[key_part, _] : diagonal_data.expected_entries)
								current_key_parts.insert(key_part);
						}
						tsl::sparse_set<std::size_t> next_actual_key_parts;
						for (auto key_part : actual_key_parts)
							if (current_key_parts.count(key_part))
								next_actual_key_parts.insert(key_part);
						actual_key_parts = next_actual_key_parts;
					}


					REQUIRE(result_count == actual_key_parts.size());
					SUCCEED("Everything is OK");
				}
			}
		}
	}


}