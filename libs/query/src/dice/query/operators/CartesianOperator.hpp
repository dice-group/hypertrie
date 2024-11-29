#ifndef QUERY_CARTESIANOPERATOR_HPP
#define QUERY_CARTESIANOPERATOR_HPP

#include "CardinalityEstimation.hpp"
#include "Operator_predeclare.hpp"

#include <robin_hood.h>

#include <utility>

namespace dice::query::operators {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool Optional = false>
	struct CartesianOperator {
		static constexpr bool bool_valued = std::is_same_v<value_type, bool>;
		using Entry_t = Entry<value_type, htt_t>;

	private:
		using SubResult = robin_hood::unordered_map<Key<value_type, htt_t>, value_type, dice::hash::DiceHashMartinus<Key<value_type, htt_t>>>;

		inline static void updateEntryKey(const std::vector<size_t> &poss,
										  Entry_t &sink,
										  Key<value_type, htt_t> const &source_key) {
			for (auto const &pos : poss) {
				sink[pos] = source_key[pos];
			}
		}

		class FullCartesianResult {
			Entry_t *entry_;
			value_type value_ = 1;
			size_t excluded_pos_ = 0;
			std::vector<std::vector<Entry_t>> sub_results_;
			bool ended_ = true;
			typename std::vector<size_t> iter_poss_;
			std::vector<std::vector<size_t>> result_poss_;

		public:
			FullCartesianResult() = default;

			FullCartesianResult(std::vector<SubResult> sub_results,
								Entry_t &entry,
								const size_t excluded_pos,
								std::vector<std::vector<size_t>> const &result_poss) noexcept
				: entry_{&entry},
				  excluded_pos_{excluded_pos},
				  iter_poss_(sub_results.size()) {
				// copy sub_result maps into vectors (faster to iterate, easier to maintain a position pointer)
				for (auto &sub_result : sub_results) {
					std::vector<Entry_t> &sub_result_vec = sub_results_.emplace_back();
					if (sub_result.empty()) {
						// if the cartesian is not between optional components this part will not be reached
						// populate the corresponding vector with a single entry, whose key_parts will remain unbound
						sub_result_vec.emplace_back(Entry_t::make_filled(entry.size(), {}));
						continue;
					}
					sub_result_vec.reserve(sub_result.size());
					for (const auto &[key, value] : sub_result)
						sub_result_vec.emplace_back(key, value);
					sub_result = {};
				}
				for (size_t i = 0; i < result_poss.size(); ++i)
					if (i != excluded_pos_)
						result_poss_.push_back(result_poss[i]);
			}

