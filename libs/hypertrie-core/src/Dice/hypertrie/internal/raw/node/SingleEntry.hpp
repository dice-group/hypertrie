#ifndef HYPERTRIE_ENTRY_HPP
#define HYPERTRIE_ENTRY_HPP

#include <cstddef>

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/ReferenceCounted.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleKey.hpp>
#include <Dice/hypertrie/internal/raw/node/Valued.hpp>

namespace hypertrie::internal::raw {

	template<size_t depth, HypertrieCoreTrait tri_t>
	class SingleEntry : public SingleKey<depth, tri_t>, public Valued<tri_t> {
	public:
		using tri = tri_t;
		using RawKey = RawKey<depth, tri_t>;
		using value_type = typename tri::value_type;

		SingleEntry() = default;

		SingleEntry(const RawKey &key, value_type value, size_t ref_count = 0) noexcept
			: ReferenceCounted(ref_count), SingleEntry<depth, tri_t>(key), Valued<tri_t>(value) {}

		auto operator<=>(const SingleEntry &other) const noexcept {
			return std::tie(this->key(), this->value()) <=> std::tie(other.key(), other.value());
		}

		auto operator==(const SingleEntry &other) const noexcept {
			return std::tie(this->key(), this->value()) == std::tie(other.key(), other.value());
		}
	};

	template<size_t depth, HypertrieCoreTrait_bool_valued tri_t>
	class SingleEntry<depth, tri_t> : public SingleKey<depth, tri_t> {
	public:
		using tri = tri_t;
		using RawKey = RawKey<depth, tri_t>;

		SingleEntry() noexcept = default;

		explicit SingleEntry(const RawKey &key, size_t ref_count = 0) noexcept
			: SingleKey<depth, tri_t>(key, ref_count) {}

		[[nodiscard]] constexpr bool value() const noexcept { return true; }

		auto operator<=>(const SingleEntry &other) const noexcept {
			return this->key() <=> other.key();
		}

		auto operator==(const SingleEntry &other) const noexcept {
			return this->key() == other.key();
		}
	};

}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_ENTRY_HPP
