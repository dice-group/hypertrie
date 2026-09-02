#ifndef HYPERTRIE_ENTRY_HPP
#define HYPERTRIE_ENTRY_HPP

#include <cstddef>

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/ReferenceCounted.hpp"
#include "dice/hypertrie/internal/raw/node/SingleKey.hpp"
#include "dice/hypertrie/internal/raw/node/Valued.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth_, HypertrieTrait htt_t_>
	class SingleEntry : public SingleKey<depth_, htt_t_>, public Valued<htt_t_> {
	public:
		static constexpr size_t depth = depth_;
		using htt_t = htt_t_;
		using RawKey_t = RawKey<depth, htt_t>;

		SingleEntry() : SingleKey<depth, htt_t>() {}

		SingleEntry(const RawKey_t &key, typename htt_t::value_type value) noexcept
			: SingleKey<depth, htt_t>(key), Valued<htt_t>(value) {}

		constexpr auto operator<=>(const SingleEntry &other) const noexcept = default;
	};

	template<size_t depth_, HypertrieTrait_bool_valued htt_t_>
	class SingleEntry<depth_, htt_t_> : public SingleKey<depth_, htt_t_> {
	public:
		static constexpr size_t depth = depth_;
		using htt_t = htt_t_;
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

	template<typename T>
	struct is_SingleEntry : std::false_type {
	};

	template<size_t depth, HypertrieTrait htt_t>
	struct is_SingleEntry<SingleEntry<depth, htt_t>> : std::true_type {
	};

	template<typename T>
	inline constexpr bool is_SingleEntry_v = is_SingleEntry<T>::value;

}// namespace dice::hypertrie::internal::raw

namespace dice::hash {
	template<typename Policy, size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct dice_hash_overload<Policy, ::dice::hypertrie::internal::raw::SingleEntry<depth, htt_t>> {
		static std::size_t dice_hash(::dice::hypertrie::internal::raw::SingleEntry<depth, htt_t> const &entry) noexcept {
			using H = dice_hash_templates<Policy>;
			return Policy::hash_combine({H::dice_hash(entry.key()), H::dice_hash(entry.value())});
		}
	};
}// namespace dice::hash

#endif//HYPERTRIE_ENTRY_HPP
