#ifndef HYPERTRIE_SINGLEKEY_HPP
#define HYPERTRIE_SINGLEKEY_HPP

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/RawKey.hpp>

namespace Dice::hypertrie::internal::raw {

	/**
 * A super class to provide a single key in a Node.
 * @tparam depth depth of the key
 * @tparam tri HypertrieInternalTrait that defines node parameters
 */
	template<size_t depth, HypertrieCoreTrait tri>
	struct SingleKey {
		using RawKey_t = RawKey<depth, tri>;

	protected:
		RawKey_t key_;

	public:
		/**
		 * Default constructor fills the key with 0, 0.0, true (value initialization)
		 */
		SingleKey() noexcept : key_{} {}

		/**
		 * Uses the provided RawKey_t as key.
		 * @param key
		 */
		explicit SingleKey(RawKey_t key) noexcept : key_(key) {}

		/**
		 * Modifiable reference to key.
		 * @return
		 */
		RawKey_t &key() noexcept { return this->key_; }

		/**
		 * Constant reference to key.
		 * @return
		 */
		const RawKey_t &key() const noexcept { return this->key_; }

		/**
		 * Size of this node (It is always 1).
		 * @return
		 */
		[[nodiscard]] constexpr size_t size() const noexcept { return 1; }
	};

}// namespace Dice::hypertrie::internal::raw
#endif//HYPERTRIE_SINGLEKEY_HPP
