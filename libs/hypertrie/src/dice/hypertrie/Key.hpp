#ifndef HYPERTRIE_KEY_HPP
#define HYPERTRIE_KEY_HPP

#include "dice/hash/DiceHash.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"

#include <algorithm>
#include <cassert>
#include <optional>
#include <vector>

namespace dice::hypertrie {
	// TODO: do not inherit from vector
	template<HypertrieTrait htt_t>
	class Key : public ::std::vector<typename htt_t::key_part_type> {
		using super_t = ::std::vector<typename htt_t::key_part_type>;

	public:
		Key() = default;
		explicit Key(typename super_t::size_type n, typename super_t::allocator_type const &a = typename super_t::allocator_type())
			: super_t(n, a) {}

		Key(typename super_t::size_type n, typename super_t::value_type const &value,
			typename super_t::allocator_type const &a = typename super_t::allocator_type())
			: super_t(n, value, a){};

		Key(super_t &&rv, typename super_t::allocator_type const &m) noexcept
			: super_t(std::move(rv), m, typename super_t::_Alloc_traits::is_always_equal{}) {}

		Key(std::initializer_list<typename super_t::value_type> l,
			typename super_t::allocator_type const &a = typename super_t::allocator_type())
			: super_t(l, a) {}

		template<typename InputIterator,
				 typename = std::_RequireInputIter<InputIterator>>
		Key(InputIterator first, InputIterator last,
			typename super_t::allocator_type const &a = typename super_t::allocator_type())
			: super_t(first, last, a) {}

		explicit operator std::string() const noexcept {
			std::string out = "⟨";
			auto size_ = this->size();
			for (size_t i = 0; i < size_; ++i) {
				out += std::to_string((*this)[i]);
				if (i != size_ - 1)
					out += ", ";
			}
			return out + "⟩";
		}
	};

	// TODO: do not inherit from vector
	template<HypertrieTrait htt_t>
	class SliceKey : public ::std::vector<::std::optional<typename htt_t::key_part_type>> {
		using super_t = ::std::vector<::std::optional<typename htt_t::key_part_type>>;

	public:
		SliceKey() = default;
		explicit SliceKey(typename super_t::size_type n, typename super_t::allocator_type const &a = typename super_t::allocator_type()) noexcept
			: super_t(n, a) {}

		SliceKey(typename super_t::size_type n, typename super_t::value_type const &value,
				 typename super_t::allocator_type const &a = typename super_t::allocator_type()) noexcept
			: super_t(n, value, a){};

		SliceKey(super_t &&rv, typename super_t::allocator_type const &m) noexcept
			: super_t(std::move(rv), m, typename super_t::_Alloc_traits::is_always_equal{}) {}

		SliceKey(std::initializer_list<typename super_t::value_type> l,
				 typename super_t::allocator_type const &a = typename super_t::allocator_type()) noexcept
			: super_t(l, a) {}

		template<typename InputIterator,
				 typename = std::_RequireInputIter<InputIterator>>
		SliceKey(InputIterator first, InputIterator last,
				 typename super_t::allocator_type const &a = typename super_t::allocator_type()) noexcept
			: super_t(first, last, a) {}

		[[nodiscard]] size_t get_fixed_depth() const noexcept {
			return this->size() - std::ranges::count_if(*this, [](auto const &item) { return not item.has_value(); });
		}
	};

	template<HypertrieTrait htt_t>
	class NonZeroEntry {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;

	private:
		struct AlwaysTrue {
			AlwaysTrue() = default;
			constexpr explicit AlwaysTrue(bool) noexcept {};
			consteval operator bool() const noexcept { return true; }
			static inline const bool default_instance = true;
		};
		using ValueType = std::conditional_t<(HypertrieTrait_bool_valued<htt_t>), AlwaysTrue, value_type>;

		Key<htt_t> key_;
		mutable ValueType value_;

	public:
		NonZeroEntry() = default;
		explicit NonZeroEntry(size_t size) noexcept
			: key_(size), value_(1) {}
		explicit NonZeroEntry(Key<htt_t> key, value_type value = value_type(1))
			: key_(std::move(key)), value_(value) {
			if (value == value_type{}) [[unlikely]] {
				throw std::logic_error("value must not be zero equivalent.");
			}
		}

		inline static NonZeroEntry make_filled(size_t size, key_part_type key_part, value_type value = value_type(1)) noexcept {
			return NonZeroEntry(Key<htt_t>(size, key_part), value);
		}

		Key<htt_t> const &key() const noexcept { return key_; }

		Key<htt_t> &key() noexcept { return key_; }

		value_type const &value() const noexcept {
			if constexpr (HypertrieTrait_bool_valued<htt_t>) {
				return AlwaysTrue::default_instance;
			} else {
				return this->value_;
			}
		}

