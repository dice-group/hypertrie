#ifndef SPARSETENSOR_EINSUM_SUBSCRIPT_HPP
#define SPARSETENSOR_EINSUM_SUBSCRIPT_HPP


#include <vector>
#include <tuple>
#include <map>
#include <numeric>
#include <algorithm>
#include <memory>
#include <ostream>

#include <boost/container/flat_map.hpp>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include "Dice/einsum/internal/util/DirectedGraph.hpp"
#include "Dice/einsum/internal/util/UndirectedGraph.hpp"
#include "Dice/einsum/internal/RawSubscript.hpp"


#include <vector>
#include <tuple>
#include <boost/container_hash/hash.hpp>
#include <tsl/hopscotch_set.h>
#include <tsl/hopscotch_map.h>

namespace einsum::internal {

	using DependencyGraph = util::UndirectedGraph<Label>;
	using DirectedDependencyGraph = util::DirectedGraph<Label>;
	using ConnectedComponent = typename DependencyGraph::NodeSet;
	using ConnectedComponents = std::vector<ConnectedComponent>;

	using WeaklyConnectedComponent = std::set<Label>;
	using WeaklyConnectedComponents = std::vector<WeaklyConnectedComponent>;

	using SubOperatorPos = std::uint8_t;

	using CartesianOperandPos = OperandPos;
	using OriginalOperandPos = OperandPos;
	using OriginalResultPos = OperandPos;
	using OriginalOperandPoss = std::vector<OriginalOperandPos>;
	using OriginalResultPoss = std::vector<OriginalResultPos>;

	/**
	 * Representation of the subscript of a expression in einstein summation convention.
	 * This provides also  bracketing of independently computable parts and resulting in a
	 * cross product of the bracketed parts.
	 */
	class Subscript {
	public:
		enum class Type {
			None = 0, Join, LeftJoin, JoinSelection, Cartesian, Resolve, Count, EntryGenerator, CarthesianMapping
		};
		using Label = char;

		class CartesianSubSubscripts {

		private:
			mutable std::vector<std::shared_ptr<Subscript>> sub_subscripts;
			mutable std::vector<OriginalOperandPoss> original_operand_poss_of_sub_subscript;
			mutable std::vector<OriginalResultPoss> original_result_poss_of_sub_subscript;
			mutable std::shared_ptr<Subscript> subscript;

		public:
			CartesianSubSubscripts() = default;

			CartesianSubSubscripts(const Subscript &original_subscript) {
				OperandsSc operands_labels{};
				for (const auto &weakly_connected_component: original_subscript.weakly_connected_components) {
					auto[cartesian_sub_subscript, original_op_poss, original_result_poss] =
					extractCartesianSubSubscript(original_subscript, weakly_connected_component);
					operands_labels.push_back(cartesian_sub_subscript->raw_subscript.result);
					sub_subscripts.emplace_back(std::move(cartesian_sub_subscript));
					original_operand_poss_of_sub_subscript.emplace_back(std::move(original_op_poss));
					original_result_poss_of_sub_subscript.emplace_back(std::move(original_result_poss));
				}
				ResultSc result_labels = original_subscript.raw_subscript.result;
				subscript = std::make_shared<Subscript>(operands_labels, result_labels, Type::CarthesianMapping);
			}

			const std::shared_ptr<Subscript> &getSubscript() const {
				return subscript;
			}

			const std::vector<std::shared_ptr<Subscript>> &getSubSubscripts() const {
				return sub_subscripts;
			}

			const OriginalOperandPoss &getOriginalOperandPoss(CartesianOperandPos cart_op_pos) const {
				return original_operand_poss_of_sub_subscript[cart_op_pos];
			}

			const std::vector<OriginalResultPoss> &getOriginalResultPoss() const {
				return original_result_poss_of_sub_subscript;
			}

