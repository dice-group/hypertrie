#ifndef HYPERTRIE_TORCHTENSORABSTRACTION_HPP
#define HYPERTRIE_TORCHTENSORABSTRACTION_HPP

#include <Dice/hypertrie/hypertrie.hpp>
#include <torch/torch.h>
#include <fmt/format.h>

namespace hypertrie::tests {
	template<typename dtype>
	class TorchHelper {
	public:
		static dtype &resolve(torch::Tensor &tensor, const hypertrie::Key<size_t> &key) {
			auto x =  internal::switch_cases<1, hypertrie::hypertrie_depth_limit>(
					key.size(),
					[&](auto depth_arg) -> dtype* {
						auto &x = getTorchValue_raw<depth_arg>(tensor, key);
						return &x;
					},
					[]() -> dtype* {
						throw std::logic_error{
								fmt::format("Torch tensor value retrieval only supported for depths [1,{})", hypertrie_depth_limit)};
					});
			return *x;
		}

	private:
		template<size_t depth>
		inline static dtype &getTorchValue_raw(torch::Tensor &tensor, const hypertrie::Key<size_t> &key) {
			auto accessor = tensor.accessor<dtype, depth>();
			return getTorchValue_rek<depth>(accessor, key);
		}

		template<size_t depth, size_t current_depth = depth>
		inline static dtype &getTorchValue_rek(
				torch::TensorAccessor<dtype, current_depth> &accessor,
				const hypertrie::Key<size_t> &key) {
			if constexpr (current_depth > 1) {
				torch::TensorAccessor<dtype, current_depth - 1 > sub_accessor = accessor[key[depth - current_depth]];
				return getTorchValue_rek<depth, current_depth - 1>(sub_accessor, key);
			} else {// current depth == 1
				return accessor[key[depth - current_depth]];
			}
		}
	};
}// namespace hypertrie::tests

#endif//HYPERTRIE_TORCHTENSORABSTRACTION_HPP
