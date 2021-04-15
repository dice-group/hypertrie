#include <fmt/format.h>
#include <range/v3/all.hpp>

#include "Dice/einsum/internal/new_subscript/SubscriptUnion.hpp"
namespace einsum::internal::new_subscript {

	SubscriptUnion::SubscriptUnion() = default;
	SubscriptUnion::~SubscriptUnion() = default;
	std::shared_ptr<SubscriptUnion> SubscriptUnion::make() {
		return std::make_shared<SubscriptUnion>();
	}
	const std::list<std::shared_ptr<Subscript>> &SubscriptUnion::union_operands() const { return union_operands_; }
	std::list<std::shared_ptr<Subscript>> &SubscriptUnion::union_operands() { return union_operands_; }
	SubscriptUnion *SubscriptUnion::append_union_operand(std::shared_ptr<Subscript> union_operand) {
		union_operands_.push_back(union_operand);
		return this;
	}

	std::string SubscriptUnion::str() const {
		auto operands = fmt::join(union_operands_ | ranges::views::transform([&](auto &n) { return n->str(); }), "+");
		return fmt::format("({}){}",
						   fmt::to_string(operands),
						   (this->result_subscript()) ? this->result_subscript()->str() : "");
	}
}// namespace einsum::internal::new_subscript