#ifndef SPARSETENSOR_EINSUM_SUBSCRIPT_HPP
#define SPARSETENSOR_EINSUM_SUBSCRIPT_HPP

#include "dice/einsum/internal/RawSubscript.hpp"
#include "dice/einsum/internal/util/UndirectedGraph.hpp"

#include <robin_hood.h>

#include <algorithm>
#include <memory>
#include <numeric>
#include <ostream>
#include <tuple>
#include <vector>

namespace dice::einsum {

	// TODO: needs a reader-writer lock for its cache
	/**
	 * Representation of the subscript of a expression in einstein summation convention.
	 * This provides also  bracketing of independently computable parts and resulting in a
	 * cross product of the bracketed parts.
	 */
	class Subscript {
	public:
		using DependencyGraph = internal::util::UndirectedGraph<internal::Label>;
		using ConnectedComponent = typename DependencyGraph::NodeSet;
		using ConnectedComponents = std::vector<ConnectedComponent>;

		using OperandPos = internal::OperandPos;
		using CartesianOperandPos = OperandPos;
		using OriginalOperandPos = OperandPos;
		using OriginalResultPos = OperandPos;
		using OriginalOperandPoss = std::vector<OriginalOperandPos>;
		using OriginalResultPoss = std::vector<OriginalResultPos>;
		using LabelPossInOperand = internal::LabelPossInOperand;
		using LabelPossInOperands = internal::LabelPossInOperands;
		using LabelPos = internal::LabelPos;
		using OperandsSc = internal::OperandsSc;
		using OperandSc = internal::OperandSc;
		using ResultSc = internal::ResultSc;

		enum class Type {
			None = 0,
			Join,
			Cartesian,
			Resolve,
			Count,
			EntryGenerator,
			CarthesianMapping
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

			CartesianSubSubscripts(Subscript const &original_subscript) {
				internal::OperandsSc operands_labels{};
				for (auto const &connected_component : original_subscript.connected_components) {
					auto [cartesian_sub_subscript, original_op_poss, original_result_poss] =
							extractCartesianSubSubscript(original_subscript, connected_component);
					operands_labels.push_back(cartesian_sub_subscript->raw_subscript.result);
					sub_subscripts.emplace_back(std::move(cartesian_sub_subscript));
					original_operand_poss_of_sub_subscript.emplace_back(std::move(original_op_poss));
					original_result_poss_of_sub_subscript.emplace_back(std::move(original_result_poss));
				}
				internal::ResultSc result_labels = original_subscript.raw_subscript.result;
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
			extractCartesianSubSubscript(Subscript const &subscripts, ConnectedComponent const &label_subset) {
				internal::OperandsSc operands_labels{};
				OriginalOperandPoss original_op_poss{};
				OriginalResultPoss original_result_poss{};
				for (size_t parent_op_pos = 0; parent_op_pos < subscripts.raw_subscript.operands.size(); ++parent_op_pos) {
					auto const &parent_op_labels = subscripts.raw_subscript.operands[parent_op_pos];
					internal::OperandSc op_labels{};
					for (Label const label : parent_op_labels) {
						if (label_subset.count(label)) {
							op_labels.emplace_back(label);
						}
					}
					if (not op_labels.empty()) {
						operands_labels.push_back(op_labels);
						original_op_poss.push_back(parent_op_pos);
					}
				}
				internal::ResultSc result_labels{};
				for (size_t result_pos = 0; result_pos < subscripts.raw_subscript.result.size(); ++result_pos) {
					char &label = subscripts.raw_subscript.result[result_pos];
					if (label_subset.count(label)) {
						result_labels.emplace_back(label);
						original_result_poss.push_back(result_pos);
					}
				}
				return {std::make_shared<Subscript>(operands_labels, result_labels),
						original_op_poss,
						original_result_poss};
			}

			std::size_t size() const noexcept {
				return sub_subscripts.size();
			}
		};


	private:
		mutable robin_hood::unordered_map<Label, std::shared_ptr<Subscript>> sub_subscripts{};

		internal::RawSubscript raw_subscript{};

		robin_hood::unordered_set<Label> operands_label_set{};

		robin_hood::unordered_set<Label> result_label_set{};

		robin_hood::unordered_set<Label> lonely_non_result_labels{};

		DependencyGraph dependency_graph{};

		ConnectedComponents connected_components{};

		LabelPossInOperand used_result_poss_{};

		// Join
		mutable robin_hood::unordered_map<Label, LabelPossInOperands> label_poss_in_operands{};
		// Join & resolve
		mutable robin_hood::unordered_map<Label, LabelPos> label_poss_in_result{};
		// Resolve
		LabelPossInOperand operand2result_mapping_resolveType{};

		// Join
		robin_hood::unordered_map<Label, std::vector<OperandPos>> poss_of_operands_with_labels{};