		private:
			static std::tuple<std::shared_ptr<Subscript>, OriginalOperandPoss, OriginalResultPoss>
			extractCartesianSubSubscript(const Subscript &subscripts, const WeaklyConnectedComponent &label_subset) {
				OperandsSc operands_labels{};
				OriginalOperandPoss original_op_poss{};
				OriginalResultPoss original_result_poss{};
				auto& poss_in_op = subscripts.getRawSubscript().poss_in_operands;
				const auto& [opt_begin, opt_end] = subscripts.getRawSubscript().optional_brackets;
				for (const auto &[parent_op_pos, parent_op_labels] :
						iter::enumerate(subscripts.raw_subscript.original_operands)) {
					if(parent_op_labels == opt_begin || parent_op_labels == opt_end) {
						operands_labels.push_back(parent_op_labels);
						continue;
					}
					OperandSc op_labels{};
					for (const Label label : parent_op_labels)
						if (label_subset.count(label))
							op_labels.emplace_back(label);

					if (not op_labels.empty()) {
                        operands_labels.push_back(op_labels);
                        original_op_poss.push_back(poss_in_op[parent_op_pos]);
					}
				}
				ResultSc result_labels{};
				for (const auto[result_pos, label] : iter::enumerate(subscripts.raw_subscript.result))
					if (label_subset.count(label)) {
						result_labels.emplace_back(label);
						original_result_poss.push_back(result_pos);
					}

				return {std::make_shared<Subscript>(operands_labels, result_labels, subscripts.type),
				        original_op_poss,
				        original_result_poss};
			}

			std::size_t size() const noexcept {
				return sub_subscripts.size();
			}
		};


	private:
		mutable tsl::hopscotch_map<Label, std::shared_ptr<Subscript>> sub_subscripts{};

		RawSubscript raw_subscript{};

		tsl::hopscotch_set<Label> operands_label_set{};

		tsl::hopscotch_set<Label> result_label_set{};

		tsl::hopscotch_set<Label> lonely_non_result_labels{};

		DependencyGraph dependency_graph{};

		DirectedDependencyGraph directed_dependency_graph{};

		ConnectedComponents connected_components{};

		WeaklyConnectedComponents weakly_connected_components{};

		// Join
		mutable tsl::hopscotch_map<Label, LabelPossInOperands> label_poss_in_operands{};
		// Join & resolve
		mutable tsl::hopscotch_map<Label, LabelPos> label_poss_in_result{};
		// Resolve
		LabelPossInOperand operand2result_mapping_resolveType{};
		// Join
		tsl::hopscotch_map<Label, std::vector<OperandPos>> poss_of_operands_with_labels{};
		// Join
        tsl::hopscotch_set<Label> join_labels{};
		// Cartesian
		CartesianSubSubscripts cartesian_sub_subscripts;
		// Cartesian
        std::vector<std::vector<OperandPos>> sub_op_dependencies;
		// Left Join
        tsl::hopscotch_set<Label> left_join_labels{};
		// Left Join
        tsl::hopscotch_map<Label, std::vector<OperandPos>> non_optional_operands_of_label{};
		// Recursive Left Join
		std::vector<OperandPos> non_optional_operands{};

	public:
		std::shared_ptr<Subscript> removeLabel(Label label) const {
			auto iterator = sub_subscripts.find(label);
			if (iterator != sub_subscripts.end())
				return iterator->second;
			else
                return sub_subscripts.insert(
                                {label, std::make_shared<Subscript>(raw_subscript.removeLabel(label), this->type)})
                        .first->second;
		}

		std::shared_ptr<Subscript> removeOperands(const std::vector<OperandPos>& operands_poss) {
			return std::make_shared<Subscript>(raw_subscript.removeOperands(operands_poss), this->type);
		}

        std::shared_ptr<Subscript> removeLabels(const std::set<Label>& labels) {
            return std::make_shared<Subscript>(raw_subscript.removeLabels(labels), this->type);
        }

		const tsl::hopscotch_set<Label> &getLonelyNonResultLabelSet() const {
			return lonely_non_result_labels;
		}

		const tsl::hopscotch_set<Label> &getOperandsLabelSet() const {
			return operands_label_set;
		}

