#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <dice/einsum.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>

#include "einsum2map.hpp"
#include "test_data/Einsum.hpp"
#include "test_data/Operand.hpp"
#include <utils/Product.hpp>

#include <fmt/format.h>

#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

//#include <metall/metall.hpp>
#include "../metall_mute_warnings.hpp"


namespace dice::einsum::tests {

	using namespace fmt::literals;

	TEST_SUITE("Testing Einsum") {
		// TODO: make max_hypertrie_depth configurable (and configure it to 5 or 6 for this test)

		using allocator_type = metall::manager::allocator_type<std::byte>;

		using namespace ::dice::hypertrie;

		using namespace std::literals::chrono_literals;
		using time_point = std::chrono::steady_clock::time_point;

		template<utils::TorchDtype value_type, HypertrieTrait htt_t, ByteAllocator allocator_type, typename HypertrieEinsumResultType>
		void validateResult(const long max_key_part, const test_data::Einsum<value_type, htt_t, allocator_type> &test_einsum, HypertrieEinsumResultType actual_result, torch::Tensor &expected_result) {
			unsigned long result_depth = test_einsum.subscript()->resultLabelCount();
			for (const auto &key_parts : utils::product<size_t>(result_depth, max_key_part + 1, 1)) {
				using ResultKey = ::dice::einsum::Key<value_type, htt_t>;
				ResultKey key{key_parts.begin(), key_parts.end()};
				auto actual_entry = (actual_result.count(key)) ? actual_result[key] : 0;
				std::cout << "result_empty: " << (actual_result.size() == 0) << std::endl;
				dice::einsum::tests::utils::TorchDtype auto expected_entry = value_type(utils::TorchTensorAccessor<value_type, htt_t>::get(expected_result, key_parts));// to bool
				WARN_MESSAGE(actual_entry == expected_entry,
							 ("key: ({})\n"_format(fmt::join(key, ", ")) +
							  "expected: {}, actual {}"_format(expected_entry, actual_entry))
									 .c_str());
			}
		}


		template<HypertrieTrait htt_t, ByteAllocator allocator_type, typename value_type>
		void runTest(long max_key_part, test_data::Einsum<value_type, htt_t, allocator_type> &test_einsum) {
			// result how it is
			// TODO: actual_result is already problematic
			auto actual_result = einsum2map<value_type, htt_t, allocator_type>(test_einsum.subscript(), test_einsum.hypertrieOperands());
			std::string actual_result_str = [&]() {
				std::vector<std::string> elements;
				for (auto &[key, value] : actual_result)
					elements.push_back(fmt::format("⟨{}⟩ → {}", fmt::join(key, ", "), value));
				return fmt::format("[ {} ]", fmt::join(elements, ", "));
			}();
			// expected result
			torch::Tensor expected_result = at::einsum(test_einsum.subscript_str(), test_einsum.torchOperands());
			validateResult<value_type, htt_t>(max_key_part, test_einsum, actual_result, expected_result);
		}

		template<HypertrieTrait htt_t, ByteAllocator allocator_type, typename T>
		void runTest(long max_key_part, std::vector<test_data::Operand<T, htt_t, allocator_type>> &operands, const std::shared_ptr<Subscript> &subscript) {
			test_data::Einsum<T, htt_t, allocator_type> test_einsum{subscript, operands};
			runTest<htt_t, allocator_type, T>(max_key_part, test_einsum);
		}


		template<HypertrieTrait htt_t, ByteAllocator allocator_type, typename result_type>
		void runSubscript(std::string const &subscript_string, int64_t max_key_part = 4, bool empty = false, std::size_t runs = 15) {
			static std::string result_type_str = std::is_same_v<result_type, bool> ? "bool" : "ulong";
			SUBCASE("{} [res:{}]"_format(subscript_string, result_type_str).c_str()) {
				for (std::size_t run : iter::range(runs)) {
					SUBCASE("run {}"_format(run).c_str()) {
						const std::string path = fmt::format("/tmp/{}", std::random_device()()); // important for parallel execution of different tests
						constexpr auto context = "context";
						//create segment
						{
							metall::manager::remove(path.c_str());
							metall::manager manager(metall::create_only, path.c_str());
						}

						try {// write into manager
							metall::manager manager(metall::open_only, path.c_str());
							// create context
							auto ctx_ptr = manager.construct<HypertrieContext<htt_t, allocator_type>>(context)(manager.get_allocator());
							{
								auto subscript = std::make_shared<Subscript>(subscript_string);
								// operands must not outlive metall manager. The instances of hypertrie (held in the operands) will clean up their data in ctx_ptr on destruction which is availle through the metall manager.
								std::vector<test_data::Operand<result_type, htt_t, allocator_type>> operands{};
								for (const auto &operand_sc : subscript->getRawSubscript().operands) {
									operands.emplace_back(uint8_t(operand_sc.size()), ctx_ptr, max_key_part, empty);
								}
								runTest<htt_t, allocator_type, result_type>(max_key_part, operands, subscript);
							}
						} catch (...) {}

						metall::manager::remove(path.c_str());
					}
				}
			}
		}


		TEST_CASE_TEMPLATE("default test cases", htt_t, default_bool_Hypertrie_trait /*, ::dice::hypertrie::tagged_bool_Hypertrie_trait*/) {
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
					/// complex test cases:
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