		// Cartesian
		CartesianSubSubscripts cartesian_sub_subscripts;

	public:
		std::shared_ptr<Subscript> removeLabel(Label label) const noexcept {
			auto iterator = sub_subscripts.find(label);
			if (iterator != sub_subscripts.end()) {
				return iterator->second;
			} else {
				return sub_subscripts.insert({label, std::make_shared<Subscript>(raw_subscript.removeLabel(label))})
						.first->second;
			}
		}

		robin_hood::unordered_set<Label> const &getLonelyNonResultLabelSet() const noexcept {
			return lonely_non_result_labels;
		}

		robin_hood::unordered_set<Label> const &getOperandsLabelSet() const noexcept {
			return operands_label_set;
		}

		robin_hood::unordered_set<Label> const &getResultLabelSet() const noexcept {
			return result_label_set;
		}

		std::vector<OperandPos> const &getPossOfOperandsWithLabel(Label label) const {
			auto iterator = poss_of_operands_with_labels.find(label);
			if (iterator != poss_of_operands_with_labels.end()) {
				return iterator->second;
			}
			throw std::invalid_argument("label is not used in operands.");
		}


		/**
		 * for Join
		 * @param label
		 * @return
		 */
		const LabelPossInOperands &getLabelPossInOperands(const Label label) const noexcept {
			assert(operands_label_set.count(label));
			auto iterator = label_poss_in_operands.find(label);
			if (iterator != label_poss_in_operands.end()) {
				return iterator->second;
			} else {
				return label_poss_in_operands.insert({label, raw_subscript.getLabelPossInOperands(label)})
						.first->second;
			}
		}

		bool isResultLabel(const Label label) const noexcept {
			return result_label_set.count(label);
		}

		LabelPos getLabelPosInResult(const Label label) const {
			auto iterator = label_poss_in_result.find(label);
			if (iterator != label_poss_in_result.end()) {
				return iterator->second;
			}
			throw std::invalid_argument("label is not in result.");
		}

		[[nodiscard]] auto operandsCount() const noexcept {
			return raw_subscript.operandsCount();
		}

		[[nodiscard]] auto labelCount(OperandPos const operand_pos) const {
			return raw_subscript.labelCount(operand_pos);
		}

		[[nodiscard]] auto resultLabelCount() const noexcept {
			return raw_subscript.resultLabelCount();
		}

		[[nodiscard]] auto getOperandLabels(OperandPos const label_pos) const {
			assert(operandsCount() > label_pos);
			return raw_subscript.operands[label_pos];
		}

		auto operand2resultMapping_ResolveType() const noexcept {
			assert(type == Type::Resolve);
			return operand2result_mapping_resolveType;
		}

		const LabelPossInOperand &getUsedResultPoss() const noexcept {
			return used_result_poss_;
		}

		Type type;
		mutable bool all_result_done;

		Subscript() = default;

		Subscript(const Subscript &) = default;

		Subscript(Subscript &) = default;

		Subscript(Subscript &&) = default;

		Subscript &operator=(Subscript const &) = default;

		Subscript &operator=(Subscript &) = default;

		Subscript(std::string const &subscript_str) : Subscript(from_string(subscript_str)) {}

		bool calcAllResultDone(robin_hood::unordered_set<Label> const &operand_labels,
							   robin_hood::unordered_set<Label> const &result_labels) {
			return std::none_of(result_labels.begin(), result_labels.end(),
								[&](auto label){ return operand_labels.count(label);});
		}

