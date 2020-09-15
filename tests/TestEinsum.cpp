#include <catch2/catch.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <set>
#include <unordered_set>
#include <set>
#include <iostream>

#include <Dice/hypertrie/hypertrie.hpp>

#include <torch/torch.h>

#include "utils/GenerateTriples.hpp"
#include "einsum/EinsumTestData.hpp"
#include <chrono>


namespace hypertrie::tests::einsum {
	using namespace std::literals::chrono_literals;
	using time_point = std::chrono::steady_clock::time_point;

	template<typename T>
	void runTest(long excl_max, TestEinsum &test_einsum, std::chrono::milliseconds timeout_duration = 0ms) {
		auto einsum = &hypertrie::einsum2map<T,tr>;
		// result how it is
		auto start_time = std::chrono::steady_clock::now();
		auto timeout = (timeout_duration != 0ms) ? start_time + timeout_duration : time_point::max();
		auto actual_result = einsum(test_einsum.subscript, test_einsum.hypertrieOperands(), timeout);
		auto end_time = std::chrono::steady_clock::now();
		WARN(fmt::format("hypertrie: {}ms ",
		                 std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()));
		if (timeout_duration != 0ms)
			REQUIRE((end_time - start_time) < (timeout_duration + 10ms));



		// expected result
		start_time = std::chrono::steady_clock::now();
		torch::Tensor expected_result = torch::einsum(test_einsum.str_subscript, test_einsum.torchOperands());
		end_time = std::chrono::steady_clock::now();
		WARN(fmt::format("pytorch: {}ms ",
		                 std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()));
		if (timeout_duration == 0ms) {
			unsigned long result_depth = test_einsum.subscript->resultLabelCount();
			for (const auto &key : product<std::size_t>(result_depth, excl_max)) {
				auto actual_entry = (actual_result.count(key)) ? actual_result[key] : 0;
				auto expected_entry = T(resolve(expected_result, key));  // to bool
				INFO("key: ({})"_format(fmt::join(key, ", ")));
				INFO("expected: {}, actual {}"_format(resolve(expected_result, key), actual_entry));
				REQUIRE (actual_entry == expected_entry);
			}
		}
	}

	template<typename T>
	void runTest(long excl_max, std::vector<TestOperand> &operands, const std::shared_ptr<Subscript> &subscript,
	             std::chrono::milliseconds timeout_duration = 0ms) {
		TestEinsum test_einsum{subscript, operands};
		runTest<T>(excl_max, test_einsum, timeout_duration);
	}

	template<typename T>
	void runSubscript(std::string subscript_string, long excl_max = 4, bool empty = false, std::size_t runs = 15,
	                  std::chrono::milliseconds timeout_duration = 0ms) {
		static std::string result_type_str = std::is_same_v<T, bool> ? "bool" : "ulong";
		SECTION("{} [res:{}]"_format(subscript_string, result_type_str)) {
			for (std::size_t run : iter::range(runs))
				SECTION("run {}"_format(run)) {
					torch::manual_seed(std::hash<std::size_t>()(run));
					auto subscript = std::make_shared<Subscript>(subscript_string);
					std::vector<TestOperand> operands{};
					for (const auto &operand_sc : subscript->getRawSubscript().operands) {
						[[maybe_unused]] TestOperand &operand = operands.emplace_back(operand_sc.size(), excl_max, empty);
//						WARN(operand.torch_tensor);
					}
					runTest<T>(excl_max, operands, subscript, timeout_duration);

				}
		}
	}

	TEST_CASE("timeout", "[einsum]") {
		std::vector<std::string> subscript_strs{
				"abc,dcebf,gdghg,bdg,ijibg->c", // is calculated faster
				"abc,dcebf,gdghg,ijibg->c", // its minimal
				"abcd,ceffb,cfgaf,hbgi,ccfaj->j",
				"abbc,d,ebcfg,hdif,hhchj->b"
		};
		using namespace std::literals::chrono_literals;
		std::chrono::milliseconds timeout_duration;
		timeout_duration = 30ms;

		for (bool empty : {false, true})
			SECTION("empty = {}"_format(empty)) {
				for (auto excl_max : {10}) {
					SECTION("excl_max = {}"_format(excl_max)) {
						for (auto subscript_str : subscript_strs) {
							runSubscript<std::size_t>(subscript_str, excl_max, empty, 1, timeout_duration);

							runSubscript<bool>(subscript_str, excl_max, empty, 1, timeout_duration);
						}
					}
				}
			}

	}

