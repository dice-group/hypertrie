#ifndef HYPERTRIE_EINSUMTESTDATA_HPP
#define HYPERTRIE_EINSUMTESTDATA_HPP

#include <ranges>
#include <fmt/core.h>
#include <fmt/format.h>
#include <set>
#include <unordered_set>
#include <iostream>

#include <Dice/hypertrie/hypertrie.hpp>
#include <Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp>
#include <Dice/einsum/internal/Subscript.hpp>


#include <torch/torch.h>

#include "../utils/GenerateTriples.hpp"
#include "../utils/AssetGenerator.hpp"
#include "../utils/Product.hpp"
#include "../utils/TorchHelper.hpp"

namespace hypertrie::tests::einsum {

	namespace {

		using namespace ::hypertrie::tests::utils;
		using namespace ::einsum::internal;

		using namespace ::fmt::literals;
		using namespace ::hypertrie;
		using Key = typename hypertrie::Key<size_t>;
		using SliceKey = typename hypertrie::SliceKey <size_t>;
		using pos_type = uint8_t;
	}

	template<HypertrieTrait tr>
	struct TestOperand {

		using tri = typename internal::raw::Hypertrie_internal_t<tr>;
		using dtype = typename tr::value_type;
		static_assert(std::is_same_v<dtype, bool> or
					  std::is_same_v<dtype, int64_t> or
					  std::is_same_v<dtype, float> or
					  std::is_same_v<dtype, double>);
		torch::Tensor torch_tensor;
		hypertrie::Hypertrie<tr> hypertrie;
		size_t excl_max;
		uint8_t depth;

		inline static utils::EntryGenerator<unsigned long, unsigned long, size_t(tri::is_lsb_unused)> gen{};

		TestOperand() = default;

		TestOperand(TestOperand &) = default;

		TestOperand(const TestOperand &) = default;

		TestOperand(TestOperand &&) = default;

		TestOperand(uint8_t depth, size_t excl_max = 10, bool empty = false) :
				torch_tensor(torch::zeros(std::vector<long>(depth, excl_max), torch::TensorOptions().dtype<long>())),
				hypertrie{depth},
				excl_max(excl_max),
				depth(depth) {

			if (not empty) {
				gen.setKeyPartMinMax(0, excl_max - 1);
				auto entries = gen.entries(excl_max * depth / 2, depth);
				// TODO: only supports boolean tensors so far
				BulkInserter<tr> bulk_inserter{hypertrie};
				for (auto [key, value] : entries){
					long &value_ref = TorchHelper<long>::resolve(torch_tensor, key);
					value_ref = long(dtype(value));
					auto key_cpy = key;
					bulk_inserter.add(std::move(key_cpy));
				}
			}
			if (empty)
				assert(hypertrie.size() == 0);
//			WARN(torch_tensor);
			WARN((std::string) hypertrie);
//			WARN((std::string) hypertrie.context()->raw_context.storage);
			assert(hypertrie.size() == std::size_t(torch_tensor.sum().item<long>()));

		}

		void set(const Key &key, const dtype &value){
			std::ranges::for_each(key, [&](const auto &i){assert(i < excl_max);});
			this->hypertrie.set(key, value);
			long &value_ref = TorchHelper<long>::resolve(this->torch_tensor, key);
			value_ref = long(value);
		}
	};

	template<HypertrieTrait tr>
	struct TestEinsum {
		std::vector<std::reference_wrapper<TestOperand<tr>>> operands;
		std::shared_ptr<Subscript> subscript;
		std::string str_subscript;

		TestEinsum(const std::shared_ptr<Subscript> &test_subscript, std::vector<TestOperand<tr>> &test_operands)
				: subscript(test_subscript), str_subscript(subscript->to_string()) {
			for (auto &test_operand : test_operands)
				operands.emplace_back(test_operand);
		}


		/**
		 * Generates a random TestEinsum over the given test_operand_candidates
		 * @param test_operand_candidates
		 */
		TestEinsum(std::vector<TestOperand<tr>> &test_operand_candidates, int max_operand_count= 5) {
			uint8_t operands_count = *gen_random<uint8_t>(1, 1, max_operand_count).begin();
			auto depth_sum = 0;
			for (auto &&i : gen_random<std::size_t>(operands_count, 0, test_operand_candidates.size() - 1)) {
				depth_sum += test_operand_candidates[i].depth;
				operands.emplace_back(test_operand_candidates[i]);
			}
			std::set<Label> used_labels;
			std::binomial_distribution<Label> d(depth_sum, 0.5);
			OperandsSc operands_sc;
			for (TestOperand<tr> &operand : operands) {
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
			for (TestOperand<tr> &operand : operands) {
				ops.emplace_back(operand.torch_tensor);
			}
			return ops;
		}

		std::vector<hypertrie::const_Hypertrie<tr>> hypertrieOperands() const {
			std::vector<hypertrie::const_Hypertrie<tr>> ops{};
			for (TestOperand<tr> &operand : operands) {
				ops.emplace_back(operand.hypertrie);
			}
			return ops;
		}
	};

}
#endif //HYPERTRIE_EINSUMTESTDATA_HPP
