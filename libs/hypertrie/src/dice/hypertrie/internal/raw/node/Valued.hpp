#ifndef HYPERTRIE_VALUED_HPP
#define HYPERTRIE_VALUED_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"

namespace dice::hypertrie::internal::raw {

	/**
	* A super class to provide a single value in a Node.
	* @tparam htt_t HypertrieTrait that defines node parameters
	*/
	template<HypertrieTrait htt_t>
	struct Valued {
		static_assert(not std::is_same_v<typename htt_t::value_type, bool>, "The value of Boolean valued nodes should not be stored explicitly.");
		using value_type = typename htt_t::value_type;

	protected:
		value_type value_{};

	public:
		/**
		 * Default constructor sets value to 0, 0.0, true (value initialization)
		 */
		Valued() = default;

		/**
		 * Uses the provided value.
		 * @param key
		 */
		explicit Valued(value_type value) noexcept : value_(value) {}

		/**
		 * Modifiable reference to value.
		 * @return
		 */
		[[nodiscard]] value_type &value() noexcept { return this->value_; }

		/**
		 * Constant reference to value.
		 * @return
		 */
		[[nodiscard]] const value_type &value() const noexcept { return this->value_; }
	};
}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_VALUED_HPP
