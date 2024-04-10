#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <dice/einsum.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>

#include "einsum2map.hpp"
#include "test_data/Einsum.hpp"
#include "test_data/Operand.hpp"
#include <utils/Product.hpp>

#include <fmt/format.h>


namespace dice::einsum::tests {

	using namespace fmt::literals;

	TEST_SUITE("Testing Einsum") {
		// TODO: make max_hypertrie_depth configurable (and configure it to 5 or 6 for this test)

		using allocator_type = std::allocator<std::byte>;

		using namespace ::dice::hypertrie;

		using namespace std::literals::chrono_literals;
		using time_point = std::chrono::steady_clock::time_point;

		template<utils::TorchDtype value_type, HypertrieTrait htt_t, ByteAllocator allocator_type, typename HypertrieEinsumResultType>
		void validateResult(const long max_key_part, const test_data::Einsum<value_type, htt_t, allocator_type> &test_einsum, HypertrieEinsumResultType actual_result, torch::Tensor &expected_result) {
			unsigned long result_depth = test_einsum.subscript()->resultLabelCount();
			//	std::cout << "result_empty: " << (actual_result.size() == 0) << std::endl;
			//	std::cout << "expected_result: " << expected_result << std::endl;
			//	std::cout << "actual_result: ";
			//	for (auto const &entry : actual_result)
			//		std::cout << entry.first << "->" << entry.second << ",";
			//	std::cout << std::endl;
			for (const auto &key_parts : utils::product<size_t>(result_depth, max_key_part + 1, 1)) {
				using ResultKey = ::dice::einsum::Key<value_type, htt_t>;
				ResultKey key{key_parts.begin(), key_parts.end()};
				auto actual_entry = (actual_result.count(key)) ? actual_result[key] : 0;
				dice::einsum::tests::utils::TorchDtype auto expected_entry = value_type(utils::TorchTensorAccessor<value_type, htt_t>::get(expected_result, key_parts));// to bool
				WARN_MESSAGE(actual_entry == expected_entry,
							 ("key: ({})\n"_format(fmt::join(key, ", ")) +
							  "expected: {}, actual {}"_format(expected_entry, actual_entry))
									 .c_str());
			}
		}


		template<HypertrieTrait htt_t, ByteAllocator allocator_type, typename value_type>
		void runTest(long max_key_part, test_data::Einsum<value_type, htt_t, allocator_type> &test_einsum, std::chrono::milliseconds timeout_duration = 0ms) {
			// result how it is
			auto start_time = std::chrono::steady_clock::now();
			auto timeout = (timeout_duration != 0ms) ? start_time + timeout_duration : time_point::max();
			auto actual_result = einsum2map<value_type, htt_t>(test_einsum.subscript(), test_einsum.hypertrieOperands(), timeout);
			std::string actual_result_str = [&]() {
				std::vector<std::string> elements;
				for (auto &[key, value] : actual_result)
					elements.push_back(fmt::format("⟨{}⟩ → {}", fmt::join(key, ", "), value));
				return fmt::format("[ {} ]", fmt::join(elements, ", "));
			}();
			auto end_time = std::chrono::steady_clock::now();
			if (timeout_duration != 0ms)
				CHECK((end_time - start_time) < (timeout_duration + 10ms));


			// expected result
			start_time = std::chrono::steady_clock::now();
			torch::Tensor expected_result = at::einsum(test_einsum.subscript_str(), test_einsum.torchOperands());
			end_time = std::chrono::steady_clock::now();
			if (timeout_duration == 0ms) {
				validateResult<value_type, htt_t>(max_key_part, test_einsum, actual_result, expected_result);
			}
		}

		template<HypertrieTrait htt_t, ByteAllocator allocator_type, typename T>
		void runTest(long max_key_part, std::vector<test_data::Operand<T, htt_t, allocator_type>> &operands, const std::shared_ptr<Subscript> &subscript,
					 std::chrono::milliseconds timeout_duration = 0ms) {
			test_data::Einsum<T, htt_t, allocator_type> test_einsum{subscript, operands};
			runTest<htt_t, allocator_type, T>(max_key_part, test_einsum, timeout_duration);
		}


		template<HypertrieTrait htt_t, ByteAllocator allocator_type, typename result_type>
		void runSubscript(std::string const &subscript_string, int64_t max_key_part = 4, bool empty = false, std::size_t runs = 15,
						  std::chrono::milliseconds timeout_duration = 0ms) {
			static std::string result_type_str = std::is_same_v<result_type, bool> ? "bool" : "ulong";
			SUBCASE("{} [res:{}]"_format(subscript_string, result_type_str).c_str()) {
				for (std::size_t run : iter::range(runs)) {
					SUBCASE("run {}"_format(run).c_str()) {
						auto subscript = std::make_shared<Subscript>(subscript_string);
						std::vector<test_data::Operand<result_type, htt_t, allocator_type>> operands{};
						for (const auto &operand_sc : subscript->getRawSubscript().operands) {
							[[maybe_unused]] test_data::Operand<result_type, htt_t, allocator_type> &operand = operands.emplace_back(uint8_t(operand_sc.size()), &DefaultHypertrieContext<htt_t, allocator_type>::instance(), max_key_part, empty);
							//							std::cout << operand.torch_tensor() << std::endl;
							//							std::cout << std::string(operand.hypertrie()) << std::endl;
						}
						runTest<htt_t, allocator_type, result_type>(max_key_part, operands, subscript, timeout_duration);
					}
				}
			}
		}

		template<utils::TorchDtype result_type, HypertrieTrait htt_t, ByteAllocator allocator_type>
		void run_single_cases(
				const std::string &subscript_str,
				const std::vector<std::set<NonZeroEntry<htt_t>>> &operands_entries) {

			auto subscript = std::make_shared<Subscript>(Subscript::from_string(subscript_str));

			int64_t max_key_part = [&]() {
				auto max = 0;
				for (const auto &entries : operands_entries)
					for (const auto &entry : entries)
						for (const auto &key_part : entry.key())
							max = std::max<long>(max, key_part);
				return max;
			}();


			std::vector<test_data::Operand<result_type, htt_t, allocator_type>> test_operands{};

			for (auto [op_pos, op_entries] : iter::enumerate(operands_entries)) {
				const uint8_t op_dims = subscript->getOperandLabels(op_pos).size();
				const bool empty = true;
				test_data::Operand<result_type, htt_t, allocator_type> operand{op_dims, &DefaultHypertrieContext<htt_t, allocator_type>::instance(), max_key_part, empty};

				for (const auto &entry : op_entries)
					operand.set(entry);
				test_operands.push_back(std::move(operand));
			}

			test_data::Einsum<result_type, htt_t, allocator_type> test_einsum(subscript, test_operands);

			auto actual_result = einsum2map<result_type, htt_t>(test_einsum.subscript(), test_einsum.hypertrieOperands());
			std::string actual_result_str = [&]() {
				std::vector<std::string> elements;
				for (auto &[key, value] : actual_result)
					elements.push_back(fmt::format("⟨{}⟩ → {}", fmt::join(key, ", "), value));
				return fmt::format("[ {} ]", fmt::join(elements, ", "));
			}();

			torch::Tensor expected_result = at::einsum(test_einsum.subscript_str(), test_einsum.torchOperands());


			validateResult<result_type, htt_t>(max_key_part, test_einsum, actual_result, expected_result);
		}


		TEST_CASE_TEMPLATE("single, once-problematic bool test cases for einsum", htt_t, ::dice::hypertrie::tagged_bool_Hypertrie_trait) {
			using result_type = bool;
			using namespace std::string_literals;
			using Entry = NonZeroEntry<htt_t>;
			std::vector<std::tuple<std::string, std::vector<std::set<Entry>>>> configurations{
					{std::string{"ab,bc,bc,de,ee,ef,gh,hg->adg"},
					 {{Entry{{3, 1}, true},
					   Entry{{3, 3}, true},
					   Entry{{2, 3}, true},
					   Entry{{4, 4}, true}},
					  {Entry{{3, 1}, true},
					   Entry{{3, 2}, true},
					   Entry{{1, 2}, true},
					   Entry{{4, 4}, true}},
					  {Entry{{2, 1}, true},
					   Entry{{1, 2}, true},
					   Entry{{4, 3}, true},
					   Entry{{2, 4}, true}},
					  {Entry{{1, 2}, true},
					   Entry{{4, 3}, true},
					   Entry{{2, 3}, true},
					   Entry{{4, 4}, true}},
					  {Entry{{2, 1}, true},
					   Entry{{1, 2}, true},
					   Entry{{3, 3}, true},
					   Entry{{1, 3}, true}},
					  {Entry{{2, 2}, true},
					   Entry{{3, 3}, true},
					   Entry{{2, 3}, true},
					   Entry{{1, 4}, true}},
					  {Entry{{4, 2}, true},
					   Entry{{4, 3}, true},
					   Entry{{2, 3}, true},
					   Entry{{1, 4}, true}},
					  {Entry{{3, 1}, true},
					   Entry{{3, 2}, true},
					   Entry{{1, 3}, true},
					   Entry{{4, 4}, true}}}},
					{std::string{"abcd,ceffb,cfgaf,hbgi,ccfaj->j"},
					 {{Entry{{1, 1, 1, 1}, true},
					   Entry{{1, 1, 2, 1}, true},
					   Entry{{2, 1, 1, 2}, true},
					   Entry{{1, 2, 2, 2}, true}},
					  {Entry{{2, 2, 1, 2, 1}, true},
					   Entry{{1, 2, 2, 2, 1}, true},
					   Entry{{2, 1, 1, 1, 2}, true},
					   Entry{{1, 1, 2, 1, 2}, true},
					   Entry{{2, 1, 2, 1, 2}, true}},
					  {Entry{{1, 2, 1, 1, 1}, true},
					   Entry{{2, 2, 1, 1, 2}, true},
					   Entry{{1, 2, 2, 1, 2}, true},
					   Entry{{1, 1, 1, 2, 2}, true},
					   Entry{{2, 2, 2, 2, 2}, true}},
					  {Entry{{2, 1, 1, 2}, true},
					   Entry{{1, 2, 1, 2}, true},
					   Entry{{1, 1, 2, 2}, true},
					   Entry{{2, 2, 2, 2}, true}},
					  {Entry{{2, 2, 2, 1, 1}, true},
					   Entry{{2, 1, 1, 2, 1}, true},
					   Entry{{1, 2, 2, 2, 1}, true},
					   Entry{{1, 1, 2, 1, 2}, true},
					   Entry{{2, 2, 2, 2, 2}, true}}}}};
			auto i = 0;
			for (const auto &[subscript_str, operands_entries] : configurations) {
				SUBCASE("case {}: {}"_format(i++, subscript_str).c_str()) {
					run_single_cases<result_type, htt_t, allocator_type>(subscript_str, operands_entries);
				}
			}
		}


		TEST_CASE_TEMPLATE("single, once-problematic test cases for einsum", htt_t, ::dice::hypertrie::default_bool_Hypertrie_trait) {
			using result_type = long;
			using namespace std::string_literals;
			using Entry = NonZeroEntry<htt_t>;
			std::vector<std::tuple<std::string, std::vector<std::set<Entry>>>> configurations{
					{std::string{"abc,dcebf,gdghg,bdg,ijibg->c"},
					 {{Entry{{2, 1, 1}, true},
					   Entry{{2, 1, 2}, true},
					   Entry{{2, 2, 2}, true}},
					  {Entry{{2, 1, 1, 1, 1}, true},
					   Entry{{2, 2, 1, 2, 1}, true},
					   Entry{{1, 2, 2, 2, 1}, true},
					   Entry{{1, 1, 1, 1, 2}, true},
					   Entry{{2, 1, 2, 2, 2}, true}},
					  {Entry{{1, 2, 1, 2, 1}, true},
					   Entry{{1, 1, 1, 1, 2}, true},
					   Entry{{2, 1, 1, 1, 2}, true},
					   Entry{{1, 2, 2, 2, 2}, true},
					   Entry{{2, 2, 2, 2, 2}, true}},
					  {Entry{{1, 1, 1}, true},
					   Entry{{1, 2, 1}, true},
					   Entry{{2, 2, 2}, true}},
					  {Entry{{1, 1, 1, 1, 1}, true},
					   Entry{{1, 2, 1, 1, 1}, true},
					   Entry{{2, 1, 2, 1, 1}, true},
					   Entry{{1, 2, 2, 1, 2}, true},
					   Entry{{2, 2, 2, 2, 2}, true}}}},
					{std::string{"abc,dcebf,gdghg,ijibg->c"},
					 {{Entry{{2, 1, 1}, true},
					   Entry{{1, 1, 2}, true},
					   Entry{{2, 2, 2}, true}},
					  {Entry{{1, 1, 2, 1, 1}, true},
					   Entry{{1, 2, 1, 1, 2}, true},
					   Entry{{1, 2, 2, 1, 2}, true},
					   Entry{{2, 1, 1, 2, 2}, true},
					   Entry{{1, 1, 2, 2, 2}, true}},
					  {Entry{{2, 1, 2, 1, 1}, true},
					   Entry{{1, 1, 1, 2, 1}, true},
					   Entry{{2, 1, 1, 2, 1}, true},
					   Entry{{2, 1, 1, 1, 2}, true},
					   Entry{{2, 2, 1, 1, 2}, true}},
					  {Entry{{2, 1, 2, 1, 1}, true},
					   Entry{{2, 1, 1, 2, 1}, true},
					   Entry{{1, 2, 1, 1, 2}, true},
					   Entry{{2, 2, 1, 1, 2}, true},
					   Entry{{1, 2, 1, 2, 2}, true}}}},
					{std::string{"ab,bc,bc,de,ee,ef,gh,hg->adg"},
					 {{Entry{{1, 1}, true},
					   Entry{{2, 1}, true}},
					  {Entry{{1, 1}, true},
					   Entry{{1, 2}, true}},
					  {Entry{{1, 1}, true},
					   Entry{{2, 1}, true}},
					  {Entry{{1, 2}, true},
					   Entry{{2, 2}, true}},
					  {Entry{{1, 1}, true},
					   Entry{{2, 2}, true}},
					  {Entry{{2, 1}, true},
					   Entry{{1, 2}, true}},
					  {Entry{{1, 2}, true},
					   Entry{{2, 2}, true}},
					  {Entry{{2, 1}, true},
					   Entry{{1, 2}, true}}}},
					{std::string{"ac,cb->b"},
					 {{Entry{{1, 1}, true},
					   Entry{{2, 1}, true}},
					  {Entry{{1, 1}, true},
					   Entry{{1, 2}, true}}}},
					{std::string{"caa->ca"},
					 {{Entry{{1, 1, 1}, true},
					   Entry{{2, 1, 1}, true},
					   Entry{{2, 2, 2}, true}}}},
					{std::string{"caa->ca"},
					 {{Entry{{3, 2, 2}, true},
					   Entry{{3, 1, 3}, true},
					   Entry{{2, 1, 1}, true},
					   Entry{{0, 3, 1}, true},
					   Entry{{1, 3, 1}, true},
					   Entry{{1, 1, 1}, true}}}},
					{std::string{"abc,ab->a"},
					 {{Entry{{1, 2, 1}, true},
					   Entry{{2, 1, 2}, true},
					   Entry{{1, 2, 2}, true}},
					  {Entry{{2, 1}, true},
					   Entry{{1, 2}, true}}}},
					{std::string{"cab,cc->a"},
					 {{Entry{{1, 1, 1}, true},
					   Entry{{1, 1, 2}, true},
					   Entry{{1, 2, 2}, true}},
					  {Entry{{1, 1}, true},
					   Entry{{1, 2}, true}}}},
					{std::string{"ab,bc,bc,de,ee,ef,gh,hg->adg"},
					 {{Entry{{1, 1}, true},
					   Entry{{2, 2}, true}},
					  {Entry{{1, 1}, true},
					   Entry{{1, 2}, true}},
					  {Entry{{2, 1}, true},
					   Entry{{2, 2}, true}},
					  {Entry{{1, 2}, true},
					   Entry{{2, 2}, true}},
					  {Entry{{1, 2}, true},
					   Entry{{2, 2}, true}},
					  {Entry{{1, 2}, true},
					   Entry{{2, 2}, true}},
					  {Entry{{2, 1}, true},
					   Entry{{2, 2}, true}},
					  {Entry{{1, 1}, true},
					   Entry{{2, 2}, true}}}}};
			auto i = 0;
			for (const auto &[subscript_str, operands_entries] : configurations) {
				SUBCASE("case {}: {}"_format(i++, subscript_str).c_str()) {
					run_single_cases<result_type, htt_t, allocator_type>(subscript_str, operands_entries);
				}
			}
		}

		TEST_CASE_TEMPLATE("default test cases", htt_t, ::dice::hypertrie::default_bool_Hypertrie_trait, ::dice::hypertrie::tagged_bool_Hypertrie_trait) {
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
					"aa->a",
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
					"aa,ae,ac,ad,a,ab->ab",
					/// Cartesian test cases
					"a,b->ab",
					"a,b->ba",
					"a,b->a",
					"a,b->b",
					"a,b,c->abc",
					"a,b,c->ab",
					"a,b,c->bc",
					"a,b,c->ca",
					"a,b,c->a",
					"a,b,c->b",
					"a,b,c->c",
					"ab,bc,d,e->abc",
					"d,ab,bc,e->abc",
					"d,e,ab,bc->abc",
					"ab,bc,bc,de,ee,ef,gh,hg->adg",
					/// compex test cases:
					"abc,dcebf,gdghg,bdg,ijibg->c",// is calculated faster
					"abc,dcebf,gdghg,ijibg->c",    // its minimal
					"abcd,ceffb,cfgaf,hbgi,ccfaj->j",
					"abbc,d,ebcfg,hdif,hhchj->b"};
			for (bool empty : {false, true}) {
				SUBCASE("empty = {}"_format(empty).c_str()) {
					for (auto max_key_part : {2, 4, 7, 10, 15}) {
						SUBCASE("max_key_part = {}"_format(max_key_part).c_str()) {
							for (const auto &subscript_str : subscript_strs) {
								runSubscript<htt_t, allocator_type, ssize_t>(subscript_str, max_key_part, empty);
								runSubscript<htt_t, allocator_type, bool>(subscript_str, max_key_part, empty);
							}
						}
					}
				}
			}
		}
	}

};// namespace dice::einsum::tests