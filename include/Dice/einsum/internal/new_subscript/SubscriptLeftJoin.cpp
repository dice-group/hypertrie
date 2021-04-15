#include <fmt/format.h>
#include <range/v3/all.hpp>

#include "Dice/einsum/internal/new_subscript/SubscriptLeftJoin.hpp"

namespace einsum::internal::new_subscript {
	SubscriptLeftJoin::~SubscriptLeftJoin() = default;
	std::shared_ptr<SubscriptLeftJoin> SubscriptLeftJoin::make() {
		return std::make_shared<SubscriptLeftJoin>();
	}
	const std::shared_ptr<Subscript> &SubscriptLeftJoin::left_operand() const {
		return left_operand_;
	}
	std::shared_ptr<Subscript> &SubscriptLeftJoin::left_operand() {
		return left_operand_;
	}
	SubscriptLeftJoin *SubscriptLeftJoin::left_operand(std::shared_ptr<Subscript> left_operand) {
		this->left_operand_ = std::move(left_operand);
		return this;
	}
	const std::list<std::shared_ptr<Subscript>> &SubscriptLeftJoin::right_operands() const {
		return right_operands_;
	}
	std::list<std::shared_ptr<Subscript>> &SubscriptLeftJoin::right_operands() {
		return right_operands_;
	}
	SubscriptLeftJoin *SubscriptLeftJoin::right_operands(std::list<std::shared_ptr<Subscript>> right_operands) {
		this->right_operands_ = std::move(right_operands);
		return this;
	}
	SubscriptLeftJoin *SubscriptLeftJoin::append_right_operand(std::shared_ptr<Subscript> operand) {
		right_operands_.push_back(operand);
		return this;
	}
	std::string SubscriptLeftJoin::str() const {
		return fmt::format("({},[{}]){}",
						   left_operand_->str(),
						   fmt::join(right_operands_ | ranges::views::transform([&](auto &n) { return n->str(); }), "],["),
						   (this->result_subscript()) ? this->result_subscript()->str() : "");
	}

}// namespace einsum::internal::new_subscript