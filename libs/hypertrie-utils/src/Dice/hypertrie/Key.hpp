#ifndef HYPERTRIE_KEY_HPP
#define HYPERTRIE_KEY_HPP

#include <Dice/hash/DiceHash.hpp>

#include <Dice/hypertrie/Hypertrie_trait.hpp>

#include <algorithm>
#include <optional>
#include <vector>

namespace Dice::hypertrie {
	template<HypertrieTrait tr>
	class Key : public ::std::vector<typename tr::key_part_type> {
		using super_t = ::std::vector<typename tr::key_part_type>;

	public:
		Key() = default;
		explicit Key(typename super_t::size_type n, const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(n, a) {}

		Key(typename super_t::size_type n, const typename super_t::value_type &value,
			const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(n, value, a){};

		Key(super_t &&rv, const typename super_t::allocator_type &m) noexcept
			: super_t(std::move(rv), m, typename super_t::_Alloc_traits::is_always_equal{}) {}

		Key(std::initializer_list<typename super_t::value_type> l,
			const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(l, a) {}

		template<typename InputIterator,
				 typename = std::_RequireInputIter<InputIterator>>
		Key(InputIterator first, InputIterator last,
			const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(first, last, a) {}
	};

	template<HypertrieTrait tr>
	class SliceKey : public ::std::vector<::std::optional<typename tr::key_part_type>> {
		using super_t = ::std::vector<::std::optional<typename tr::key_part_type>>;

	public:
		SliceKey() = default;
		explicit SliceKey(typename super_t::size_type n, const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(n, a) {}

		SliceKey(typename super_t::size_type n, const typename super_t::value_type &value,
				 const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(n, value, a){};

		SliceKey(super_t &&rv, const typename super_t::allocator_type &m) noexcept
			: super_t(std::move(rv), m, typename super_t::_Alloc_traits::is_always_equal{}) {}

		SliceKey(std::initializer_list<typename super_t::value_type> l,
				 const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(l, a) {}

		template<typename InputIterator,
				 typename = std::_RequireInputIter<InputIterator>>
		SliceKey(InputIterator first, InputIterator last,
				 const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(first, last, a) {}

	public:
		[[nodiscard]] size_t get_fixed_depth() const noexcept {
			return this->size() - std::ranges::count_if(*this, [](auto const &item) { return not item.has_value(); });
		}
	};

	template<HypertrieTrait tr>
	class NonZeroEntry {
	public:
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;

	private:
		struct AlwaysTrue {
			AlwaysTrue() = default;
			constexpr explicit AlwaysTrue(bool) noexcept {};
			consteval operator bool() const noexcept { return true; }
		};
		using ValueType = std::conditional_t<(HypertrieTrait_bool_valued<tr>), AlwaysTrue, value_type>;

		Key<tr> key_;
		mutable ValueType value_;

	public:
		NonZeroEntry() = default;
		explicit NonZeroEntry(size_t size) noexcept
			: key_(size), value_(1) {}
		explicit NonZeroEntry(Key<tr> key, value_type value = value_type(1))
			: key_(key), value_(value) {
			if (value == value_type{}) [[unlikely]]
				throw std::logic_error("value must not be zero equivalent.");
		}

		inline static NonZeroEntry make_filled(size_t size, key_part_type key_part, value_type value = value_type(1)) noexcept {
			return NonZeroEntry(Key<tr>(size, key_part), value);
		}

		const Key<tr> &key() const noexcept { return key_; }
		Key<tr> &key() noexcept { return key_; }
		const value_type &value() const noexcept { return value_; }
		void value([[maybe_unused]] value_type new_value) {
			if constexpr (not HypertrieTrait_bool_valued<tr>) {
				if (new_value != value_type{}) [[likely]]
					value_ = new_value;
				else [[unlikely]]
					throw std::logic_error("value must not be zero equivalent.");
			} else {
				assert(new_value);
			}
		}
		auto &operator[](size_t pos) noexcept { return key_[pos]; }
		const auto &operator[](size_t pos) const noexcept { return key_[pos]; }
		auto &at(size_t pos) noexcept { return key_.at(pos); }
		const auto &at(size_t pos) const noexcept { return key_.at(pos); }

		constexpr void resize(size_t count) noexcept { key_.resize(count); }

		[[nodiscard]] size_t size() const noexcept { return key_.size(); }

		std::tuple<Key<tr> const &, value_type> tuple() const noexcept {
			return std::forward_as_tuple(key_, value_);
		}

		std::tuple<Key<tr> &, value_type> tuple() noexcept {
			return std::forward_as_tuple(key_, value_);
		}

		void fill(key_part_type const &key_part) noexcept {
			std::fill(key_.begin(), key_.end(), key_part);
		}
	};
}// namespace Dice::hypertrie

namespace Dice::hash {
	template<::Dice::hypertrie::HypertrieTrait tr>
	struct is_ordered_container<::Dice::hypertrie::Key<tr>> : std::true_type {};

	template<::Dice::hypertrie::HypertrieTrait tr>
	struct is_ordered_container<::Dice::hypertrie::SliceKey<tr>> : std::true_type {};

	template<typename Policy, ::Dice::hypertrie::HypertrieTrait tr>
	struct dice_hash_overload<Policy, ::Dice::hypertrie::NonZeroEntry<tr>> {
		static std::size_t dice_hash(::Dice::hypertrie::NonZeroEntry<tr> const &entry) noexcept {
			return dice_hash_templates<Policy>::dice_hash(std::make_tuple(entry.key(), entry.value()));
		}
	};
}// namespace Dice::hash

namespace Dice::hash {

}// namespace Dice::hash

#endif//HYPERTRIE_KEY_HPP