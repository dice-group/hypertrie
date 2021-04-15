#ifndef HYPERTRIE_SUBSCRIPTCOMMONS_HPP
#define HYPERTRIE_SUBSCRIPTCOMMONS_HPP

#include <vector>

namespace einsum::internal::new_subscript {

	using Label = u_int8_t;
	using ResultLabels = std::vector<Label>;
	using OperandLabels = std::vector<Label>;
}// namespace einsum::internal::new_subscript
#endif//HYPERTRIE_SUBSCRIPTCOMMONS_HPP
