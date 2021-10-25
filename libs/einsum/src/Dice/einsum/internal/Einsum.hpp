#ifndef HYPERTRIE_EINSUM_HPP
#define HYPERTRIE_EINSUM_HPP

#include <tsl/sparse_set.h>
#include <utility>

#include <Dice/hypertrie/Hypertrie_trait.hpp>

#include "Dice/einsum/internal/CartesianOperator.hpp"
#include "Dice/einsum/internal/Context.hpp"
#include "Dice/einsum/internal/CountOperator.hpp"
#include "Dice/einsum/internal/EntryGeneratorOperator.hpp"
#include "Dice/einsum/internal/JoinOperator.hpp"
#include "Dice/einsum/internal/Operator.hpp"
#include "Dice/einsum/internal/ResolveOperator.hpp"

namespace Dice::einsum::internal {

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued tr>
	std::shared_ptr<Operator<value_type, tr>>
	Operator<value_type, tr>::construct(const std::shared_ptr<Subscript> &subscript,
										const std::shared_ptr<Context> &context) {
		switch (subscript->type) {
			case Subscript::Type::Join:
				return std::make_shared<JoinOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::Resolve:
				return std::make_shared<ResolveOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::Count:
				return std::make_shared<CountOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::Cartesian:
				return std::make_shared<CartesianOperator<value_type, tr>>(subscript, context);
			case Subscript::Type::EntryGenerator:
				return std::make_shared<EntryGeneratorOperator<value_type, tr>>(subscript, context);
			default:
				throw std::invalid_argument{"subscript is of an undefined type."};
		}
	}

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued tr_t>
	class Einsum {
		using tr = tr_t;

		using Operator_t = Operator<value_type, tr>;
		using Entry_t = Entry<value_type, tr>;
		using Key_t = Key<value_type, tr>;
		using key_part_type = typename tr::key_part_type;


		std::shared_ptr<Subscript> subscript{};
		std::shared_ptr<Context> context{};
		std::vector<hypertrie::const_Hypertrie<tr>> operands{};
		std::shared_ptr<Operator_t> op{};
		Entry_t entry{};


	public:
		Einsum() = default;

		Einsum(std::shared_ptr<Subscript> subscript, const std::vector<hypertrie::const_Hypertrie<tr>> &operands,
			   TimePoint timeout = TimePoint::max())
			: subscript(std::move(subscript)), context{std::make_shared<Context>(timeout)},
			  operands(operands),
			  op{Operator_t::construct(this->subscript, context)},
			  entry(Entry_t::make_filled(this->subscript->resultLabelCount(), Operator_t::default_key_part)) {}

		[[nodiscard]] const std::shared_ptr<Subscript> &getSubscript() const {
			return subscript;
		}

		const std::vector<hypertrie::const_Hypertrie<tr>> &getOperands() const {
			return operands;
		}

		const std::shared_ptr<Operator_t> &getOp() const {
			return op;
		}

		struct iterator {
		private:
			std::shared_ptr<Operator_t> op;
			Entry_t *current_entry;
			bool ended_ = false;

		public:
			iterator() = default;

			explicit iterator(Einsum &einsum, Entry_t &entry) : op(einsum.op), current_entry{&entry} {}

			iterator &operator++() {
				op->next();
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
			op->load(operands, entry);
			return iterator{*this, entry};
		}

		[[nodiscard]] bool end() const {
			return false;
		}

		void clear() {
			op->clear();
		}
	};

	template<hypertrie::HypertrieTrait_bool_valued tr_t>
	class Einsum<bool, tr_t> {
		using tr = tr_t;
		using value_type = bool;

		using Operator_t = Operator<value_type, tr>;
		using Entry_t = Entry<bool, tr>;
		using key_part_type = typename tr::key_part_type;

		std::shared_ptr<Subscript> subscript{};
		std::shared_ptr<Context> context{};
		std::vector<hypertrie::const_Hypertrie<tr>> operands{};
		std::shared_ptr<Operator_t> op{};
		Entry_t entry{};

	public:
		Einsum(std::shared_ptr<Subscript> subscript, const std::vector<hypertrie::const_Hypertrie<tr>> &operands,
			   TimePoint timeout = std::numeric_limits<TimePoint>::max())
			: subscript(std::move(subscript)), context{std::make_shared<Context>(timeout)},
			  operands(operands),
			  op{Operator_t::construct(this->subscript, context)},
			  entry(Entry_t::make_filled(this->subscript->resultLabelCount(), Operator_t::default_key_part)) {}

		[[nodiscard]] const std::shared_ptr<Subscript> &getSubscript() const {
			return subscript;
		}

		const std::vector<hypertrie::const_Hypertrie<tr>> &getOperands() const {
			return operands;
		}

		const std::shared_ptr<Operator_t> &getOp() const {
			return op;
		}

		struct iterator {
		private:
			std::shared_ptr<Operator_t> op;
			tsl::sparse_set<size_t, std::identity> found_entries{};
			Entry_t *current_entry;
			bool ended_ = false;

		public:
			iterator() = default;

			explicit iterator(Einsum &einsum, Entry_t &entry) : op(einsum.op), current_entry{&entry} {
				if (not op->ended()) {
					const size_t hash = Dice::hash::DiceHashxxh3<decltype(current_entry->key())>()(current_entry->key());
					found_entries.insert(hash);
				}
			}

			iterator &operator++() {
				op->next();
				while (not op->ended()) {
					assert(current_entry->value() == true);
					const size_t hash = Dice::hash::DiceHashxxh3<decltype(current_entry->key())>()(current_entry->key());
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
			op->load(operands, entry);
			return iterator{*this, entry};
		}

		[[nodiscard]] bool end() const {
			return false;
		}

		void clear() {
			op->clear();
		}
	};
}// namespace Dice::einsum::internal
#endif//HYPERTRIE_EINSUM_HPP
