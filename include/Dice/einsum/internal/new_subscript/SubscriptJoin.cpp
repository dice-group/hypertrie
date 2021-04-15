#include <fmt/format.h>
#include <range/v3/all.hpp>

#include "Dice/einsum/internal/new_subscript/SubscriptJoin.hpp"
namespace einsum::internal::new_subscript {

	SubscriptJoin::~SubscriptJoin() = default;

	SubscriptJoin *SubscriptJoin::append(std::shared_ptr<AbstractSubscript> operand) {
		join_operands.push_back(std::move(operand));
		return this;
	}
	std::shared_ptr<SubscriptJoin> SubscriptJoin::make() {
		return std::make_shared<SubscriptJoin>();
	}

	std::string SubscriptJoin::str() const {
		return fmt::format("({}){}",
						   fmt::join(join_operands | ranges::views::transform([&](auto &n) { return n->str(); }), ","),
						   (this->result_subscript()) ? this->result_subscript()->str() : "");
	}

}// namespace einsum::internal::new_subscript