		const tsl::hopscotch_set<Label> &getResultLabelSet() const {
			return result_label_set;
		}

		const std::vector<OperandPos> &getPossOfOperandsWithLabel(Label label) const {
			auto iterator = poss_of_operands_with_labels.find(label);
			if (iterator != poss_of_operands_with_labels.end())
				return iterator->second;
			else
				throw std::invalid_argument("label is not used in operands.");
		}

		auto getDependentOperands(OperandPos operand_position) {
			return directed_dependency_graph.transitivelyGetNeighborsLabelled(operand_position);
		}

        std::vector<std::vector<OperandPos>>& getSubOperatorDependencies() {
			return sub_op_dependencies;
		}

        auto getSubOperatorOfOperands() {
            return directed_dependency_graph.getWeakComponentsOfVertices();
        }

		/**
		 * for Join
		 * @param label
		 * @return
		 */
		const LabelPossInOperands &getLabelPossInOperands(const Label label) const {
			assert(operands_label_set.count(label));
			auto iterator = label_poss_in_operands.find(label);
			if (iterator != label_poss_in_operands.end())
				return iterator->second;
			else
				return label_poss_in_operands.insert(
								{label, raw_subscript.getLabelPossInOperands(label)})
						.first->second;
		}

		bool isResultLabel(const Label label) const noexcept {
			return result_label_set.count(label);
		}

		LabelPos getLabelPosInResult(const Label label) const {
			auto iterator = label_poss_in_result.find(label);
			if (iterator != label_poss_in_result.end())
				return iterator->second;
			else
				throw std::invalid_argument("label is not in result.");
		}

		[[nodiscard]] auto operandsCount() const noexcept {
			return raw_subscript.operandsCount();
		}

		[[nodiscard]] auto labelCount(const OperandPos operand_pos) const {
			return raw_subscript.labelCount(operand_pos);
		}

		[[nodiscard]] auto resultLabelCount() const noexcept {
			return raw_subscript.resultLabelCount();
		}

		[[nodiscard]] auto getOperandLabels(const OperandPos label_pos) const {
			assert(operandsCount() > label_pos);
			return raw_subscript.operands[label_pos];
		}

		auto operand2resultMapping_ResolveType() const noexcept {
			assert(type == Type::Resolve);
			return operand2result_mapping_resolveType;
		}

	public:
		Type type;
		mutable bool all_result_done;

		Subscript() = default;

		Subscript(const Subscript &) = default;

		Subscript(Subscript &) = default;

		Subscript(Subscript &&) = default;

		Subscript &operator=(const Subscript &) = default;

		Subscript &operator=(Subscript &) = default;

		Subscript(const std::string &subscript_str) : Subscript(from_string(subscript_str)) {}

		bool calcAllResultDone(const tsl::hopscotch_set<Label> &operand_labels,
		                       const tsl::hopscotch_set<Label> &result_labels) {
			for (auto result_label : result_labels) {
				if (operand_labels.count(result_label))
					return false;
			}
			return true;
		}