		Subscript(internal::RawSubscript raw_subscript, Type type = Type::None)
			: raw_subscript(raw_subscript),
			  operands_label_set(raw_subscript.getOperandsLabelSet()),
			  result_label_set(raw_subscript.getResultLabelSet()),
			  dependency_graph(calcDependencyGraph(raw_subscript)),
			  connected_components(dependency_graph.getConnectedComponents()),
			  used_result_poss_{[&]() {
				  LabelPossInOperand used_result_poss;
				  used_result_poss.reserve(raw_subscript.resultLabelCount());
				  for (size_t i = 0; i < raw_subscript.resultLabelCount(); ++i) {
					  if (operands_label_set.contains(raw_subscript.result[i])) {
						  used_result_poss.push_back(i);
					  }
				  }
				  return used_result_poss;
			  }()},
			  type((type == Type::CarthesianMapping) ? Type::CarthesianMapping : calcState(raw_subscript, operands_label_set, result_label_set, connected_components)),
			  all_result_done(calcAllResultDone(operands_label_set, result_label_set)) {

			for (size_t op_pos = 0; op_pos < raw_subscript.operands.size(); ++op_pos) {
				for (const Label label : raw_subscript.operands[op_pos]) {
					poss_of_operands_with_labels[label].push_back(op_pos);
				}
			}

			for (auto label : operands_label_set) {
				if (not result_label_set.count(label)) {
					const auto &op_poss = poss_of_operands_with_labels[label];
					if (op_poss.size() == 1) {
						const auto &op = raw_subscript.operands[op_poss[0]];
						if (std::count(op.begin(), op.end(), label) == 1) {
							lonely_non_result_labels.insert(label);
						}
					}
				}
			}

			switch (this->type) {
				case Type::Join:
					label_poss_in_result = raw_subscript.getLabelPossInResult();
					break;
				case Type::Cartesian:
					cartesian_sub_subscripts = {*this};
					break;
				case Type::Resolve:
					label_poss_in_result = raw_subscript.getLabelPossInResult();
					operand2result_mapping_resolveType.reserve(resultLabelCount());
					for (auto label : getOperandLabels(0)) {
						operand2result_mapping_resolveType.emplace_back(getLabelPosInResult(label));
					}
					break;
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


		Subscript(OperandsSc operands, ResultSc result, Type type = Type::None) : Subscript{internal::RawSubscript{std::move(operands), std::move(result)}, type} {}

		std::string to_string() const noexcept {
			return static_cast<std::string>(this->raw_subscript);
		}

		static Subscript from_string(std::string const &subscript_str) {
			auto iter = subscript_str.cbegin();
			auto end = subscript_str.end();
			robin_hood::unordered_map<char, Label> char_mapping{};
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
				if (*iter == '-') {
					break;
				}
				++iter;
			}
			if (operands_sc.empty()) {
				operands_sc.emplace_back();
			}
			iter = iter + 2;
			while (iter != end) {
				assert(char_mapping.count(*iter));
				result_sc.push_back(char_mapping[*iter]);
				++iter;
			}
			return {std::move(operands_sc), std::move(result_sc)};
		}

		internal::RawSubscript const &getRawSubscript() const {
			return raw_subscript;
		}

	public:
		CartesianSubSubscripts const &getCartesianSubscript() const {
			return cartesian_sub_subscripts;
		}

	private:
		Type calcState(internal::RawSubscript const &raw_subscript,
					   robin_hood::unordered_set<Label> const &operands_label_set,
					   robin_hood::unordered_set<Label> const &result_label_set,
					   ConnectedComponents const &connected_components) {
			switch (raw_subscript.operandsCount()) {
				case 0:
					return Type::EntryGenerator;
				case 1:
					// single operand
					if (auto operand_label_count = raw_subscript.labelCount(0);
						operand_label_count == operands_label_set.size()) { // no label occurs multiple times and thus there is no diagonal
						// check if  all operand labels are either in result or all are not in result
						// none of both may also be the case
						bool all_in_result = true;
						bool none_in_result = true;
						for (auto label : operands_label_set) {
							if (result_label_set.count(label)) {
								none_in_result = false;
							} else {
								all_in_result = false;
							}
						}
						if (all_in_result) {
							return Type::Resolve;
						} else if (none_in_result) {
							return Type::Count;
						}
					}
					break;
				default:
					// multiple operands
					if (connected_components.size() > 1) {
						// more than one connected component means that there is a Cartesian product
						return Type::Cartesian;
					}
			}
			return Type::Join;
		}

		static DependencyGraph calcDependencyGraph(internal::RawSubscript const &raw_subscript) {
			DependencyGraph label_dependency_graph{};
			for (auto const &operand : raw_subscript.operands) {
				label_dependency_graph.addCompleteGraph(operand);
			}
			return label_dependency_graph;
		}

	public:
		/**
		 * Speedup at the price of a neglectable risk of a cash collision.
		 * @param other
		 * @return
		 */
		bool operator!=(Subscript const &other) const { return hash() != other.hash(); };

		[[nodiscard]] inline size_t hash() const { return raw_subscript.hash; }

		[[nodiscard]] bool valid() const {
			auto &toIter = raw_subscript.result;
			return std::all_of(toIter.begin(), toIter.end(), [&](auto label){return operands_label_set.count(label);});
		}
	};

	using CartesianSubSubscripts = Subscript::CartesianSubSubscripts;
}// namespace dice::einsum

inline std::ostream &operator<<(std::ostream &stream, const std::shared_ptr<::dice::einsum::Subscript> &sub_script) {
	stream << sub_script->to_string();
	return stream;
}

inline std::ostream &operator<<(std::ostream &stream, ::dice::einsum::Subscript const *const sub_script) {
	stream << sub_script->to_string();
	return stream;
}

inline std::ostream &operator<<(std::ostream &stream, const ::dice::einsum::Subscript &sub_script) {
	stream << sub_script.to_string();
	return stream;
}


template<>
struct std::hash<::dice::einsum::Subscript> {
	size_t operator()(const ::dice::einsum::Subscript &s) const { return s.hash(); }
};

#endif//SPARSETENSOR_EINSUM_SUBSCRIPT_HPP
