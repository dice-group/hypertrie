#ifndef HYPERTRIE_RESULTSUBSCRIPT_HPP
#define HYPERTRIE_RESULTSUBSCRIPT_HPP

#include <memory>
#include <string>

#include "Dice/einsum/internal/new_subscript/SubscriptCommons.hpp"

namespace einsum::internal::new_subscript {

	class ResultSubscript : public std::enable_shared_from_this<ResultSubscript> {
		ResultLabels result_labels_;
		bool distinct_;

	public:
		explicit ResultSubscript(ResultLabels result_labels = {}, bool distinct = false);

		static std::shared_ptr<ResultSubscript> make(ResultLabels result_labels = {}, bool distinct = false);

		[[nodiscard]] const ResultLabels &result_labels() const;
		[[nodiscard]] ResultLabels &result_labels();
		auto result_labels(ResultLabels result_labels);
		auto result_labels(std::initializer_list<Label> result_labels);

		[[nodiscard]] bool distinct() const;
		[[nodiscard]] bool &distinct();
		auto distinct(bool distinct);

		std::string str() const;
	};
};// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_RESULTSUBSCRIPT_HPP