		Subscript(RawSubscript raw_subscript, Type type = Type::None)
				: raw_subscript(raw_subscript),
				  operands_label_set(raw_subscript.getOperandsLabelSet()),
				  result_label_set(raw_subscript.getResultLabelSet()),
                  directed_dependency_graph(calcDirectedDependencyGraph(raw_subscript)),
                  weakly_connected_components(directed_dependency_graph.getWeaklyConnectedComponents()),
                  type((type == Type::CarthesianMapping) ? Type::CarthesianMapping : calcState(raw_subscript,
                                                                                               operands_label_set,
                                                                                               result_label_set,
                                                                                               weakly_connected_components)),
                  all_result_done(calcAllResultDone(operands_label_set, result_label_set)) {

			for (const auto [op_pos, labels] : iter::enumerate(raw_subscript.operands))
				for (const Label label : labels)
					poss_of_operands_with_labels[label].push_back(op_pos);

			for (auto label : operands_label_set) {
				if (not result_label_set.count(label)) {
					const auto &op_poss = poss_of_operands_with_labels[label];
					if (op_poss.size() == 1) {
						const auto &op = raw_subscript.operands[op_poss[0]];
						if (std::count(op.begin(), op.end(), label) == 1)
							lonely_non_result_labels.insert(label);
					}
				}
			}

			switch (this->type) {
				case Type::JoinSelection: {
					[[fallthrough]];
				}
				case Type::LeftJoin: {
                    label_poss_in_result = raw_subscript.getLabelPossInResult();

                    for (const auto[op_pos, labels] : iter::enumerate(raw_subscript.operands))
                        for (const Label label : labels)
							poss_of_operands_with_labels[label].push_back(op_pos);
                    auto independent_strong_component = directed_dependency_graph.getIndependentStrongComponent();
					// find the left join labels -> outgoing labels
					// store the non-optional operands of each label
					for(const auto& out_label : independent_strong_component.outgoing_labels) {
						left_join_labels.insert(out_label);
						for(const auto& [op, op_out_labels] : independent_strong_component.vertices_out_edges_labels) {
							if(op_out_labels.find(out_label) != op_out_labels.end()) {
								auto& non_opt_ops_of_label = non_optional_operands_of_label[out_label];
								non_opt_ops_of_label.push_back(op);
							}
						}
					}
					if(this->type != Type::JoinSelection)
					    break;
					[[fallthrough]]; // in case of JoinSelection visit Join as well
				}
				case Type::Join: {
					label_poss_in_result = raw_subscript.getLabelPossInResult();

					break;
				}

				case Type::Cartesian: {
                    label_poss_in_result = raw_subscript.getLabelPossInResult();
					cartesian_sub_subscripts = {*this};
					// find dependencies between sub_operators
					// store for each operand to which sub_operator it belongs
					std::map<OperandPos, SubOperatorPos> operand_sub_op_map{};
					for(auto sub_op_pos : iter::range(cartesian_sub_subscripts.getSubSubscripts().size())) {
						for(auto orig_op_pos : cartesian_sub_subscripts.getOriginalOperandPoss(sub_op_pos)) {
							operand_sub_op_map[orig_op_pos] = sub_op_pos;
						}
					}
                    sub_op_dependencies.resize(cartesian_sub_subscripts.getSubSubscripts().size());
					// use weak dependencies (unlabelled edges) between operands to find dependencies between sub_operators
					for(auto operand_pos : iter::range(raw_subscript.operands.size())) {
						auto source_sub_op = operand_sub_op_map[operand_pos];
                        auto& dependent_sub_ops = sub_op_dependencies[source_sub_op];
						for(auto weakly_dependent_operand : directed_dependency_graph.transitivelyGetNeighborsUnlabelled(operand_pos)) {
							dependent_sub_ops.push_back(operand_sub_op_map[weakly_dependent_operand]);
						}
					}
					break;
				}
				case Type::Resolve: {
					label_poss_in_result = raw_subscript.getLabelPossInResult();

					operand2result_mapping_resolveType.reserve(resultLabelCount());
					for (auto label: getOperandLabels(0))
						operand2result_mapping_resolveType.emplace_back(
								getLabelPosInResult(label));
					break;
				}
				case Type::Count:
					break;
				case Type::EntryGenerator:
					break;
				case Type::None:
					break;
				case Type::CarthesianMapping:
					break;
			}
		}


		Subscript(OperandsSc operands, ResultSc result, Type type = Type::None) : Subscript{
				RawSubscript{operands, std::move(result)}, type} {}

		std::string to_string() const {
			std::vector<std::string> operand_strings;
			operand_strings.reserve(raw_subscript.original_operands.size());
			for (const auto &operand : raw_subscript.original_operands) {
				operand_strings.push_back(fmt::format("{}", fmt::join(operand, "")));
			}
			return fmt::format("{}->{}", fmt::join(operand_strings, ","), fmt::join(raw_subscript.result, ""));
		}

