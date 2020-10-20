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

	template<typename value_type, typename tr, typename HypertrieEinsumResultType>
	void validateResult(const long excl_max, const TestEinsum<tr> &test_einsum, HypertrieEinsumResultType actual_result, torch::Tensor &expected_result) {
		unsigned long result_depth = test_einsum.subscript->resultLabelCount();
		for (const auto &key : product<std::size_t>(result_depth, excl_max)) {
			auto actual_entry = [&] {
				if constexpr (tr::is_bool_valued and tr::lsb_unused) {
					auto shifted_key = key;
					for (auto &key_part : shifted_key)
						key_part <<= 2;
					return (actual_result.count(shifted_key)) ? actual_result[shifted_key] : 0;
				} else
					return (actual_result.count(key)) ? actual_result[key] : 0;
			}();
			auto expected_entry = value_type(TorchHelper<long>::resolve(expected_result, key));// to bool
			INFO("key: ({})"_format(fmt::join(key, ", ")));
			INFO("expected: {}, actual {}"_format(TorchHelper<long>::resolve(expected_result, key), actual_entry))
			REQUIRE(actual_entry == expected_entry);
		}
	}

	template<HypertrieTrait tr, typename value_type>
	void runTest(long excl_max, TestEinsum<tr> &test_einsum, std::chrono::milliseconds timeout_duration = 0ms) {
		auto einsum = &hypertrie::einsum2map<value_type, tr>;
		// result how it is
		auto start_time = std::chrono::steady_clock::now();
		auto timeout = (timeout_duration != 0ms) ? start_time + timeout_duration : time_point::max();
		auto actual_result = einsum(test_einsum.subscript, test_einsum.hypertrieOperands(), timeout);
		std::string actual_result_str = [&]() {
			std::vector<std::string> elements;
			for(auto &[key, value] : actual_result)
				elements.push_back(fmt::format("⟨{}⟩ → {}", fmt::join(key, ", "), value));
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
			validateResult<value_type, tr>(excl_max, test_einsum, actual_result, expected_result);
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

	template<HypertrieTrait tr>
	void run_single_cases(
			const std::string subscript_str,
			const std::vector<std::set<std::pair<typename tr::Key, typename tr::value_type>>> &operands_entries) {
		using value_type = typename tr::value_type;
		auto einsum = &hypertrie::einsum2map<value_type, tr>;


		auto subscript = std::make_shared<Subscript>(Subscript::from_string(subscript_str));

		size_t excl_max = [&]() {
			auto max = 0;
			for (const auto &entries : operands_entries)
				for (const auto &[key, _] : entries)
					for (const auto &key_part : key)
						max = std::max<long>(max, key_part);
			return max + 1;
		}();


		std::vector<TestOperand<tr>> test_operands{};

		for (auto [op_pos, op_entries] : iter::enumerate(operands_entries)) {
			const uint8_t op_dims = subscript->getOperandLabels(op_pos).size();
			const bool emtpy = true;
			TestOperand<tr> operand{op_dims, excl_max, emtpy};

			for (const auto &[key, value] : op_entries) {
				operand.set(key, value);
			}
			WARN(fmt::format("operand {}:\n{}", op_pos, (std::string) operand.hypertrie));
			test_operands.push_back(std::move(operand));
		}

		TestEinsum<tr> test_einsum(subscript, test_operands);

		auto actual_result = einsum(test_einsum.subscript, test_einsum.hypertrieOperands(), TimePoint::max());
		std::string actual_result_str = [&]() {
			std::vector<std::string> elements;
			for (auto &[key, value] : actual_result)
				elements.push_back(fmt::format("⟨{}⟩ → {}", fmt::join(key, ", "), value));
			return fmt::format("[ {} ]", fmt::join(elements, ", "));
		}();
		WARN(fmt::format("actual result:\n{}", actual_result_str));

		torch::Tensor expected_result = at::einsum(test_einsum.str_subscript, test_einsum.torchOperands());


		validateResult<value_type, tr>(excl_max, test_einsum, actual_result, expected_result);
	}

	TEMPLATE_TEST_CASE("Problematic Cases", "[einsum]", lsbunused_bool_Hypertrie_t, default_bool_Hypertrie_t) {
		using tr = TestType;
		using namespace std::string_literals;
		using Key = typename tr::Key;
		using value_type = typename tr::value_type;
		using Entry = std::pair<Key, value_type>;

		std::vector<std::tuple<std::string, std::vector<std::set<Entry>>>> configurations{
				{std::string{"caa->ca"},
				 {
						 {
								 {{3, 2, 2}, true},
								 {{3, 1, 3}, true},
								 {{2, 1, 1}, true},
								 {{0, 3, 1}, true},
								 {{1, 3, 1}, true},
								 {{1, 1, 1}, true}// op0}
						 }                        //op0
				 }},                              // caa->ca
				{
						std::string{"caa->ca"},
						{
								{{{2, 2, 2}, true},
								 {{2, 2, 3}, true},
								 {{2, 3, 3}, true},
								 {{0, 1, 1}, true},
								 {{3, 2, 2}, true},
								 {{3, 2, 0}, true}}// op0
						}                          //op0
				},                                 // caa->ca
		};
		auto i = 0;
		for (const auto &[subscript_str, operands_entries] : configurations) {
			SECTION("case {}: {}"_format(i++, subscript_str)) {
				run_single_cases<tr>(subscript_str, operands_entries);
			}
		}
	}

	TEMPLATE_TEST_CASE("default test cases", "[einsum]", lsbunused_bool_Hypertrie_t, default_bool_Hypertrie_t) {
		using tr = TestType;
		std::vector<std::string> subscript_strs{
				"a->a",
				"ab->a",
				"ab->b",
				"ab->ab",
				"ab->ba",
				"caa->ca",
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
				"aa,ae,ac,ad,a,ab->ab"};
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

	TEMPLATE_TEST_CASE("complex test cases", "[einsum]", lsbunused_bool_Hypertrie_t, default_bool_Hypertrie_t) {
		using tr = TestType;
		std::vector<std::string> subscript_strs{
				"abc,dcebf,gdghg,bdg,ijibg->c",// is calculated faster
				"abc,dcebf,gdghg,ijibg->c",    // its minimal
				"abcd,ceffb,cfgaf,hbgi,ccfaj->j",
				"abbc,d,ebcfg,hdif,hhchj->b"

		};
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
