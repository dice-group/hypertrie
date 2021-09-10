#ifndef HYPERTRIE_EINSUM_HPP
#define HYPERTRIE_EINSUM_HPP

#include <tsl/hopscotch_set.h>
#include <utility>

#include "Dice/einsum/internal/CartesianOperator.hpp"
#include "Dice/einsum/internal/Context.hpp"
#include "Dice/einsum/internal/CountOperator.hpp"
#include "Dice/einsum/internal/EntryGeneratorOperator.hpp"
#include "Dice/einsum/internal/JoinOperator.hpp"
#include "Dice/einsum/internal/JoinSelectionOperator.hpp"
#include "Dice/einsum/internal/LeftJoinOperator.hpp"
#include "Dice/einsum/internal/Operator.hpp"
#include "Dice/einsum/internal/RecursiveLeftJoinOperator.hpp"
#include "Dice/einsum/internal/ResolveOperator.hpp"
#include "Dice/einsum/internal/UnionOperator.hpp"

namespace einsum::internal {

	template<typename value_type, HypertrieTrait tr>
	std::shared_ptr<Operator<value_type, tr>>
	Operator<value_type, tr>::construct(const std::shared_ptr<Subscript> &subscript,
										const std::shared_ptr<Context<key_part_type>> &context) {
		switch (subscript->type) {
			case Subscript::Type::Join:
				return std::make_shared<JoinOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::LeftJoin:
				return std::make_shared<LeftJoinOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::RecursiveLeftJoin:
				return std::make_shared<RecursiveLeftJoinOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::JoinSelection:
				return std::make_shared<JoinSelectionOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::Resolve:
				return std::make_shared<ResolveOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::Count:
				return std::make_shared<CountOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::Cartesian:
				if (not context->union_)
					return std::make_shared<CartesianOperator<value_type, tr>>(subscript, context);
				[[fallthrough]];
			case Subscript::Type::Union:
				return std::make_shared<UnionOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::EntryGenerator:
				return std::make_shared<EntryGeneratorOperator<value_type, tr>>(subscript, context);
			default:
				throw std::invalid_argument{"subscript is of an undefined type."};
		}
	}

	template<typename value_type, HypertrieTrait tr_t>
	class Einsum {
		using tr = tr_t;

		using Operator_t = Operator<value_type, tr>;
		using Entry_t = Entry<value_type, tr>;
		using key_part_type = typename tr::key_part_type;

		std::shared_ptr<Subscript> subscript;
		std::shared_ptr<Context<key_part_type>> context;
		std::vector<const_Hypertrie<tr>> operands;
		std::shared_ptr<Operator_t> op;
		Entry_t entry;


	public:
		Einsum() = default;

		Einsum(const std::shared_ptr<Subscript> &subscript, const std::vector<const_Hypertrie<tr>> &operands,
			   TimePoint timeout = TimePoint::max(), bool union_ = false)
			: context(std::make_shared<Context<key_part_type>>(timeout, union_)),
			  entry(subscript->resultLabelCount(), Operator_t::default_key_part) {
			// check for empty operands
			tsl::hopscotch_set<std::size_t> prune_pos{};
			for (const auto &[pos, operand] : iter::enumerate(operands)) {
				if (operand.empty()) {
					prune_pos.insert(pos);
					const auto &dependent_operands = subscript->getDependentOperands(pos);
					if (not dependent_operands.empty()) {
						prune_pos.insert(dependent_operands.begin(), dependent_operands.end());
					} else {
						// weakly dependencies -- appear in cartesian products
						const auto &weakly_dependent_operands = subscript->getWeaklyDependentOperands(pos);
						prune_pos.insert(weakly_dependent_operands.begin(), weakly_dependent_operands.end());
						// operands will be pruned via weak dependencies -- need to prune their dependent operands
						for (const auto &op_pos : weakly_dependent_operands) {
							const auto &dep_ops = subscript->getDependentOperands(op_pos);
							prune_pos.insert(dep_ops.begin(), dep_ops.end());
						}
					}
				}
			}
			if (not prune_pos.empty()) {
				std::vector<const_Hypertrie<tr>> operands_after_pruning{};
				std::vector<std::vector<Label>> operands_labels_after_pruning{};
				// populate new operands vector
				for (const auto &[pos, operand] : iter::enumerate(operands)) {
					if (not prune_pos.contains(pos)) {
						operands_after_pruning.push_back(std::move(operand));
					}
				}
				// populate new operands' labels vector
				uint8_t pos = 0;
				for (const auto &operand_labels : subscript->getRawSubscript().original_operands) {
					if (operand_labels == std::vector<Label>{'['} or operand_labels == std::vector<Label>{']'}) {
						operands_labels_after_pruning.emplace_back(operand_labels);
						continue;
					}
					if (not prune_pos.contains(pos))
						operands_labels_after_pruning.emplace_back(operand_labels);
					pos += 1;
				}
				this->subscript = std::make_shared<Subscript>(operands_labels_after_pruning,
															  subscript->getRawSubscript().result);
				this->operands = std::move(operands_after_pruning);
			} else {
				this->subscript = subscript;
				this->operands = operands;
			}
			this->op = Operator_t::construct(this->subscript, this->context);
			this->context = std::make_shared<Context<key_part_type>>(timeout, union_);
		}

		[[nodiscard]] const std::shared_ptr<Subscript> &getSubscript() const {
			return subscript;
		}

		const std::vector<const_Hypertrie<tr>> &getOperands() const {
			return operands;
		}

		const std::shared_ptr<Operator_t> &getOp() const {
			return op;
		}

		struct iterator {
		private:
			std::shared_ptr<Operator_t> op;
			Context<key_part_type> *ctx;
			Entry_t *current_entry;
			bool ended_ = false;
            tsl::sparse_set<size_t, std::identity> found_entries{};

		public:
			iterator() = default;

            template<typename T = value_type> requires (not std::is_same_v<T, bool>)
			explicit iterator(Einsum &einsum, Entry_t &entry) : op(einsum.op),
																ctx(einsum.context.get()),
																current_entry(&entry) {}

            template<typename T = value_type> requires (std::is_same_v<T, bool>)
            explicit iterator(Einsum &einsum, Entry_t &entry) : op(einsum.op), current_entry{&entry} {
                if (not op->ended()) {
                    const size_t hash = Dice::hash::dice_hash(current_entry->key);
                    found_entries.insert(hash);
                }
            }

            template<typename T = value_type> requires (not std::is_same_v<T, bool>)
			iterator &operator++() {
				op->next();
				return *this;
			}

            template<typename T = value_type> requires (std::is_same_v<T, bool>)
            iterator &operator++() {
                op->next();
                while (not op->ended()) {
                    assert(current_entry->value == true);
                    const size_t hash = Dice::hash::dice_hash(current_entry->key);
                    if (not found_entries.contains(hash)) {
                        found_entries.insert(hash);
                        return *this;
                    }
                    op->next();
                }
                return *this;
            }

			inline const Entry<value_type, tr> &operator*() {
				return *current_entry;
			}

			inline const Entry<value_type, tr> &value() {
				return *current_entry;
			}

			operator bool() const {
				return not op->ended();
			}

			[[nodiscard]] inline bool ended() const { return op->ended(); }
		};

		iterator begin() {
			if (not operands.empty()) {
				op->load(operands, entry);
			}
			return iterator{*this, entry};
		}

		[[nodiscard]] bool end() const {
			return false;
		}

		void clear() {
			op->clear();
		}

	};

}// namespace einsum::internal
#endif//HYPERTRIE_EINSUM_HPP