		static Subscript from_string(const std::string &subscript_str) {
			auto iter = subscript_str.cbegin();
			auto end = subscript_str.end();
			boost::container::flat_map<char, Label> char_mapping{};
			OperandsSc operands_sc{};
			ResultSc result_sc{};
			Label next_label = 'a';

			while (*iter != '-') {
				OperandSc &operand_sc = operands_sc.emplace_back();
				while (*iter != ',' and *iter != '-') {
					if (not char_mapping.count(*iter)) {
						char_mapping[*iter] = next_label++;
					}
					operand_sc.push_back(char_mapping[*iter]);
					++iter;
				}
				if (*iter == '-')
					break;
				++iter;
			}
			if (operands_sc.empty())
				operands_sc.emplace_back();
			iter = iter + 2;
			while (iter != end) {
				assert(char_mapping.count(*iter));
				result_sc.push_back(char_mapping[*iter]);
				++iter;
			}
			return {std::move(operands_sc), std::move(result_sc)};
		}

		const tsl::hopscotch_set<Label>& getLeftJoinLabels() const {
			return left_join_labels;
		}

        const tsl::hopscotch_set<Label>& getJoinLabels() const {
            return join_labels;
        }

		const std::vector<OperandPos>& getNonOptionalOperands(Label label) {
			return non_optional_operands_of_label[label];
		}

        const std::vector<OperandPos>& getNonOptionalOperands() {
            return non_optional_operands;
        }

		const RawSubscript &getRawSubscript() const {
			return raw_subscript;
		}

	public:
		const CartesianSubSubscripts &getCartesianSubscript() const {
			return cartesian_sub_subscripts;
		}

	private:
		Type calcState(const RawSubscript &raw_subscript,
		               const tsl::hopscotch_set<Label> &operands_label_set,
		               const tsl::hopscotch_set<Label> &result_label_set,
		               const WeaklyConnectedComponents &weakly_connected_components) {
			switch (raw_subscript.operandsCount()) {
				case 0:
					return Type::EntryGenerator;
				case 1:
					// single operand
					if (auto operand_label_count = raw_subscript.labelCount(0);
							operand_label_count == operands_label_set.size()) {
						// at least one label occurs multiple times and thus there is a diagonal
						// check if  all operand labels are either in result or all are not in result
						// none of both may also be the case
						bool all_in_result = true;
						bool none_in_result = true;
						for (auto label : operands_label_set) {
							if (result_label_set.count(label))
								none_in_result = false;
							else
								all_in_result = false;
						}
						if (all_in_result)
							return Type::Resolve;
						else if (none_in_result)
							return Type::Count;
					}
					break;
				default: {
					// multiple operands
                    // more than one WEAKLY connected component means that there is a Cartesian product
					if (weakly_connected_components.size() > 1) {
						return Type::Cartesian;
					}
					break;
				}
			}
			// use the strongly connected components of the dependency graph in order to choose the proper join operation
			// the strong components of a graph create a directed acyclic graph (independent strong component)
			auto ind_strong_comp = directed_dependency_graph.getIndependentStrongComponent();
			// if the independent component contains multiple operands do a join
			// if there are multiple operands in the independent component, component_labels will not be empty
			join_labels.insert(ind_strong_comp.component_labels.begin(), ind_strong_comp.component_labels.end());
			left_join_labels.insert(ind_strong_comp.outgoing_labels.begin(), ind_strong_comp.outgoing_labels.end());
			tsl::hopscotch_set<Label> common_labels{};
            for(const auto& op_labels_pair : ind_strong_comp.vertices_out_edges_labels)
                non_optional_operands.emplace_back(op_labels_pair.first);
			if(join_labels.size() > 0) {
				// store non-optional operands
                std::set_intersection(join_labels.begin(), join_labels.end(),
                                      left_join_labels.begin(), left_join_labels.end(),
									  std::inserter(common_labels, common_labels.end()));
				// empty common labels -> there is not a label that participates in join and left join -> join
				if(common_labels.empty()) {
					return Type::Join;
				}
				// common labels not empty && component labels > 0
				// multiple join labels and at least one label that participates in join and left join
				if(join_labels.size() > 1 and !common_labels.empty()) {
					return Type::JoinSelection;
				}
			}
			// if the independent component contains only one operand do a left join
            // if there are multiple operands in the independent component, component_labels will be empty
			return Type::LeftJoin;
		}

