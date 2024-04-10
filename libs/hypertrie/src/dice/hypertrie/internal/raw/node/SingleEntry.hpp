#ifndef HYPERTRIE_ENTRY_HPP
#define HYPERTRIE_ENTRY_HPP

#include <cstddef>

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/ReferenceCounted.hpp"
#include "dice/hypertrie/internal/raw/node/SingleKey.hpp"
#include "dice/hypertrie/internal/raw/node/Valued.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t>
	class SingleEntry : public SingleKey<depth, htt_t>, public Valued<htt_t> {
	public:
		using RawKey_t = RawKey<depth, htt_t>;
		using value_type = typename htt_t::value_type;

		SingleEntry() : SingleKey<depth, htt_t>() {}

		SingleEntry(const RawKey_t &key, value_type value) noexcept
			: SingleKey<depth, htt_t>(key), Valued<htt_t>(value) {}

		auto operator<=>(const SingleEntry &other) const noexcept {
			return std::tie(this->key(), this->value()) <=> std::tie(other.key(), other.value());
		}

		bool operator==(const SingleEntry &other) const noexcept {
			return std::tie(this->key(), this->value()) == std::tie(other.key(), other.value());
		}
	};

	template<size_t depth, HypertrieTrait_bool_valued htt_t>
	class SingleEntry<depth, htt_t> : public SingleKey<depth, htt_t> {
	public:
		using RawKey_t = RawKey<depth, htt_t>;

		SingleEntry() = default;

		/**
		 * Constructor
		 *
		 * @param key
		 * @param value Needed so this constructor has the same form as the other SingleEntry constructor.
		 */
		explicit SingleEntry(const RawKey_t &key, [[maybe_unused]] bool value = true) noexcept
			: SingleKey<depth, htt_t>(key) {}

		[[nodiscard]] constexpr bool value() const noexcept { return true; }

		auto operator<=>(const SingleEntry &other) const noexcept {
			return this->key() <=> other.key();
		}

		bool operator==(const SingleEntry &other) const noexcept {
			return this->key() == other.key();
		}
	};


}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_ENTRY_HPP
