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

#include "einsum/EinsumTestData.hpp"
#include "utils/GenerateTriples.hpp"
#include "utils/TorchHelper.hpp"
#include <chrono>


namespace hypertrie::tests::einsum {
	using namespace std::literals::chrono_literals;
	using time_point = std::chrono::steady_clock::time_point;

	template<HypertrieTrait tr, typename value_type>
	void runTest(long excl_max, TestEinsum<tr> &test_einsum, std::chrono::milliseconds timeout_duration = 0ms) {
		auto einsum = &hypertrie::einsum2map<value_type, tr>;
		// result how it is
		auto start_time = std::chrono::steady_clock::now();
		auto timeout = (timeout_duration != 0ms) ? start_time + timeout_duration : time_point::max();
		auto actual_result = einsum(test_einsum.subscript, test_einsum.hypertrieOperands(), timeout);
		std::string actual_result_str= [&](){
			std::vector<std::string> elements;
			for(auto &[key, value] : actual_result)
				elements.push_back(fmt::format("⟨{} → {}⟩", fmt::join(key, ", "), value));
			return fmt::format("[ {} ]", fmt::join(elements, ", "));
		}();
		WARN(actual_result_str);
		WARN("result entries: {}"_format(actual_result.size()));
		auto end_time = std::chrono::steady_clock::now();
		WARN(fmt::format("hypertrie: {}ms ",
						 std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()));
		if (timeout_duration != 0ms)
			REQUIRE((end_time - start_time) < (timeout_duration + 10ms));


		// expected result
		start_time = std::chrono::steady_clock::now();
		torch::Tensor expected_result = at::einsum(test_einsum.str_subscript, test_einsum.torchOperands());
		end_time = std::chrono::steady_clock::now();
		WARN(fmt::format("pytorch: {}ms ",
						 std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()));
		if (timeout_duration == 0ms) {
			unsigned long result_depth = test_einsum.subscript->resultLabelCount();
			for (const auto &key : product<std::size_t>(result_depth, excl_max)) {
				auto actual_entry = (actual_result.count(key)) ? actual_result[key] : 0;
				auto expected_entry = value_type(TorchHelper<long>::resolve(expected_result, key));// to bool
				INFO("key: ({})"_format(fmt::join(key, ", ")));
				INFO("expected: {}, actual {}"_format(TorchHelper<long>::resolve(expected_result, key), actual_entry));
				REQUIRE(actual_entry == expected_entry);
			}
		}
	}

	template<HypertrieTrait tr, typename T>
	void runTest(long excl_max, std::vector<TestOperand<tr>> &operands, const std::shared_ptr<Subscript> &subscript,
				 std::chrono::milliseconds timeout_duration = 0ms) {
		TestEinsum<tr> test_einsum{subscript, operands};
		runTest<tr, T>(excl_max, test_einsum, timeout_duration);
	}

	template<HypertrieTrait tr, typename result_type>
	void runSubscript(std::string subscript_string, long excl_max = 4, bool empty = false, std::size_t runs = 15,
					  std::chrono::milliseconds timeout_duration = 0ms) {
		static std::string result_type_str = std::is_same_v<result_type, bool> ? "bool" : "ulong";
		SECTION("{} [res:{}]"_format(subscript_string, result_type_str)) {
			for (std::size_t run : iter::range(runs))
				SECTION("run {}"_format(run)) {
					torch::manual_seed(std::hash<std::size_t>()(run));
					auto subscript = std::make_shared<Subscript>(subscript_string);
					std::vector<TestOperand<tr>> operands{};
					for (const auto &operand_sc : subscript->getRawSubscript().operands) {
						[[maybe_unused]] TestOperand<tr> &operand = operands.emplace_back(operand_sc.size(), excl_max, empty);
						//						WARN(operand.torch_tensor);
					}
					runTest<tr, result_type>(excl_max, operands, subscript, timeout_duration);
				}
		}
	}

	TEST_CASE("default test cases", "[einsum]") {

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
				"abc,ab->a",
				"abc,ab->b",
				"abc,ab->c",
				"abc,ab->ca",
				"aac,ab->ca",
				"aca,ab->ca",
				"caa,ab->ca",
				"caa,cc->ca",
				"cab,cc->ca",
				"a,b,c->abc",
				"a,b,c->ac",
				"a,b,c->ca",
				"a,b,c->a",
				"a,b,c->c",
				"ab,cd,aec->bde",
				"a,b,cd->d",
				"a,bbc,cdc,cf->f",
				"ab,bc,ca->abc",
				"ab,bc,ca,ax,xy,ya->a",
				"aa,ae,ac,ad,a,ab->abcde"};
		using tr = default_bool_Hypertrie_t;
		for (bool empty : {false, true}) {
			SECTION("empty = {}"_format(empty))
			for (auto excl_max : {4, 7, 10, 15, 30}) {
				SECTION("excl_max = {}"_format(excl_max))
				for (auto subscript_str : subscript_strs) {
					runSubscript<tr, std::size_t>(subscript_str, excl_max, empty);
					runSubscript<tr, bool>(subscript_str, excl_max, empty);
				}
			}
		}
	}

	TEST_CASE("complex test cases", "[einsum]") {

		std::vector<std::string> subscript_strs{
				"abc,dcebf,gdghg,bdg,ijibg->c",// is calculated faster
				"abc,dcebf,gdghg,ijibg->c",    // its minimal
				"abcd,ceffb,cfgaf,hbgi,ccfaj->j",
				"abbc,d,ebcfg,hdif,hhchj->b"

		};
		using tr = default_bool_Hypertrie_t;
		for (bool empty : {false, true}) {
			SECTION("empty = {}"_format(empty))
			for (auto excl_max : {4, 7, 10, 15, 30}) {
				SECTION("excl_max = {}"_format(excl_max))
				for (auto subscript_str : subscript_strs) {
					runSubscript<tr, std::size_t>(subscript_str, excl_max, empty);
					runSubscript<tr, bool>(subscript_str, excl_max, empty);
				}
			}
		}
	}

	// TODO: re-add randomly generated test-cases
}// namespace hypertrie::tests::einsum
