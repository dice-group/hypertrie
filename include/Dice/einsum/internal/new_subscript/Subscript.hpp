#ifndef HYPERTRIE_SUBSCRIPT_HPP
#define HYPERTRIE_SUBSCRIPT_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <range/v3/all.hpp>

#include "Dice/einsum/internal/new_subscript/AbstractSubscript.hpp"


namespace einsum::internal::new_subscript {

	using Label = u_int8_t;
	using ResultLabels = std::vector<Label>;
	using OperandLabels = std::vector<Label>;

	class ResultSubscript : public std::enable_shared_from_this<ResultSubscript> {
		ResultLabels result_labels_;
		bool distinct_;

	public:
		explicit ResultSubscript(ResultLabels resultLabels = {}, bool distinct = false) : result_labels_(resultLabels), distinct_(distinct) {}

		static std::shared_ptr<ResultSubscript> make(ResultLabels resultLabels = {}, bool distinct = false) {
			return std::make_shared<ResultSubscript>(resultLabels, distinct);
		}

		[[nodiscard]] const ResultLabels &result_labels() const { return result_labels_; }
		[[nodiscard]] ResultLabels &result_labels() { return result_labels_; }
		auto result_labels(ResultLabels result_labels) {
			this->result_labels_.swap(result_labels);
			return this;
		}
		auto result_labels(std::initializer_list<Label> result_labels) {
			this->result_labels_ = {result_labels};
			return this;
		}

		[[nodiscard]] bool distinct() const { return distinct_; }
		[[nodiscard]] bool &distinct() { return distinct_; }
		auto distinct(bool distinct) {
			distinct_ = distinct;
			return this;
		}

		auto str() const {
			return fmt::format("{}{}",
							   (distinct_) ? "-->" : "->",
							   fmt::join(this->result_labels() | ranges::views::transform([](auto c) { return std::string(1, c); }), ""));
		}
	};


	/**
	 * Bag-Semantics: xy,yz,f->x
	 * Set-Semantics: xy,yz,f-->x
	 */
	class Subscript : public AbstractSubscript {
	protected:
		std::shared_ptr<ResultSubscript> result_subscript_;

	public:
		virtual ~Subscript() = default;

		void set_result_subscript(ResultLabels result_labels = {}, bool distinct = false) {
			result_subscript_ = ResultSubscript::make(result_labels, distinct);
		}

		void set_result_subscript(std::initializer_list<Label> result_labels, bool distinct = false) {
			result_subscript_ = ResultSubscript::make(result_labels, distinct);
		}

		[[nodiscard]] std::shared_ptr<ResultSubscript> &result_subscript() { return result_subscript_; }
		[[nodiscard]] const std::shared_ptr<ResultSubscript> &result_subscript() const { return result_subscript_; }

		[[nodiscard]] bool distinct() const noexcept(false) { return result_subscript_->distinct(); }
		[[nodiscard]] bool &distinct() noexcept(false) { return result_subscript_->distinct(); }

		[[nodiscard]] const ResultLabels &result_labels() const noexcept(false) { return result_subscript_->result_labels(); }
		[[nodiscard]] ResultLabels &result_labels() noexcept(false) { return result_subscript_->result_labels(); }
	};
}// namespace einsum::internal::new_subscript
#endif//HYPERTRIE_SUBSCRIPT_HPP
