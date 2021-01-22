#ifndef HYPERTRIE_OPERATOR_HPP
#define HYPERTRIE_OPERATOR_HPP

#include <memory>
#include <utility>
#include "Dice/einsum/internal/Entry.hpp"
#include "Dice/einsum/internal/Subscript.hpp"
#include "Dice/einsum/internal/Context.hpp"
#include "Dice/einsum/internal/CardinalityEstimation.hpp"

namespace einsum::internal {

//	class Context<key_part_type>;


	template<typename value_type, HypertrieTrait tr_t>
	class Operator {
	public:
		using tr = tr_t;
		// TODO: check if we still need all of that
		constexpr static const bool bool_value_type = std::is_same_v<value_type, bool>;
		typedef Operator<value_type, tr> Operator_t;
		using CardinalityEstimation_t = CardinalityEstimation<tr>;
		using key_part_type = typename tr::key_part_type;
		static constexpr key_part_type default_key_part = []() {
			if constexpr (std::is_pointer_v<key_part_type>) return nullptr;
			else return std::numeric_limits<key_part_type>::max();
		}();
		mutable Subscript::Type type = Subscript::Type::None;
	protected:
		std::shared_ptr<Subscript> subscript;
		std::shared_ptr<Context<key_part_type>> context;
		Entry <value_type, tr> *entry;

		/**
		 * Pointer to the next Function of the operator implementation.
		 * @param self pointer to the actual operator instance
		 * @return the next Entry. Afterwards the Iterator is automatically forwarded.
		 */
		void (*next_fp)(void *self);

		/**
		 * Pointer to the ended Function of the operator implementation.
		 * @param self actual operator instance
		 * @return if iteration ended
		 */
		bool (*ended_fp)(const void *self);

		/**
		 * Pointer to the load Function of the operator implementation.
		 * @param self actual operator instance
		 * @param operands operands to be loaded
		 */
		void (*load_fp)(void *self, std::vector<const_Hypertrie<tr>> operands,
						Entry<value_type, tr> &entry);

		void (*clear_fp)(void *self);

	protected:
		template<typename T>
		Operator(Subscript::Type type,
				 std::shared_ptr<Subscript> subscript,
				 std::shared_ptr<Context<key_part_type>> context,
				 [[maybe_unused]] const T * dummy)
				: type(type),
				  subscript(std::move(subscript)),
				  context(std::move(context)),
				  next_fp(&T::next),
				  ended_fp(&T::ended),
				  load_fp(&T::load),
				  clear_fp(&T::clear){}

	public:

		static std::shared_ptr<Operator>  construct(const std::shared_ptr<Subscript> &subscript,
												    const std::shared_ptr<Context<key_part_type>> &context);
		Operator() = default;

		Operator(Operator &) = default;

		Operator(Operator &&) noexcept = default;

		Operator &operator=(Operator &&op) noexcept = default;

		/**
			 * This is only a stub to fulfill the C++ iterator interface.
			 * The iterator is frowarded by using the operator* or by calling value().
			 * @return reference to self.
			 */
		inline Operator &operator++() {
			next();
			return *this;
		}

		/**
		 * Returns the next entry and forwards the iterator. equal to operator*
		 * @return
		 */
		inline void next() { next_fp(this); }

		/**
		 * Is true as long as there are more entrys retrievable via operator* or value.
		 * @return
		 */
		inline operator bool() const { return not ended_fp(this); }

		/**
		 * Returns true if the iteration is at its end.
		 * @return
		 */
		inline bool ended() const { return ended_fp(this); }


		Operator &begin() { return *this; }

		bool end() { return false; }

		void load(std::vector<const_Hypertrie<tr>> operands,
				  Entry<value_type, tr> &entry) {
			load_fp(this, std::move(operands), entry);
		}

		std::size_t hash() const { return subscript->hash(); }

		void clear() { clear_fp(this); }

		bool operator!=(const Operator &other) const { return hash() != other.hash(); };

		const std::shared_ptr<Subscript> &getSubscript() { return this->subscript; }
	};


}

#endif //HYPERTRIE_OPERATOR_HPP

