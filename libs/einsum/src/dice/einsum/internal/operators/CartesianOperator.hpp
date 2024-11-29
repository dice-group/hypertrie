#ifndef HYPERTRIE_CARTESIANOPERATOR_HPP
#define HYPERTRIE_CARTESIANOPERATOR_HPP

#include "dice/einsum/internal/CardinalityEstimation.hpp"
#include "dice/einsum/internal/operators/Operator_predeclare.hpp"

#include <robin_hood.h>

namespace dice::einsum::internal::operators {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct CartesianOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;
		using Entry_t = Entry<value_type, htt_t>;


	private:
		using SubResult = robin_hood::unordered_map<Key<value_type, htt_t>, value_type, dice::hash::DiceHashMartinus<Key<value_type, htt_t>>>;

		inline static void updateEntryKey(Subscript::OriginalResultPoss const &original_result_poss, Entry_t &sink,
										  Key<value_type, htt_t> const &source_key) noexcept {
			for (size_t i = 0; i < original_result_poss.size(); ++i) {
				sink[original_result_poss[i]] = source_key[i];
			}
		}

		class FullCartesianResult {
			Entry_t *entry_;
			value_type value_ = 1;
			size_t excluded_pos_ = 0;
			std::vector<std::vector<Entry_t>> sub_results_;
			bool ended_ = true;
			typename std::vector<size_t> iter_poss_;
			std::vector<Subscript::OriginalResultPoss> result_mapping_;

		public:
			FullCartesianResult() = default;

			FullCartesianResult(std::vector<SubResult> sub_results,
								CartesianSubSubscripts const &cartSubSubscript,
								Entry_t &entry,
								size_t const excluded_pos) noexcept
				: entry_{&entry},
				  excluded_pos_{excluded_pos},
				  iter_poss_(sub_results.size()) {
				assert(std::ranges::none_of(sub_results, [](auto const &sub_result) { return sub_result.empty(); }));
				// copy sub_result maps into vectors (faster to iterate, easier to maintain a position pointer)
				for (auto &sub_result : sub_results) {
					std::vector<Entry_t> &sub_result_vec = sub_results_.emplace_back();
					sub_result_vec.reserve(sub_result.size());
					for (const auto &[key, value] : sub_result) {
						sub_result_vec.emplace_back(key, value);
					}
					sub_result = {};
				}
				auto const &original_result_poss = cartSubSubscript.getOriginalResultPoss();
				for (size_t i = 0; i < original_result_poss.size(); ++i) {
					if (i != excluded_pos_) {
						result_mapping_.push_back(original_result_poss[i]);
					}
				}
			}

			inline void operator++() noexcept {
				for (std::size_t i = 0; i < sub_results_.size(); ++i) {
					auto const &sub_result = sub_results_[i];
					auto &iter_pos = iter_poss_[i];
					[[maybe_unused]] auto const &last_entry = sub_result[iter_pos];
					++iter_pos;
					const bool carry_over = iter_pos == sub_result.size();
					if (carry_over) {
						iter_pos = 0;
					}

					auto const &current_entry = sub_result[iter_pos];
					updateEntryKey(result_mapping_[i], *this->entry_, current_entry.key());
					assert(value_);
					assert(current_entry.value());
					assert(last_entry.value());
					assert(value_ * current_entry.value() / last_entry.value() != 0);
					if constexpr (not bool_valued) {// all entries are true anyways
						value_ = (value_ * current_entry.value()) / last_entry.value();
					}
					assert(value_);
					if (not carry_over) {
						++i;
						for (; i < sub_results_.size(); ++i) {
							auto const &sub_result_rem = sub_results_[i];
							auto const &iter_pos_rem = iter_poss_[i];
							auto const &current_entry_rem = sub_result_rem[iter_pos_rem];
							updateEntryKey(result_mapping_[i], *this->entry_, current_entry_rem.key());
						}
						return;
					}
				}
				assert(value_);
				ended_ = true;
			}

			value_type operator*() const noexcept {
				return value_;
			}

