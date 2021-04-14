#ifndef HYPERTRIE_SUBSCRIPTJOIN_HPP
#define HYPERTRIE_SUBSCRIPTJOIN_HPP
#include "Dice/einsum/internal/new_subscript/Subscript.hpp"
#include <range/v3/all.hpp>
namespace einsum::internal::new_subscript {
	/**
	 * Example: xy,yz->yz
	 */
	class SubscriptJoin : public Subscript {
	protected:
		std::list<std::shared_ptr<Subscript>> join_operands;

	public:
		void append(std::shared_ptr<Subscript> operand) {
			join_operands.push_back(std::move(operand));
		}

		SubscriptJoin() = default;

		virtual ~SubscriptJoin() {}


		static std::shared_ptr<SubscriptJoin> make_triple_pattern(OperandLabels operand_labels);

		static std::shared_ptr<SubscriptJoin> make_basic_graph_pattern(std::initializer_list<OperandLabels> operands_labels);

		std::string str() const {
			return fmt::format("({}){}",
							   fmt::join(join_operands | ranges::views::transform([&](auto &n) { return n->str(); }), ","),
							   (this->result_subscript()) ? this->result_subscript()->str() : "");
		}
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_SUBSCRIPTJOIN_HPP
