#include <fmt/format.h>
#include <range/v3/all.hpp>

#include "Dice/einsum/internal/new_subscript/SubscriptMinus.hpp"
namespace einsum::internal::new_subscript {
	SubscriptMinus::~SubscriptMinus() = default;

	std::shared_ptr<SubscriptMinus> SubscriptMinus::make() {
		return std::make_shared<SubscriptMinus>();
	}

	const std::shared_ptr<Subscript> &SubscriptMinus::subtrahend() const {
		return subtrahend_;
	}
	std::shared_ptr<Subscript> &SubscriptMinus::subtrahend() {
		return subtrahend_;
	}
	SubscriptMinus *SubscriptMinus::subtrahend(std::shared_ptr<Subscript> subtrahend) {
		this->subtrahend_ = std::move(subtrahend);
		return this;
	}

	const std::list<std::shared_ptr<Subscript>> &SubscriptMinus::minuends() const {
		return minuends_;
	}
	std::list<std::shared_ptr<Subscript>> &SubscriptMinus::minuends() {
		return minuends_;
	}
	SubscriptMinus *SubscriptMinus::minuends(std::list<std::shared_ptr<Subscript>> minuends) {
		this->minuends_ = std::move(minuends);
		return this;
	}

	SubscriptMinus *SubscriptMinus::append_minuend(std::shared_ptr<Subscript> operand) {
		minuends_.push_back(operand);
		return this;
	}

	std::string SubscriptMinus::str() const {
		return fmt::format("({}-{}){}",
						   subtrahend_->str(),
						   fmt::join(minuends_ | ranges::views::transform([&](auto &n) { return n->str(); }), "-"),
						   (this->result_subscript()) ? this->result_subscript()->str() : "");
	}


}// namespace einsum::internal::new_subscript
