#ifndef HYPERTRIE_TESTDATA_OPERAND_HPP
#define HYPERTRIE_TESTDATA_OPERAND_HPP

#include <dice/hypertrie.hpp>
#include <torch/torch.h>

#include <utils/AssetGenerator.hpp>

#include <utils/TorchTensorAccessor.hpp>


namespace dice::einsum::tests::test_data {

	using namespace dice::hypertrie;
	template<utils::TorchDtype dtype_t, ::dice::hypertrie::HypertrieTrait operand_htt_t, ::dice::hypertrie::ByteAllocator allocator_type>
	class Operand {
	public:
		using dtype = dtype_t;

	private:
		size_t depth_{};
		size_t max_key_part{};
		::torch::Tensor torch_tensor_;
		Hypertrie<operand_htt_t, allocator_type> hypertrie_;


		inline static ::dice::hypertrie::tests::utils::EntryGenerator<operand_htt_t> gen{};

	public:
		Operand() = default;


		explicit Operand(size_t depth, HypertrieContext_ptr<operand_htt_t, allocator_type> ctx, int64_t max_key_part = 10, bool empty = false)
			: depth_(depth),
			  max_key_part(max_key_part),
			  torch_tensor_(torch::zeros(std::vector<int64_t>(depth, max_key_part + 1), torch::TensorOptions().dtype<int64_t>())),
			  hypertrie_{depth, ctx}

		{

			if (not empty) {
				gen.setKeyPartMinMax(1, max_key_part);
				auto entries = gen.entries(max_key_part * depth_ / 2, depth_);
				for (const auto &[key, value] : entries)
					hypertrie_.set(key, value);
				for (const auto &[key, value] : entries)
					utils::TorchTensorAccessor<dtype, operand_htt_t>::set(torch_tensor_, NonZeroEntry<operand_htt_t>{key, value});
				auto torch_sum = size_t(torch_tensor_.sum().item<int64_t>());
				assert(hypertrie_.size() == torch_sum);
			}
			if (empty)
				assert(hypertrie_.empty());
		}

		void set(const NonZeroEntry<operand_htt_t> &entry) {
			assert(std::ranges::all_of(entry.key(), [&](auto i) { return i <= max_key_part; }));
			hypertrie_.set(entry.key(), entry.value());
			utils::TorchTensorAccessor<dtype, operand_htt_t>::set(torch_tensor_, entry);
		}

		[[nodiscard]] const torch::Tensor &torch_tensor() const {
			return torch_tensor_;
		}
		const Hypertrie<operand_htt_t, allocator_type> &hypertrie() const {
			return hypertrie_;
		}

		[[nodiscard]] size_t depth() const {
			return depth_;
		}
	};


}// namespace dice::einsum::tests::test_data
#endif//HYPERTRIE_TESTDATA_OPERAND_HPP