			void restart() noexcept {
				ended_ = false;
				std::ranges::fill(iter_poss_, 0);
				value_ = 1;
				for (size_t i = 0; i < sub_results_.size(); ++i) {
					auto const &sub_result = sub_results_[i];
					auto const &current_entry = sub_result[0];
					assert(value_);
					updateEntryKey(result_mapping_[i], *this->entry_, current_entry.key());
					if constexpr (not bool_valued) {
						value_ *= current_entry.value();
					}
				}
			}

			FullCartesianResult &begin() noexcept {
				restart();
				return *this;
			}

			constexpr bool end() const noexcept {
				return false;
			}

			operator bool() const noexcept {
				return not ended_;
			}
		};

		inline static std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>
		extract_operands(std::shared_ptr<Subscript> const &subscript,
						 Subscript::CartesianOperandPos cart_op_pos,
						 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands) {
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> sub_operands;
			for (auto const &original_op_pos : subscript->getCartesianSubscript().getOriginalOperandPoss(
						 cart_op_pos)) {
				sub_operands.emplace_back(operands[original_op_pos]);
			}
			return sub_operands;
		}


		static std::optional<std::vector<SubResult>> get_sub_results(size_t iterated_pos,
																	 std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>> &sub_operandss,
																	 std::vector<std::shared_ptr<Subscript>> const &sub_subscripts,
																	 std::shared_ptr<Context> &context) {
			const size_t no_cart_ops = sub_subscripts.size();

			std::vector<SubResult> sub_results(no_cart_ops - 1);

			for (size_t cart_op_pos = 0; cart_op_pos < no_cart_ops; ++cart_op_pos) {
				if (cart_op_pos == iterated_pos) {
					continue;
				}
				SubResult &sub_result = sub_results[cart_op_pos - (cart_op_pos > iterated_pos)];
				auto &sub_operands = sub_operandss[cart_op_pos];
				auto &sub_subscript = sub_subscripts[cart_op_pos];
				auto sub_entry_arg = Entry_t::make_filled(sub_subscript->resultLabelCount(), {});

				if (sub_subscript->all_result_done) {
					auto const &sub_entry = get_sub_operator<value_type, htt_t, allocator_type, true>(sub_subscript, context, sub_operands, sub_entry_arg);
					if (sub_entry.value()) {
						sub_result[sub_entry.key()] = sub_entry.value();
					}
				} else {
					for (auto const &sub_entry : get_sub_operator<value_type, htt_t, allocator_type, false>(sub_subscript, context, sub_operands, sub_entry_arg)) {
						assert(sub_entry.value());
						if constexpr (bool_valued) {
							sub_result[sub_entry.key()] = true;
						} else {
							sub_result[sub_entry.key()] += sub_entry.value();
						}
						context->check_time_out();
					}
				}
				if (sub_result.empty()) {
					return {};
				}
			}
			return sub_results;
		}

		static std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>> extract_suboperands(size_t &iterated_pos,
																											   std::shared_ptr<Subscript> const &subscript,
																											   std::shared_ptr<Context> &context,
																											   std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands) {
			double max_estimated_size = 0;
			std::vector<std::shared_ptr<Subscript>> const &sub_subscripts = subscript->getCartesianSubscript().getSubSubscripts();
			const size_t no_cart_ops = sub_subscripts.size();
			// initialize operands of the Cartesian product
			std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>> sub_operandss(no_cart_ops);
			for (size_t cart_op_pos = 0; cart_op_pos < no_cart_ops; ++cart_op_pos) {
				auto sub_operands = extract_operands(subscript, cart_op_pos, operands);
				double const estimated_size = CardinalityEstimation<htt_t, allocator_type>::estimate(sub_operands, sub_subscripts[cart_op_pos], context);
				sub_operandss[cart_op_pos] = std::move(sub_operands);
				if (estimated_size > max_estimated_size) {
					max_estimated_size = estimated_size;
					iterated_pos = cart_op_pos;
				}
			}
			return sub_operandss;
		}