	TEST_CASE("problematic test cases", "[einsum]") {

		std::vector<std::string> subscript_strs{
				"abc,dcebf,gdghg,bdg,ijibg->c", // is calculated faster
				"abc,dcebf,gdghg,ijibg->c", // its minimal
				"abcd,ceffb,cfgaf,hbgi,ccfaj->j",
				"abbc,d,ebcfg,hdif,hhchj->b"

		};
		for (bool empty : {false, true})
			SECTION("empty = {}"_format(empty)) {
				for (auto excl_max : {10}) {
					SECTION("excl_max = {}"_format(excl_max)) {
						for (auto subscript_str : subscript_strs) {
							runSubscript<std::size_t>(subscript_str, excl_max, empty, 1);
							runSubscript<bool>(subscript_str, excl_max, empty, 1);
						}
					}
				}
			}

	}


	TEST_CASE("run simple cases", "[einsum]") {

		std::vector<std::string> subscript_strs{
				"a->a",
				"ab->a",
				"ab->b",
				"ab->ab",
				"ab->ba",
				"a,a->a",
				"ab,a->a",
				"ab,a->b",
				"ab,a->ab",
				"ab,a->ba",
				"a,b->a",
				"a,b->b",
				"a,b->ab",
				"aa,bb->ab",
				"aa,bb->b",
				"aa,bb->a",
				"ac,cb->c",
				"ac,cb->b",
				"a,b,c->abc",
				"a,b,c->ac",
				"a,b,c->ca",
				"a,b,c->a",
				"a,b,c->c",
				"a,b,cd->d",
				"a,bbc,cdc,cf->f",
				"ab,bc,ca->abc",
				"ab,bc,ca,ax,xy,ya->a",
				"aa,ae,ac,ad,a,ab->abcde"

		};
		for (bool empty : {false, true}) {
			SECTION("empty = {}"_format(empty))for (auto excl_max : {4, 7, 10, 15}) {
					SECTION("excl_max = {}"_format(excl_max))for (auto subscript_str : subscript_strs) {
							runSubscript<std::size_t>(subscript_str, excl_max, empty);
							runSubscript<bool>(subscript_str, excl_max, empty);
						}
				}

		}

	}

	template<typename T>
	void generate_and_run_exec(long excl_max, TestEinsum &test_einsum) {
		SECTION("{}"_format(test_einsum.str_subscript)) {
			runTest<T>(excl_max, test_einsum);
		}
	}


	struct GenerateAndRunSetup {
		long excl_max;
		std::vector<TestOperand> test_operands;
		std::vector<TestEinsum> test_einsums;
	};


	TEST_CASE("generate and run", "[einsum]") {
		static GenerateAndRunSetup setup = []() -> GenerateAndRunSetup {
			torch::manual_seed(std::hash<std::size_t>()(42));
			long excl_max = 15;
			std::size_t test_operands_count = 50;
			std::vector<TestEinsum> test_einsums;

			std::vector<TestOperand> test_operands;
			test_operands.reserve(test_operands_count);
			// generate tensors
			for (auto depth: gen_random<uint8_t>(test_operands_count, 1, 4)) {
				TestOperand &operand = test_operands.emplace_back(depth, excl_max);
				WARN("Operand generated: depth = {}, dim_basis = [0,{}), nnz = {}"_format(operand.depth,
				                                                                          operand.excl_max,
				                                                                          operand.hypertrie.size()));
			}
			std::size_t test_einsums_count = 500;
			test_einsums.reserve(test_einsums_count);

			for ([[maybe_unused]]auto i : iter::range(test_einsums_count))
				test_einsums.emplace_back(test_operands, 8);

			return {excl_max, std::move(test_operands), std::move(test_einsums)};
		}();

		for (auto &test_einsum :setup.test_einsums) {
			SECTION("{}[res:ulong]"_format(test_einsum.str_subscript)) {
				runTest<std::size_t>(setup.excl_max, test_einsum);
			}SECTION("{}[res:bool]"_format(test_einsum.str_subscript)) {
				runTest<bool>(setup.excl_max, test_einsum);
			}
		}


	}
}