		void value([[maybe_unused]] value_type new_value) {
			if constexpr (HypertrieTrait_bool_valued<htt_t>) {
				assert(new_value);
			} else {
				if (new_value != value_type{}) [[likely]] {
					value_ = new_value;
				} else [[unlikely]] {
					throw std::logic_error("value must not be zero equivalent.");
				}
			}
		}

		auto &operator[](size_t pos) noexcept { return key_[pos]; }

		auto const &operator[](size_t pos) const noexcept { return key_[pos]; }

		auto &at(size_t pos) noexcept { return key_.at(pos); }

		auto const &at(size_t pos) const noexcept { return key_.at(pos); }

		constexpr void resize(size_t count) noexcept { key_.resize(count); }

		[[nodiscard]] size_t size() const noexcept { return key_.size(); }

		std::tuple<Key<htt_t> const &, value_type> tuple() const noexcept {
			return std::forward_as_tuple(key_, value());
		}

		std::tuple<Key<htt_t> &, value_type> tuple() noexcept {
			return std::forward_as_tuple(key_, value());
		}

		void fill(key_part_type const &key_part) noexcept {
			std::fill(key_.begin(), key_.end(), key_part);
		}

		auto operator<=>(NonZeroEntry const &rhs) const noexcept {
			return std::tie(key_, value()) <=> std::tie(rhs.key_, rhs.value());
		}

		bool operator==(NonZeroEntry const &rhs) const noexcept {
			return std::tie(key_, value()) == std::tie(rhs.key_, rhs.value());
		}

		explicit operator std::string() const noexcept {
			return std::string(key_) + " -> " + std::to_string(value());
		}
	};

	template<HypertrieTrait htt_t>
	class Entry {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;

	private:
		Key<htt_t> key_;
		value_type value_ = value_type(1);

	public:
		Entry() = default;
		explicit Entry(Key<htt_t> key, value_type value = value_type(1))
			: key_(key), value_(value) {}

		Entry(NonZeroEntry<htt_t> const &non_zero_entry) : Entry(non_zero_entry.key(), non_zero_entry.value()) {}

		static Entry make_filled(size_t size, key_part_type key_part, value_type value = value_type(1)) noexcept {
			return Entry(Key<htt_t>(size, key_part), value);
		}

		Key<htt_t> const &key() const noexcept { return key_; }

		Key<htt_t> &key() noexcept { return key_; }

		value_type const &value() const noexcept { return value_; }

		void value([[maybe_unused]] value_type new_value) { value_ = new_value; }

		auto &operator[](size_t pos) noexcept { return key_[pos]; }

		auto const &operator[](size_t pos) const noexcept { return key_[pos]; }

		auto &at(size_t pos) noexcept { return key_.at(pos); }

		auto const &at(size_t pos) const noexcept { return key_.at(pos); }

		constexpr void resize(size_t count) noexcept { key_.resize(count); }

		[[nodiscard]] size_t size() const noexcept { return key_.size(); }

		std::tuple<Key<htt_t> const &, value_type> tuple() const noexcept {
			return std::forward_as_tuple(key_, value());
		}

		std::tuple<Key<htt_t> &, value_type> tuple() noexcept {
			return std::forward_as_tuple(key_, value());
		}

		void fill(key_part_type const &key_part) noexcept {
			std::fill(key_.begin(), key_.end(), key_part);
		}

		auto operator<=>(Entry const &rhs) const noexcept {
			return std::tie(key_, value()) <=> std::tie(rhs.key_, rhs.value());
		}

		bool operator==(Entry const &rhs) const noexcept {
			return std::tie(key_, value()) == std::tie(rhs.key_, rhs.value());
		}

		explicit operator std::string() const noexcept {
			return std::string(key_) + " -> " + std::to_string(value());
		}
	};
}// namespace dice::hypertrie

namespace dice::hash {
	template<::dice::hypertrie::HypertrieTrait htt_t>
	struct is_ordered_container<::dice::hypertrie::Key<htt_t>> : std::true_type {};

	template<::dice::hypertrie::HypertrieTrait htt_t>
	struct is_ordered_container<::dice::hypertrie::SliceKey<htt_t>> : std::true_type {};

	template<typename Policy, ::dice::hypertrie::HypertrieTrait htt_t>
	struct dice_hash_overload<Policy, ::dice::hypertrie::NonZeroEntry<htt_t>> {
		static std::size_t dice_hash(::dice::hypertrie::NonZeroEntry<htt_t> const &entry) noexcept {
			return dice_hash_templates<Policy>::dice_hash(std::make_tuple(entry.key(), entry.value()));
		}
	};

	template<typename Policy, ::dice::hypertrie::HypertrieTrait htt_t>
	struct dice_hash_overload<Policy, ::dice::hypertrie::Entry<htt_t>> {
		static std::size_t dice_hash(::dice::hypertrie::Entry<htt_t> const &entry) noexcept {
			return dice_hash_templates<Policy>::dice_hash(std::make_tuple(entry.key(), entry.value()));
		}
	};
}// namespace dice::hash

#endif//HYPERTRIE_KEY_HPP