			inline void operator++() noexcept {
				for (size_t i = 0; i < sub_results_.size(); ++i) {
					auto const &sub_result = sub_results_[i];
					auto &iter_pos = iter_poss_[i];
					[[maybe_unused]] auto const &last_entry = sub_result[iter_pos];
					++iter_pos;
					const bool carry_over = iter_pos == sub_result.size();
					if (carry_over)
						iter_pos = 0;

					auto const &current_entry = sub_result[iter_pos];
					updateEntryKey(result_poss_[i], *this->entry_, current_entry.key());
					assert(value_);
					assert(current_entry.value());
					assert(last_entry.value());
					assert(value_ * current_entry.value() / last_entry.value() != 0);
					if constexpr (not bool_valued)// all entries are true anyway
						value_ = (value_ * current_entry.value()) / last_entry.value();
					assert(value_);
					if (not carry_over) {
						++i;
						for (; i < sub_results_.size(); ++i) {
							auto const &sub_result_rem = sub_results_[i];
							auto const &iter_pos_rem = iter_poss_[i];
							auto const &current_entry_rem = sub_result_rem[iter_pos_rem];
							updateEntryKey(result_poss_[i], *this->entry_, current_entry_rem.key());
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
					updateEntryKey(result_poss_[i], *this->entry_, current_entry.key());
					if constexpr (not bool_valued)
						value_ *= current_entry.value();
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

		/*
		 * Materializes the results of the cartesian components
		 * The component corresponding to iterated_pos will be skipped
		 */
		static std::vector<SubResult>
		get_sub_results(size_t iterated_pos,
						std::vector<OperandDependencyGraph> &cartesian_components,
						std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>> &sub_operandss,
						Query<htt_t, allocator_type> const &query) {
			const size_t no_cart_ops = cartesian_components.size();
			std::vector<SubResult> sub_results(no_cart_ops - 1);
			for (size_t cart_op_pos = 0; cart_op_pos < no_cart_ops; ++cart_op_pos) {
				if (cart_op_pos == iterated_pos)
					continue;
				auto sub_entry_arg = Entry_t::make_filled(query.projected_vars().size(), {});
				SubResult &sub_result = sub_results[cart_op_pos - (cart_op_pos > iterated_pos)];
				auto &sub_operands = sub_operandss[cart_op_pos];
				auto &cart_comp = cartesian_components[cart_op_pos];

				if (query.all_result_done(cart_comp)) {
					auto const &sub_entry = get_sub_operator<value_type, htt_t, allocator_type, true>(cart_comp, sub_operands, query, sub_entry_arg);
					if (sub_entry.value())
						sub_result[sub_entry.key()] = sub_entry.value();
				} else {
					for (auto const &sub_entry : get_sub_operator<value_type, htt_t, allocator_type, false>(cart_comp, sub_operands, query, sub_entry_arg)) {
						assert(sub_entry.value());
						if constexpr (bool_valued)
							sub_result[sub_entry.key()] = true;
						else
							sub_result[sub_entry.key()] += sub_entry.value();
						query.check_time_out();
					}
				}
				// if the cartesian is between non-optional components and the sub_result is empty, terminate
				if constexpr (not Optional) {
					if (sub_result.empty())
						return {};
				}
			}
			return sub_results;
		}

		/*
		 * Initializes the components of the Cartesian: sub_operandss
		 * Initializes the positions of the result that each component modifies: result_poss
		 * The component that is expected to yield the least results will be iterated
		 */
		static std::pair<std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>>, std::vector<std::vector<size_t>>>
		init_cartesian(size_t &iterated_pos,
					   std::vector<OperandDependencyGraph> &cartesian_components,
					   std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					   Query<htt_t, allocator_type> const &q) {
			double max_estimated_size = 0;
			size_t const no_cart_ops = cartesian_components.size();
			// initialization
			std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>> sub_operandss(no_cart_ops);
			std::vector<std::vector<size_t>> result_poss(no_cart_ops);
			// extract the operands of each component and find out which component will be iterated
			for (size_t cart_op_pos = 0; cart_op_pos < no_cart_ops; ++cart_op_pos) {
				auto sub_operands = extract_operands(cartesian_components[cart_op_pos], operands);
				double const estimated_size = CardinalityEstimation<htt_t, allocator_type>::estimate(cartesian_components[cart_op_pos], sub_operands, q);
				sub_operandss[cart_op_pos] = std::move(sub_operands);
				for (auto const &label : cartesian_components[cart_op_pos].operands_var_ids_set()) {
					if (q.contains_proj_var(label))
						result_poss[cart_op_pos].push_back(q.projected_var_position(label));
				}
				if (estimated_size > max_estimated_size) {
					max_estimated_size = estimated_size;
					iterated_pos = cart_op_pos;
				}
			}
			return std::make_pair(std::move(sub_operandss), std::move(result_poss));
		}

	public:
		inline static std::generator<Entry<value_type, htt_t> const &>
		generator(OperandDependencyGraph &odg,
				  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				  Query<htt_t, allocator_type> const &query,
				  Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t, allocator_type>(entry_arg, odg, query);
			auto &cart_comps = odg.cartesian_components();
			size_t iterated_pos = 0;// the cartesian component that will be iterated; the rest will be materialized
			auto [sub_operandss, result_poss] = init_cartesian(iterated_pos, cart_comps, operands, query);
			// materialize the components (except for the component corresponding to iterated_pos)
			std::vector<SubResult> sub_results = get_sub_results(iterated_pos, cart_comps, sub_operandss, query);
			// sub_results will be empty only if we have cartesian between non-optional components and any of the sub_results are empty
			if constexpr (not Optional) {
				if (sub_results.empty())
					co_return;
			}

			FullCartesianResult calculated_operands(std::move(sub_results), entry_arg, iterated_pos, result_poss);

			// init iterator for the subscript part that is iterated as results are written out.
			auto sub_entry_arg = Entry_t::make_filled(query.projected_vars().size(), {});
			// a boolean flag used in the case of cartesian between optional components
			// indicates whether the iterated component has at least one solution
			bool has_result = false;

			if (not query.all_result_done(cart_comps[iterated_pos])) {
				for (auto const &iter_sub_entry : get_sub_operator<value_type, htt_t, allocator_type, false>(cart_comps[iterated_pos], sub_operandss[iterated_pos], query, sub_entry_arg)) {
					if constexpr (Optional)
						has_result = true;
					assert(iter_sub_entry.value());
					for ([[maybe_unused]] auto precalculated_value : calculated_operands) {
						assert(precalculated_value);
						query.check_time_out();
						updateEntryKey(result_poss[iterated_pos], entry_arg, iter_sub_entry.key());
						if constexpr (not bool_valued)
							entry_arg.value(iter_sub_entry.value() * precalculated_value);
						else
							entry_arg.value(iter_sub_entry.value() and precalculated_value);
						assert(entry_arg.value());
						co_yield entry_arg;
					}
				}
				if constexpr (Optional) {
					// in case the iterated component does not have a single solution, we need to return the precalculated solutions only once
					if (not has_result) {
						for ([[maybe_unused]] auto precalculated_value : calculated_operands) {
							assert(precalculated_value);
							assert(entry_arg.value());
							co_yield entry_arg;
						}
					}
				}
			} else {
				auto const &iterated_sub_entry = get_sub_operator<value_type, htt_t, allocator_type, true>(cart_comps[iterated_pos], sub_operandss[iterated_pos], query, sub_entry_arg);
				if (iterated_sub_entry.value()) {
					for (auto precalculated_value : calculated_operands) {
						assert(precalculated_value);
						query.check_time_out();
						updateEntryKey(result_poss[iterated_pos], entry_arg, iterated_sub_entry.key());
						if constexpr (not bool_valued)
							entry_arg.value(iterated_sub_entry.value() * precalculated_value);
						else
							entry_arg.value(iterated_sub_entry.value() and precalculated_value);
						assert(entry_arg.value());
						co_yield entry_arg;
					}
				} else {
					if constexpr (Optional) {
						// in case the iterated component does not have a single solution, we need to return the precalculated solutions only once
						for (auto precalculated_value : calculated_operands) {
							assert(precalculated_value);
							entry_arg.value(precalculated_value);
							co_yield entry_arg;
						}
					}
				}
			}
		}

		inline static Entry<value_type, htt_t> const &
		single_result(OperandDependencyGraph &odg,
					  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					  Query<htt_t, allocator_type> const &query,
					  Entry<value_type, htt_t> &entry_arg) {
			clear_used_entry_poss<value_type, htt_t, allocator_type>(entry_arg, odg, query);
			auto &cart_comps = odg.cartesian_components();
			size_t iterated_pos = 0;// the cartesian component that will be iterated; the rest will be materialized
			auto [sub_operandss, result_poss] = init_cartesian(iterated_pos, cart_comps, operands, query);
			std::vector<SubResult> sub_results = get_sub_results(iterated_pos, cart_comps, sub_operandss, query);
			// sub_results will be empty only if we have cartesian between non-optional components and any of the sub_results are empty
			if constexpr (not Optional) {
				if (sub_results.empty()) {
					entry_arg.value(0);
					return entry_arg;
				}
			}

			FullCartesianResult calculated_operands(std::move(sub_results), entry_arg, iterated_pos, result_poss);

			// init iterator for the subscript part that is iterated as results are written out.
			auto sub_entry_arg = Entry_t::make_filled(query.projected_vars().size(), {});

			auto const &iterated_sub_entry = get_sub_operator<value_type, htt_t, allocator_type, true>(cart_comps[iterated_pos], sub_operandss[iterated_pos], query, sub_entry_arg);
			if constexpr (not bool_valued) {
				value_type other_subs_value = calculated_operands.begin().operator*();
				assert(other_subs_value);
				if constexpr (Optional) {
					if (iterated_sub_entry.value())
						entry_arg.value(iterated_sub_entry.value() * other_subs_value);
					else
						entry_arg.value(other_subs_value);
				} else {
					entry_arg.value(iterated_sub_entry.value() * other_subs_value);
				}
			} else {
				if constexpr (Optional) {
					entry_arg.value(1);
				} else {
					entry_arg.value(iterated_sub_entry.value());
				}
			}
			return entry_arg;
		}
	};

}// namespace dice::query::operators
#endif//QUERY_CARTESIANOPERATOR_HPP
