#ifndef HYPERTRIE_TESTDATA_EINSUM_HPP
#define HYPERTRIE_TESTDATA_EINSUM_HPP

#include "Operand.hpp"

#include <dice/einsum.hpp>

#include <utility>

namespace dice::einsum::tests::test_data {

	template<utils::TorchDtype dtype_t, ::dice::hypertrie::HypertrieTrait operand_htt_t, ::dice::hypertrie::ByteAllocator allocator_type>
	class Einsum {
	public:
		using dtype = dtype_t;
		using Operand_t = Operand<dtype, operand_htt_t, allocator_type>;

	private:
		std::vector<std::reference_wrapper<Operand_t>> operands;
		std::shared_ptr<Subscript> subscript_;

	public:
		Einsum(std::shared_ptr<Subscript> test_subscript, std::vector<Operand_t> &test_operands)
			: subscript_(std::move(test_subscript)) {
			for (auto &test_operand : test_operands)
				operands.emplace_back(test_operand);
		}


		/**
		 * Generates a random TestEinsum over the given test_operand_candidates
		 * @param test_operand_candidates
		 */
		explicit Einsum(std::vector<Operand_t> &test_operand_candidates, int max_operand_count = 5) {
			using namespace ::dice::einsum::internal;
			using namespace ::dice::hypertrie::tests::utils;
			static std::mt19937_64 rand{42};
			// number of operands the subscript will have
			auto operand_count_gen = uniform_dist<uint8_t>(1, max_operand_count);
			uint8_t operands_count = operand_count_gen(rand);


			auto operand_selector = uniform_dist<size_t>(0, test_operand_candidates.size() - 1);

			size_t depth_sum = 0;
			for (auto _ = 0; _ < operands_count; ++_) {
				auto pos = operand_selector(rand);
				depth_sum += test_operand_candidates[pos].depth();
				operands.emplace_back(test_operand_candidates[pos]);
			}

			std::set<Label> used_labels;

			Label total_labels = (depth_sum <= std::numeric_limits<Label>::max()) ? Label(depth_sum) : std::numeric_limits<Label>::max();
			auto label_selector = std::binomial_distribution<Label>(total_labels);

			OperandsSc operands_sc;
			for (Operand_t const &operand : operands) {
				OperandSc operand_sc(operand.depth());
				for (auto &label : operand_sc) {
					label = label_selector(rand) + 'a';
					used_labels.insert(label);
				}
				operands_sc.push_back(std::move(operand_sc));
			}

			auto max_result_depth = used_labels.size() / 2;
			max_result_depth = (max_result_depth > 0) ? max_result_depth : 1;

			uint8_t result_depth = uniform_dist<uint8_t>(1, max_result_depth)(rand);
			ResultSc result_sc;
			while (result_sc.size() < result_depth) {
				auto pos = uniform_dist<ssize_t>(0, result_depth - 1)(rand);
				Label label = *std::next(used_labels.begin(), pos);
				if (std::find(result_sc.begin(), result_sc.end(), label) == result_sc.end())
					result_sc.emplace_back(label);
			}
			subscript_ = std::make_shared<Subscript>(operands_sc, result_sc);
			assert(subscript_->valid());
		}

		[[nodiscard]] std::vector<torch::Tensor> torchOperands() const {
			std::vector<torch::Tensor> ops{};
			for (Operand_t &operand : operands) {
				ops.emplace_back(operand.torch_tensor());
			}
			return ops;
		}

		std::vector<hypertrie::const_Hypertrie<operand_htt_t, allocator_type>> hypertrieOperands() const {
			std::vector<hypertrie::const_Hypertrie<operand_htt_t, allocator_type>> ops{};
			for (Operand_t &operand : operands) {
				ops.emplace_back(operand.hypertrie());
			}
			return ops;
		}

		[[nodiscard]] const std::shared_ptr<Subscript> &subscript() const {
			return subscript_;
		}
		[[nodiscard]] std::string subscript_str() const {
			return subscript_->to_string();
		}
	};
}// namespace dice::einsum::tests::test_data

#endif//HYPERTRIE_TESTDATA_EINSUM_HPP
