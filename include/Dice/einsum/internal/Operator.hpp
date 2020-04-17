#ifndef HYPERTRIE_OPERATOR_HPP
#define HYPERTRIE_OPERATOR_HPP

#include <memory>
#include <utility>
#include "Dice/einsum/internal/Subscript.hpp"
#include "Dice/einsum/internal/Entry.hpp"
#include "Dice/einsum/internal/Context.hpp"
#include "Dice/einsum/internal/CardinalityEstimation.hpp"

namespace einsum::internal {

	class Context;


	template<typename value_type, typename key_part_type, template<typename, typename> class map_type,
			template<typename> class set_type, template<typename, template<typename, typename> class map_type_a,
            template<typename> class set_type_a> class const_BoolHypertrie, typename Diagonal>
	class Operator {
	protected:
		constexpr static const bool bool_value_type = std::is_same_v<value_type, bool>;
		using const_BoolHypertrie_t = const_BoolHypertrie<key_part_type, map_type, set_type>;
		typedef Operator<value_type, key_part_type, map_type, set_type, const_BoolHypertrie, Diagonal> Operator_t;
		using CardinalityEstimation_t = CardinalityEstimation<key_part_type, map_type, set_type, const_BoolHypertrie>;
		static constexpr key_part_type default_key_part = []() {
			if constexpr (std::is_pointer_v<key_part_type>) return nullptr;
			else return std::numeric_limits<key_part_type>::max();
		}();
	public:
		mutable Subscript::Type type = Subscript::Type::None;
	protected:
		std::shared_ptr<Subscript> subscript;
		std::shared_ptr<Context> context;
		Entry <key_part_type, value_type> *entry;

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
		void (*load_fp)(void *self, std::vector<const_BoolHypertrie<key_part_type, map_type, set_type>> operands,
						Entry<key_part_type, value_type> &entry);

		std::size_t (*hash_fp)(const void *self);

	protected:
		template<typename T>
		Operator(Subscript::Type type,
				 std::shared_ptr<Subscript> subscript,
				 std::shared_ptr<Context> context,
				 [[maybe_unused]] const T * dummy)
				: type(type),
				  subscript(std::move(subscript)),
				  context(std::move(context)),
				  next_fp(&T::next),
				  ended_fp(&T::ended),
				  load_fp(&T::load),
				  hash_fp(&T::hash) {}

	public:

		static std::shared_ptr<Operator>  construct(const std::shared_ptr<Subscript> &subscript, const std::shared_ptr<Context> &context);
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

		void load(std::vector<const_BoolHypertrie<key_part_type, map_type, set_type>> operands,
				  Entry<key_part_type, value_type> &entry) {
			load_fp(this, std::move(operands), entry);
		}

		std::size_t hash() const { return hash_fp(this); }

		bool operator!=(const Operator &other) const { return hash() != other.hash(); };

	};


}

#endif //HYPERTRIE_OPERATOR_HPP

