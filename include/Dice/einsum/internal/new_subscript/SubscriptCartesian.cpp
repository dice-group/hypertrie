#include <fmt/format.h>
#include <range/v3/all.hpp>

#include "Dice/einsum/internal/new_subscript/SubscriptCartesian.hpp"

namespace einsum::internal::new_subscript {

	SubscriptCartesian::~SubscriptCartesian() = default;

	std::shared_ptr<SubscriptCartesian> SubscriptCartesian::make() {
		return std::make_shared<SubscriptCartesian>();
	}

	std::string SubscriptCartesian::str() const {
		return fmt::format("({}){}",
						   fmt::join(cartesian_operands | ranges::views::transform([&](auto &n) { return n->str(); }), ","),
						   (this->result_subscript()) ? this->result_subscript()->str() : "");
	}
}// namespace einsum::internal::new_subscript