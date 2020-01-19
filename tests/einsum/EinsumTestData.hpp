#ifndef HYPERTRIE_EINSUMTESTDATA_HPP
#define HYPERTRIE_EINSUMTESTDATA_HPP

#include <fmt/core.h>
#include <fmt/format.h>
#include <set>
#include <unordered_set>
#include <iostream>

#include <Dice/hypertrie/boolhypertrie.hpp>

#include <torch/torch.h>

#include "../utils/GenerateTriples.hpp"
#include "../utils/Product.hpp"

namespace hypertrie::tests::einsum {

	namespace {

		using namespace ::hypertrie::tests::utils;
		using namespace ::einsum::internal;
		using bh_ns = ::hypertrie::boolhypertrie<unsigned long, ::hypertrie::internal::container::tsl_sparse_map ,
				::hypertrie::internal::container::tsl_sparse_set >;
		using namespace ::fmt::literals;
		using namespace ::hypertrie;
		using BH = bh_ns::BoolHypertrie;
		using const_BH = bh_ns::const_BoolHypertrie;
		using Join = bh_ns::HashJoin;
		using Key = BH::Key;
		using SliceKey = BH::SliceKey;
		using pos_type = bh_ns::pos_type;
	}

	int resolve(const torch::Tensor &tensor, std::vector<unsigned long> key) {
		torch::Tensor temp = tensor;
		for (const auto &key_part : key) {
			temp = temp[key_part];
		}
		return temp.item<int>();
	}

	struct TestOperand {
		torch::Tensor torch_tensor;
		BH hypertrie;
		long excl_max;
		uint8_t depth;

		TestOperand() = default;

		TestOperand(TestOperand &) = default;

		TestOperand(const TestOperand &) = default;

		TestOperand(TestOperand &&) = default;


		TestOperand(uint8_t depth, long excl_max = 10, bool empty = false) :
				torch_tensor(torch::randint(0, empty ? 1 : 2, std::vector<long>(depth, excl_max))),
				hypertrie{depth},
				excl_max(excl_max),
				depth(depth) {
			for (const auto &key : hypertrie::tests::utils::product<std::size_t>(depth, excl_max))
				if (resolve(torch_tensor, key)) hypertrie.set(key, true);
			if (empty)
				assert(hypertrie.size() == 0);
			assert(hypertrie.size() == std::size_t(torch_tensor.sum().item<long>()));

		}
	};

	struct TestEinsum {
		std::vector<std::reference_wrapper<TestOperand>> operands;
		std::shared_ptr<Subscript> subscript;
		std::string str_subscript;

		TestEinsum(const std::shared_ptr<Subscript> &test_subscript, std::vector<TestOperand> &test_operands)
				: subscript(test_subscript), str_subscript(subscript->to_string()) {
			for (auto &test_operand : test_operands)
				operands.emplace_back(test_operand);
		}


		/**
		 * Generates a random TestEinsum over the given test_operand_candidates
		 * @param test_operand_candidates
		 */
		TestEinsum(std::vector<TestOperand> &test_operand_candidates) {
			uint8_t operands_count = *gen_random<uint8_t>(1, 1, 5).begin();
			auto depth_sum = 0;
			for (auto &&i : gen_random<std::size_t>(operands_count, 0, test_operand_candidates.size() - 1)) {
				depth_sum += test_operand_candidates[i].depth;
				operands.emplace_back(test_operand_candidates[i]);
			}
			std::set<Label> used_labels;
			std::binomial_distribution<Label> d(depth_sum, 0.5);
			OperandsSc operands_sc;
			for (TestOperand &operand : operands) {
				OperandSc operand_sc(operand.depth);
				for (auto &label : operand_sc) {
					label = d(defaultRandomNumberGenerator) + 'a';
					used_labels.insert(label);
				}
				operands_sc.push_back(std::move(operand_sc));
			}

			auto max_result_depth = used_labels.size() / 2;
			max_result_depth = (max_result_depth > 0) ? max_result_depth : 1;
			uint8_t result_depth = *gen_random<uint8_t>(1, 1, max_result_depth).begin();
			ResultSc result_sc;
			result_sc.reserve(result_depth);
			for (auto pos : gen_random<std::set<Label>::iterator::difference_type>(result_depth, 0,
			                                                                       used_labels.size() - 1)) {
				auto it = used_labels.begin();
				std::advance(it, pos);
				Label label = *it;
				if (std::find(result_sc.begin(), result_sc.end(), label) == result_sc.end())
					result_sc.emplace_back(label);
			}
			subscript = std::make_shared<Subscript>(operands_sc, result_sc);
			assert(subscript->valid());
			str_subscript = subscript->to_string();
		}

		std::vector<torch::Tensor> torchOperands() const {
			std::vector<torch::Tensor> ops{};
			for (TestOperand &operand : operands) {
				ops.emplace_back(operand.torch_tensor);
			}
			return ops;
		}

		std::vector<const_BH> hypertrieOperands() const {
			std::vector<const_BH> ops{};
			for (TestOperand &operand : operands) {
				ops.emplace_back(operand.hypertrie);
			}
			return ops;
		}
	};

}
#endif //HYPERTRIE_EINSUMTESTDATA_HPP