		static DependencyGraph calcDependencyGraph(const RawSubscript &raw_subscript) {
			DependencyGraph label_dependency_graph{};
			for (const auto &operand : raw_subscript.operands)
				label_dependency_graph.addCompleteGraph(operand);
			return label_dependency_graph;
		}

		static DirectedDependencyGraph calcDirectedDependencyGraph(const RawSubscript &raw_subscript) {
			using Depth = uint8_t;
            DirectedDependencyGraph operand_directed_dependency_graph{};
			// for each label stores the position of the last observed operand at each depth
			std::map<Label, std::map<Depth, OperandPos>> label_depth_operand{};
			// for each label stores the position of the last observed operand and its depth
			std::map<Label, OperandPos> last_operand_of_label{};
			const auto& [opt_begin, opt_end] = raw_subscript.optional_brackets;
            Depth depth{0};
			// original operand labels
			const auto& orig_operands = raw_subscript.original_operands;
			// the position of the previous operand
			OperandPos prev_operand_pos = std::numeric_limits<OperandPos>::max();
			// the depth of the previous operand
			Depth prev_operand_depth;
			// the position of the last non-optional operand
			OperandPos last_non_opt_op_pos = std::numeric_limits<OperandPos>::max();
            for(const auto& [orig_op_pos, operand_labels] : iter::enumerate(orig_operands)) {
                if(operand_labels == opt_begin) {
                    depth++;
                    continue;
                }
                else if(operand_labels == opt_end) {
					// remove entries of this depth from the label_depth_operand map (removes completed optional groups)
					for(auto& label_map : label_depth_operand)
						label_map.second.erase(depth);
                    depth--;
                    continue;
                }
				operand_directed_dependency_graph.addVertex();
				// the position of the operand in the operands vector
				auto operand_pos = raw_subscript.poss_in_operands[orig_op_pos];
                bool strong_dependency = false; // indicates whether the current operand participates in a strong dependency
				for(auto label : operand_labels) {
					// check if the active label has been observed
					if(label_depth_operand.contains(label) and !label_depth_operand[label].empty()) {
						// check if there is an operand with the same label in the current group (join)
						if (label_depth_operand[label].contains(depth)) {
							auto dominant_op_orig_pos = label_depth_operand[label][depth];
							auto dominant_op_pos = raw_subscript.poss_in_operands[dominant_op_orig_pos];
							operand_directed_dependency_graph.addEdge(label, dominant_op_pos, operand_pos);
							operand_directed_dependency_graph.addEdge(label, operand_pos, dominant_op_pos);
                            strong_dependency = true;
						}
						// check if there is an operand with the same label on the upper level / group (left-join)
						else if (label_depth_operand[label].contains(depth - 1)) {
							auto dominant_op_orig_pos = label_depth_operand[label][depth - 1];
							auto dominant_op_pos = raw_subscript.poss_in_operands[dominant_op_orig_pos];
							operand_directed_dependency_graph.addEdge(label, dominant_op_pos, operand_pos);
                            strong_dependency = true;
						}
					}
                    // the pattern is not well-designed -> weakly well-designed
                    else if(last_operand_of_label.contains(label)) {
                        auto dominant_op_orig_pos = last_operand_of_label[label];
                        auto dominant_op_pos = raw_subscript.poss_in_operands[dominant_op_orig_pos];
                        // wwd edge
                        operand_directed_dependency_graph.addEdge(label, dominant_op_pos, operand_pos, true);
                        strong_dependency = true;
					}
                    // if the current label has not been seen yet, add a self-edge using the current label
					// needed for cartesian join
					else
                        operand_directed_dependency_graph.addEdge(label, operand_pos, operand_pos);
					label_depth_operand[label][depth] = orig_op_pos;
					last_operand_of_label[label] = orig_op_pos;
                }
				// find weak dependencies. for cartesian
				for(int8_t d = depth; d >= 0; d--) {
					std::set<OperandPos> operands_at_depth{};
					for(const auto &label_entry : label_depth_operand) {
						if(label_entry.second.contains(d))
							if(raw_subscript.poss_in_operands[label_entry.second.at(d)] != operand_pos)
							    operands_at_depth.insert(label_entry.second.at(d));
					}
					if(operands_at_depth.empty())
						continue;
					auto last_pos_of_depth = raw_subscript.poss_in_operands[*operands_at_depth.rbegin()]; // set is ordered take its last value
                    operand_directed_dependency_graph.addEdge(last_pos_of_depth, operand_pos);
					if(d == depth)
                        operand_directed_dependency_graph.addEdge(operand_pos, last_pos_of_depth);
					break;
				}
//                if(!strong_dependency and prev_operand_pos < std::numeric_limits<OperandPos>::max()) {
//					if(raw_subscript.original_operands[prev_operand_pos] == raw_subscript.original_operands[orig_op_pos-1]) {
//                        operand_directed_dependency_graph.addEdge(operand_pos, raw_subscript.poss_in_operands[prev_operand_pos]);
//                        operand_directed_dependency_graph.addEdge(raw_subscript.poss_in_operands[prev_operand_pos], operand_pos);
//					}
//					else if(prev_operand_depth < depth) {
//						operand_directed_dependency_graph.addEdge(raw_subscript.poss_in_operands[prev_operand_pos], operand_pos);
//					}
//					if(last_non_opt_op_pos < std::numeric_limits<OperandPos>::max() and last_non_opt_op_pos != prev_operand_pos) {
//						operand_directed_dependency_graph.addEdge(last_non_opt_op_pos, operand_pos);
//						if(depth == 0)
//                            operand_directed_dependency_graph.addEdge(operand_pos, last_non_opt_op_pos);
//					}
//                }
                if(depth == 0)
                    last_non_opt_op_pos = operand_pos;
				prev_operand_pos = orig_op_pos;
				prev_operand_depth = depth;
			}
			return operand_directed_dependency_graph;
		}

