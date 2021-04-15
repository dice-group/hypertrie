#include <fmt/format.h>
#include <range/v3/all.hpp>

#include "Dice/einsum/internal/new_subscript/ResultSubscript.hpp"

namespace einsum::internal::new_subscript {

	ResultSubscript::ResultSubscript(ResultLabels result_labels, bool distinct) : result_labels_(result_labels), distinct_(distinct) {}

	std::shared_ptr<ResultSubscript> ResultSubscript::make(ResultLabels result_labels, bool distinct) {
		return std::make_shared<ResultSubscript>(result_labels, distinct);
	}

	const ResultLabels &ResultSubscript::result_labels() const { return result_labels_; }
	ResultLabels &ResultSubscript::result_labels() { return result_labels_; }
	auto ResultSubscript::result_labels(ResultLabels result_labels) {
		this->result_labels_.swap(result_labels);
		return this;
	}
	auto ResultSubscript::result_labels(std::initializer_list<Label> result_labels) {
		this->result_labels_ = {result_labels};
		return this;
	}

	bool ResultSubscript::distinct() const { return distinct_; }
	bool &ResultSubscript::distinct() { return distinct_; }
	auto ResultSubscript::distinct(bool distinct) {
		distinct_ = distinct;
		return this;
	}

	std::string ResultSubscript::str() const {
		return fmt::format("{}{}",
						   (distinct_) ? "-->" : "->",
						   fmt::join(this->result_labels() | ranges::views::transform([](auto c) { return std::string(1, c); }), ""));
	}
}// namespace einsum::internal::new_subscript