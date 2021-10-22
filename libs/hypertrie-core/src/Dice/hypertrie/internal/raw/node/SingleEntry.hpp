#ifndef HYPERTRIE_ENTRY_HPP
#define HYPERTRIE_ENTRY_HPP

#include <cstddef>

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/ReferenceCounted.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleKey.hpp>
#include <Dice/hypertrie/internal/raw/node/Valued.hpp>

namespace Dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieCoreTrait tri_t>
	class SingleEntry : public SingleKey<depth, tri_t>, public Valued<tri_t> {
	public:
		using tri = tri_t;
		using RawKey_t = RawKey<depth, tri_t>;
		using value_type = typename tri::value_type;

		SingleEntry() = default;

		template<HypertrieCoreTrait_bool_valued tri2>
		SingleEntry(SingleEntry<depth, tri2> const& entry) noexcept : SingleKey<depth, tri_t>(entry.key()),  Valued<tri_t>(entry.value()){}

		SingleEntry(const RawKey_t &key, value_type value) noexcept
			: SingleKey<depth, tri_t>(key), Valued<tri_t>(value) {}

		auto operator<=>(const SingleEntry &other) const noexcept {
			return std::tie(this->key(), this->value()) <=> std::tie(other.key(), other.value());
		}

		bool operator==(const SingleEntry &other) const noexcept {
			return std::tie(this->key(), this->value()) == std::tie(other.key(), other.value());
		}
	};

	template<size_t depth, HypertrieCoreTrait_bool_valued tri_t>
	class SingleEntry<depth, tri_t> : public SingleKey<depth, tri_t> {
	public:
		using tri = tri_t;
		using RawKey_t = RawKey<depth, tri_t>;

		SingleEntry() = default;

		template<HypertrieCoreTrait_bool_valued tri2>
		SingleEntry(SingleEntry<depth, tri2> const& entry) noexcept  : SingleKey<depth, tri_t>(entry.key()){}


		explicit SingleEntry(const RawKey_t &key, [[maybe_unused]] bool value = true) noexcept
			: SingleKey<depth, tri_t>(key) {}

		[[nodiscard]] constexpr bool value() const noexcept { return true; }

		auto operator<=>(const SingleEntry &other) const noexcept {
			return this->key() <=> other.key();
		}

		bool operator==(const SingleEntry &other) const noexcept {
			return this->key() == other.key();
		}
	};


}// namespace Dice::hypertrie::internal::raw

#endif//HYPERTRIE_ENTRY_HPP