	public:
		bool operator!=(const Subscript &other) const { return hash() != other.hash(); };
		// TODO: estimate the risk of a hash collision
		// bool operator!=(const Subscript &other) const { return this->raw_subscript != other.raw_subscript; };

		[[nodiscard]] inline size_t hash() const { return raw_subscript.hash; }

		[[nodiscard]] bool valid() const {
			for (auto result_label : raw_subscript.result)
				if (not operands_label_set.count(result_label))
					return false;
			return true;
		}


	};

	using CartesianSubSubscripts = Subscript::CartesianSubSubscripts;
}

inline std::ostream &operator<<(std::ostream &stream, const std::shared_ptr<einsum::internal::Subscript> &sub_script) {
	stream << sub_script->to_string();
	return stream;
}

inline std::ostream &operator<<(std::ostream &stream, std::shared_ptr<einsum::internal::Subscript> sub_script) {
	stream << sub_script->to_string();
	return stream;
}

inline std::ostream &operator<<(std::ostream &stream, einsum::internal::Subscript const *const sub_script) {
	stream << sub_script->to_string();
	return stream;
}

inline std::ostream &operator<<(std::ostream &stream, const einsum::internal::Subscript &sub_script) {
	stream << sub_script.to_string();
	return stream;
}

namespace fmt {
	template<>
	struct formatter<std::shared_ptr<einsum::internal::Subscript>> {
		template<typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template<typename FormatContext>
		auto format(const std::shared_ptr<einsum::internal::Subscript> &sub_script, FormatContext &ctx) {
			return format_to(ctx.out(), "{}", sub_script->to_string());
		}
	};
}


template<>
struct std::hash<einsum::internal::Subscript> {
	size_t operator()(const einsum::internal::Subscript &s) const { return s.hash(); }
};

#endif //SPARSETENSOR_EINSUM_SUBSCRIPT_HPP


