#ifndef HYPERTRIE_SINGLEKEY_HPP
#define HYPERTRIE_SINGLEKEY_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/RawKey.hpp"

namespace dice::hypertrie::internal::raw {

	/**
 * A super class to provide a single key in a Node.
 * @tparam depth depth of the key
 * @tparam htt_t HypertrieTrait that defines node parameters
 */
	template<size_t depth, HypertrieTrait htt_t>
	struct SingleKey {
		using RawKey_t = RawKey<depth, htt_t>;

	protected:
		RawKey_t key_;

	public:
		/**
		 * Default constructor fills the key with 0, 0.0, true (value initialization)
		 */
		SingleKey() = default;

		/**
		 * Uses the provided RawKey_t as key.
		 * @param key
		 */
		explicit SingleKey(RawKey_t key) noexcept : key_(key) {}

		/**
		 * Modifiable reference to key.
		 * @return
		 */
		[[nodiscard]] RawKey_t &key() noexcept { return this->key_; }

		/**
		 * Constant reference to key.
		 * @return
		 */
		[[nodiscard]] const RawKey_t &key() const noexcept { return this->key_; }

		/**
		 * Size of this node (It is always 1).
		 * @return
		 */
		[[nodiscard]] constexpr size_t size() const noexcept { return 1; }
	};

}// namespace dice::hypertrie::internal::raw
#endif//HYPERTRIE_SINGLEKEY_HPP
