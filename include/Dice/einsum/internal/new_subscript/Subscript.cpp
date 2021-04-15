#include "Dice/einsum/internal/new_subscript/Subscript.hpp"

namespace einsum::internal::new_subscript {
	Subscript::~Subscript() = default;

	void Subscript::set_result_subscript(ResultLabels result_labels, bool distinct) {
		result_subscript_ = ResultSubscript::make(result_labels, distinct);
	}

	void Subscript::set_result_subscript(std::initializer_list<Label> result_labels, bool distinct) {
		result_subscript_ = ResultSubscript::make(result_labels, distinct);
	}

	std::shared_ptr<ResultSubscript> &Subscript::result_subscript() { return result_subscript_; }
	const std::shared_ptr<ResultSubscript> &Subscript::result_subscript() const { return result_subscript_; }

	bool Subscript::distinct() const noexcept(false) { return result_subscript_->distinct(); }
	bool &Subscript::distinct() noexcept(false) { return result_subscript_->distinct(); }

	const ResultLabels &Subscript::result_labels() const noexcept(false) { return result_subscript_->result_labels(); }
	ResultLabels &Subscript::result_labels() noexcept(false) { return result_subscript_->result_labels(); }

}// namespace einsum::internal::new_subscript