#ifndef HYPERTRIE_SUBSCRIPT_HPP
#define HYPERTRIE_SUBSCRIPT_HPP

#include <optional>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <range/v3/all.hpp>

namespace einsum::internal::new_subscript {

	using Label = u_int8_t;
	using ResultLabels = std::vector<Label>;
	using OperandLabels = std::vector<Label>;

	class ResultSubscript {
		ResultLabels result_labels_;
		bool distinct_;

	public:
		[[nodiscard]] const ResultLabels &result_labels() const { return result_labels_; }
		[[nodiscard]] ResultLabels &result_labels() { return result_labels_; }

		[[nodiscard]] bool distinct() const { return distinct_; }
		[[nodiscard]] bool &distinct() { return distinct_; }

		explicit ResultSubscript(ResultLabels resultLabels = {}, bool distinct = false) : result_labels_(resultLabels), distinct_(distinct) {}

		auto str() const {
			return fmt::format("{}{}",
							   (distinct_) ? "-->" : "->",
							   fmt::join(this->result_labels() | ranges::views::transform([](auto c) { return std::string(1, c); }), ""));
		}
	};

	class SubscriptCartesian;
	class SubscriptJoin;
	class SubscriptLeftJoin;
	class SubscriptOperand;
	class SubscriptUnion;

	/**
	 * Bag-Semantics: xy,yz,f->x
	 * Set-Semantics: xy,yz,f-->x
	 */
	class Subscript {
	protected:
		std::shared_ptr<ResultSubscript> result_subscript_;

	public:
		virtual ~Subscript() {}

		void set_result_subscript(ResultLabels resultLabels = {}, bool distinct = false) {
			result_subscript_ = std::make_shared<ResultSubscript>(resultLabels, distinct);
		}

		void set_result_subscript(std::initializer_list<Label> resultLabels, bool distinct = false) {
			result_subscript_ = std::make_shared<ResultSubscript>(resultLabels, distinct);
		}

		[[nodiscard]] std::shared_ptr<ResultSubscript> &result_subscript() { return result_subscript_; }
		[[nodiscard]] const std::shared_ptr<ResultSubscript> &result_subscript() const { return result_subscript_; }

		[[nodiscard]] bool distinct() const noexcept(false) { return result_subscript_->distinct(); }
		[[nodiscard]] bool &distinct() noexcept(false) { return result_subscript_->distinct(); }

		[[nodiscard]] const ResultLabels &result_labels() const noexcept(false) { return result_subscript_->result_labels(); }
		[[nodiscard]] ResultLabels &result_labels() noexcept(false) { return result_subscript_->result_labels(); }

		virtual std::string str() const = 0;
	};
}// namespace einsum::internal::new_subscript
#endif//HYPERTRIE_SUBSCRIPT_HPP