	public:
		inline static std::generator<Entry<value_type, htt_t> const &> generator(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t>(entry_arg, subscript);
			std::vector<std::shared_ptr<Subscript>> const &sub_subscripts = subscript->getCartesianSubscript().getSubSubscripts();
			size_t iterated_pos = 0;
			// initialize operands of the Cartesian product and find out which operand to iterate (the one which is expected to yield the least results)
			std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>> sub_operandss = extract_suboperands(iterated_pos, subscript, context, operands);
			std::optional<std::vector<SubResult>> sub_results_opt = get_sub_results(iterated_pos, sub_operandss, sub_subscripts, context);
			if (not sub_results_opt.has_value()) {
				co_return;
			}
			FullCartesianResult calculated_operands(std::move(sub_results_opt.value()),
													subscript->getCartesianSubscript(),
													entry_arg,
													iterated_pos);
			// init iterator for the subscript part that is iterated as results are written out.
			auto sub_entry_arg = Entry_t::make_filled(sub_subscripts[iterated_pos]->resultLabelCount(), {});
			auto const &iterated_sub_op_result_mapping = subscript->getCartesianSubscript().getOriginalResultPoss()[iterated_pos];
			if (not sub_subscripts[iterated_pos]->all_result_done) {
				for (auto const &iterated_sub_entry : get_sub_operator<value_type, htt_t, allocator_type, false>(sub_subscripts[iterated_pos], context, sub_operandss[iterated_pos], sub_entry_arg)) {
					assert(iterated_sub_entry.value());
					for ([[maybe_unused]] auto precalculated_value : calculated_operands) {
						assert(precalculated_value);
						context->check_time_out();
						updateEntryKey(iterated_sub_op_result_mapping, entry_arg, iterated_sub_entry.key());
						if constexpr (not bool_valued) {
							entry_arg.value(iterated_sub_entry.value() * precalculated_value);
						}
						assert(entry_arg.value());
						co_yield entry_arg;
					}
				}
			} else {
				auto const &iterated_sub_entry = get_sub_operator<value_type, htt_t, allocator_type, true>(sub_subscripts[iterated_pos], context, sub_operandss[iterated_pos], sub_entry_arg);
				if (iterated_sub_entry.value()) {
					for (auto precalculated_value : calculated_operands) {
						assert(precalculated_value);
						context->check_time_out();
						updateEntryKey(iterated_sub_op_result_mapping, entry_arg, iterated_sub_entry.key());
						if constexpr (not bool_valued) {
							entry_arg.value(iterated_sub_entry.value() * precalculated_value);
						} else {
							entry_arg.value(iterated_sub_entry.value() and precalculated_value);
						}
						assert(entry_arg.value());
						co_yield entry_arg;
					}
				}
			}
		}


		inline static Entry<value_type, htt_t> const &
		single_result(
				[[maybe_unused]] std::shared_ptr<Subscript> const &subscript,
				[[maybe_unused]] std::shared_ptr<Context> &context,
				[[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				[[maybe_unused]] Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t>(entry_arg, subscript);
			std::vector<std::shared_ptr<Subscript>> const &sub_subscripts = subscript->getCartesianSubscript().getSubSubscripts();
			size_t iterated_pos = 0;
			// initialize operands of the Cartesian product and find out which operand to iterate (the one which is expected to yield the least results)
			std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>> sub_operandss = extract_suboperands(iterated_pos, subscript, context, operands);
			std::optional<std::vector<SubResult>> sub_results_opt = get_sub_results(iterated_pos, sub_operandss, sub_subscripts, context);
			if (not sub_results_opt.has_value()) {
				entry_arg.value(0);
				return entry_arg;
			}
			FullCartesianResult calculated_operands(std::move(sub_results_opt.value()),
													subscript->getCartesianSubscript(),
													entry_arg,
													iterated_pos);
			// init iterator for the subscript part that is iterated as results are written out.
			auto sub_entry_arg = Entry<value_type, htt_t>::make_filled(sub_subscripts[iterated_pos]->resultLabelCount(), {});
			auto const &iterated_sub_entry = get_sub_operator<value_type, htt_t, allocator_type, true>(sub_subscripts[iterated_pos], context, sub_operandss[iterated_pos], sub_entry_arg);
			if constexpr (not bool_valued) {
				value_type other_subs_value = calculated_operands.begin().operator*();
				entry_arg.value(iterated_sub_entry.value() * other_subs_value);
			} else {
				entry_arg.value(iterated_sub_entry.value());
			}
			return entry_arg;
		}
	};

}// namespace dice::einsum::internal::operators
#endif//HYPERTRIE_CARTESIANOPERATOR_HPP
