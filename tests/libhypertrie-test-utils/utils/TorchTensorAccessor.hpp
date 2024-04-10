#ifndef HYPERTRIE_TORCHTENSORACCESSOR_HPP
#define HYPERTRIE_TORCHTENSORACCESSOR_HPP


#include <dice/hypertrie/Hypertrie_trait.hpp>
#include <fmt/format.h>
#include <torch/torch.h>
#include <dice/template-library/switch_cases.hpp>

namespace dice::einsum::tests::utils {

	template<typename dtype>
	concept TorchDtype = std::is_same_v<dtype, bool> or
			std::is_same_v<dtype,
						   int64_t>;

	template<typename dtype_t, ::dice::hypertrie::HypertrieTrait tr_t>
	class TorchTensorAccessor {
	public:
		using tr = tr_t;

		using dtype = dtype_t;

		static inline dtype get(torch::Tensor &tensor, std::vector<typename tr::key_part_type> const &key) {
			return template_library::switch_cases<1, ::dice::hypertrie::hypertrie_max_depth + 1>(
					key.size(),
					[&](auto depth_arg) -> dtype {
						return dtype(get_torch_value<depth_arg>(tensor, key));
					},
					[]() -> dtype {
						throw std::logic_error{
								fmt::format("Torch tensor value retrieval only supported for depths [1,{}]", ::dice::hypertrie::hypertrie_max_depth)};
					});
		}

		static inline void set(torch::Tensor &tensor, hypertrie::NonZeroEntry<tr> const &entry) {
			return template_library::switch_cases<1, ::dice::hypertrie::hypertrie_max_depth + 1>(
					entry.key().size(),
					[&](auto depth_arg) {
						get_torch_value<depth_arg>(tensor, entry.key()) = dtype(entry.value());
					},
					[]() {
						throw std::logic_error{
								fmt::format("Torch tensor value modification only supported for depths [1,{}]", ::dice::hypertrie::hypertrie_max_depth)};
					});
		}

		template<size_t depth>
		inline static int64_t &get_torch_value(torch::Tensor &tensor, std::vector<typename tr::key_part_type> const &key) {
			auto accessor = tensor.accessor<int64_t, depth>();
			return get_torch_value_rek<depth>(accessor, key);
		}

		template<size_t depth, size_t current_depth = depth>
		inline static int64_t &get_torch_value_rek(torch::TensorAccessor<int64_t, current_depth> &accessor, const std::vector<typename tr::key_part_type> &key) {
			if constexpr (current_depth > 1) {
				torch::TensorAccessor<int64_t, current_depth - 1> sub_accessor = accessor[key[depth - current_depth]];
				return get_torch_value_rek<depth, current_depth - 1>(sub_accessor, key);
			} else {// current depth == 1
				return accessor[key[depth - current_depth]];
			}
		}
	};
}// namespace dice::einsum::tests::utils

#endif//HYPERTRIE_TORCHTENSORACCESSOR_HPP
