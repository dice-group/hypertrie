#ifndef QUERY_QUERY_HPP
#define QUERY_QUERY_HPP

#include <chrono>
#include <boost/container/flat_map.hpp>

#include <dice/hypertrie/HashJoin.hpp>

#include "OperandDependencyGraph.hpp"

namespace dice::query {

	/**
	 * @brief Contains information about the query to be executed.
	 * @tparam htt_t: Bool-valued Hypertrie trait
	 * @tparam allocator_type: The allocator
	 */
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	class Query {
	private:
		OperandDependencyGraph odg_;
		std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> operands_;
		std::vector<char> proj_vars_;
		boost::container::flat_map<char, std::size_t> proj_vars_pos_;
		// for timeouts
		std::chrono::steady_clock::time_point start_time_;
		std::chrono::steady_clock::time_point end_time_;
		std::chrono::steady_clock::duration time_out_duration_;
		bool has_time_out_;
		static constexpr uint16_t max_time_out_counter_ = 512;
		mutable uint16_t time_out_counter_ = 0;
		/* query level caches */
		// maps a graph to an operator type
		mutable boost::container::flat_map<size_t, Operation> odg_operator_type_;
		mutable boost::container::flat_map<size_t, std::vector<size_t>> odg_projected_vars_positions_;
		mutable boost::container::flat_map<size_t, bool> odg_contains_projected_vars_;


	public:
		Query() = delete;

		explicit Query(OperandDependencyGraph odg,
					   std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> operands,
					   std::vector<char> proj_vars,
					   std::chrono::steady_clock::time_point const &end_time = std::chrono::steady_clock::time_point::max())
			: odg_(std::move(odg)),
			  operands_(std::move(operands)),
			  proj_vars_(std::move(proj_vars)),
			  start_time_(std::chrono::steady_clock::now()),
			  end_time_(end_time),
			  time_out_duration_(end_time_ - start_time_),
			  has_time_out_(end_time_ != std::chrono::steady_clock::time_point::max()) {
			for (std::size_t i = 0; i < proj_vars_.size(); i++) {
				proj_vars_pos_[proj_vars_[i]] = i;
			}
		}

		[[nodiscard]] OperandDependencyGraph &operand_dependency_graph() {
			return odg_;
		}

		[[nodiscard]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands() const {
			return operands_;
		}

		[[nodiscard]] std::vector<char> const &projected_vars() const {
			return proj_vars_;
		}

		[[nodiscard]] std::size_t projected_var_position(char var) const {
			assert(proj_vars_pos_.contains(var));
			return proj_vars_pos_.find(var)->second;
		}

		[[nodiscard]] boost::container::flat_map<char, std::size_t> const &projected_vars_positions() const noexcept {
			return proj_vars_pos_;
		}

		[[nodiscard]] bool contains_proj_var(char var) const {
			return proj_vars_pos_.contains(var);
		}

		[[nodiscard]] Operation &get_odg_operator_type(OperandDependencyGraph &graph) const {
			return odg_operator_type_[graph.identifier()];
		}

		[[nodiscard]] std::vector<size_t> const &get_odg_projected_vars_positions(OperandDependencyGraph &graph) const {
			auto &projected_vars_positions = odg_projected_vars_positions_[graph.identifier()];
			if (not projected_vars_positions.empty())
				return projected_vars_positions;
			auto const &odg_vars_set = graph.operands_var_ids_set();
			for (auto const &[var, pos] : proj_vars_pos_) {
				if (odg_vars_set.contains(var))
					projected_vars_positions.push_back(pos);
			}
			return projected_vars_positions;
		}

		[[nodiscard]] bool all_result_done(OperandDependencyGraph &graph) const {
			auto g_id = graph.identifier();
			if (odg_contains_projected_vars_.contains(g_id))
				return odg_contains_projected_vars_[g_id];
			auto &all_res_done = odg_contains_projected_vars_[g_id];
			auto const &vars_in_operands = graph.operands_var_ids_set();
			all_res_done = std::all_of(proj_vars_.begin(), proj_vars_.end(),
									   [&](char proj_var) { return not vars_in_operands.contains(proj_var); });
			return all_res_done;
		}

		void check_time_out() const {
			if (has_time_out_ and time_out_counter_++ > max_time_out_counter_) {
				if (std::chrono::steady_clock::now() < end_time_) [[likely]] {
					time_out_counter_ = 0;
				} else {
					throw std::runtime_error("Query timed out after " +
											 std::to_string(std::chrono::duration_cast<std::chrono::seconds>(time_out_duration_).count()) +
											 "seconds.");
				}
			}
		}
	};

}// namespace dice::query

#endif//QUERY_QUERY_HPP
