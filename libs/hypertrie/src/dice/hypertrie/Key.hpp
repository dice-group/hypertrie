#ifndef HYPERTRIE_KEY_HPP
#define HYPERTRIE_KEY_HPP

#include "dice/hash/DiceHash.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <optional>
#include <sstream>
#include <vector>

namespace dice::hypertrie {
	template<HypertrieTrait htt_t>
	class Key {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = key_part_type;
		using inner_type = std::vector<value_type>;
		using reference = typename inner_type::reference;
		using const_reference = typename inner_type::const_reference;
		using pointer = typename inner_type::pointer;
		using const_pointer = typename inner_type::const_pointer;
		using allocator_type = typename inner_type::allocator_type;
		using size_type = typename inner_type::size_type;
		using iterator = typename inner_type::iterator;
		using const_iterator = typename inner_type::const_iterator;
		using reverse_iterator = typename inner_type::reverse_iterator;
		using const_reverse_iterator = typename inner_type::const_reverse_iterator;

	private:
		inner_type inner_;

		explicit Key(inner_type const &inner) : inner_{inner} {}
		explicit Key(inner_type &&inner) noexcept : inner_{std::move(inner)} {}

	public:
		Key() noexcept = default;

		Key(std::initializer_list<value_type> const init) : inner_{init} {
		}

		[[nodiscard]] static Key from_inner(inner_type const &inner) {
			return Key{inner};
		}

		[[nodiscard]] static Key from_inner(inner_type &&inner) noexcept {
			return Key{std::move(inner)};
		}

		[[nodiscard]] static Key make_defaulted(size_type const n) {
			return Key{inner_type(n, key_part_type{})};
		}

		[[nodiscard]] static Key make_filled(size_type const n, value_type const &value) {
			return Key{inner_type(n, value)};
		}

		template<std::input_iterator Iter>
		[[nodiscard]] static Key from_iters(Iter const begin, Iter const end) {
			return Key{inner_type(begin, end)};
		}

		inner_type &as_inner() noexcept { return inner_; }
		inner_type const &as_inner() const noexcept { return inner_; }

		[[nodiscard]] size_type size() const noexcept { return inner_.size(); }
		[[nodiscard]] bool empty() const noexcept { return inner_.empty(); }

		[[nodiscard]] key_part_type const *data() const noexcept { return inner_.data(); }
		[[nodiscard]] key_part_type *data() noexcept { return inner_.data(); }

		iterator begin() noexcept { return inner_.begin(); }
		iterator end() noexcept { return inner_.end(); }
		const_iterator begin() const noexcept { return inner_.begin(); }
		const_iterator end() const noexcept { return inner_.end(); }
		const_iterator cbegin() const noexcept { return inner_.begin(); }
		const_iterator cend() const noexcept { return inner_.end(); }
		reverse_iterator rbegin() noexcept { return inner_.rbegin(); }
		reverse_iterator rend() noexcept { return inner_.rend(); }
		const_reverse_iterator rbegin() const noexcept { return inner_.rbegin(); }
		const_reverse_iterator rend() const noexcept { return inner_.rend(); }

		reference operator[](size_type const ix) noexcept { return inner_[ix]; }
		const_reference operator[](size_type const ix) const noexcept { return inner_[ix]; }

		auto operator<=>(Key const &other) const noexcept = default;

		friend std::ostream &operator<<(std::ostream &os, Key const &key) noexcept {
			if (key.size() == 0) {
				os << "⟨⟩";
				return os;
			}

			os << "⟨";
			for (size_type ix = 0; ix < key.size() - 1; ++ix) {
				os << key[ix] << ", ";
			}
			os << key[key.size() - 1] << "⟩";

			return os;
		}

		friend std::string to_string(Key const &key) noexcept {
			std::ostringstream oss;
			oss << key;
			return oss.str();
		}
	};

	template<HypertrieTrait htt_t>
	class SliceKey {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = std::optional<key_part_type>;
		using inner_type = std::vector<value_type>;
		using reference = typename inner_type::reference;
		using const_reference = typename inner_type::const_reference;
		using pointer = typename inner_type::pointer;
		using const_pointer = typename inner_type::const_pointer;
		using allocator_type = typename inner_type::allocator_type;
		using size_type = typename inner_type::size_type;
		using iterator = typename inner_type::iterator;
		using const_iterator = typename inner_type::const_iterator;
		using reverse_iterator = typename inner_type::reverse_iterator;
		using const_reverse_iterator = typename inner_type::const_reverse_iterator;
		using fixed_key_type = Key<htt_t>;

	private:
		inner_type inner_;

		explicit SliceKey(inner_type const &inner) : inner_{inner} {}
		explicit SliceKey(inner_type &&inner) noexcept : inner_{std::move(inner)} {}

	public:
		SliceKey() noexcept = default;

		explicit SliceKey(fixed_key_type const &key) {
			inner_.reserve(key.size());
			std::ranges::copy(key, std::back_inserter(inner_));
		}

		SliceKey(std::initializer_list<value_type> const init) : inner_{init} {
		}

		SliceKey(std::initializer_list<typename fixed_key_type::value_type> const init) {
			inner_.reserve(init.size());
			std::ranges::copy(init, std::back_inserter(inner_));
		}

		[[nodiscard]] static SliceKey from_inner(inner_type const &inner) {
			return SliceKey{inner};
		}

		[[nodiscard]] static SliceKey from_inner(inner_type &&inner) noexcept {
			return SliceKey{std::move(inner)};
		}

		[[nodiscard]] static SliceKey make_defaulted(size_type const n) {
			return SliceKey{inner_type(n, key_part_type{})};
		}

		[[nodiscard]] static SliceKey make_unbound(size_type const n) {
			return SliceKey{inner_type(n, std::nullopt)};
		}

		[[nodiscard]] static SliceKey make_filled(size_type const n, value_type const &value) {
			return SliceKey{inner_type(n, value)};
		}

		template<typename Iter> requires std::input_iterator<Iter>
		[[nodiscard]] static SliceKey from_iters(Iter const begin, Iter const end) {
			return Key{inner_type(begin, end)};
		}

		[[nodiscard]] inner_type &as_inner() noexcept { return inner_; }
		[[nodiscard]] inner_type const &as_inner() const noexcept { return inner_; }

		[[nodiscard]] size_type size() const noexcept { return inner_.size(); }
		[[nodiscard]] bool empty() const noexcept { return inner_.empty(); }

		iterator begin() noexcept { return inner_.begin(); }
		iterator end() noexcept { return inner_.end(); }
		const_iterator begin() const noexcept { return inner_.begin(); }
		const_iterator end() const noexcept { return inner_.end(); }
		const_iterator cbegin() const noexcept { return inner_.begin(); }
		const_iterator cend() const noexcept { return inner_.end(); }
		reverse_iterator rbegin() noexcept { return inner_.rbegin(); }
		reverse_iterator rend() noexcept { return inner_.rend(); }
		const_reverse_iterator rbegin() const noexcept { return inner_.rbegin(); }
		const_reverse_iterator rend() const noexcept { return inner_.rend(); }

		reference operator[](size_type const ix) noexcept { return inner_[ix]; }
		const_reference operator[](size_type const ix) const noexcept { return inner_[ix]; }

		[[nodiscard]] std::optional<fixed_key_type> try_into_fixed_key() const {
			typename fixed_key_type::inner_type inner;
			inner.reserve(this->size());

			for (auto const &keyp : *this) {
				if (!keyp.has_value()) {
					return std::nullopt;
				}

				inner.push_back(*keyp);
			}

			return Key<htt_t>::from_inner(std::move(inner));
		}

		[[nodiscard]] fixed_key_type into_fixed_key_unchecked() const {
			typename fixed_key_type::inner_type inner;
			inner.reserve(this->size());
			std::ranges::transform(*this, std::back_inserter(inner), [](auto const &keyp) noexcept {
				return *keyp;
			});

			return Key<htt_t>::from_inner(std::move(inner));
		}

		auto operator<=>(SliceKey const &other) const noexcept = default;

		friend std::ostream &operator<<(std::ostream &os, SliceKey const &key) noexcept {
			if (key.size() == 0) {
				os << "⟨⟩";
				return os;
			}

			os << "⟨";
			for (size_type ix = 0; ix < key.size() - 1; ++ix) {
				if (auto const &keyp = key[ix]; keyp.has_value()) {
					os << *keyp << ", ";
				} else {
					os << ":, ";
				}
			}

			if (auto const &keyp = key[key.size() - 1]; keyp.has_value()) {
				os << *keyp;
			} else {
				os << ':';
			}
			os << "⟩";

			return os;
		}

		friend std::string to_string(SliceKey const &key) noexcept {
			std::ostringstream oss;
			oss << key;
			return oss.str();
		}

		[[nodiscard]] size_t get_fixed_depth() const noexcept {
			return this->size() - std::ranges::count_if(*this, [](auto const &item) { return !item.has_value(); });
		}
	};

	template<HypertrieTrait htt_t_>
	class NonZeroEntry {
	public:
		using htt_t = htt_t_;
		using key_part_type = typename htt_t::key_part_type;
		using key_type = Key<htt_t>;
		using value_type = typename htt_t::value_type;
		using size_type = typename key_type::size_type;

	private:
		struct KeyValue {
			key_type key_;
			value_type value_{1};

			KeyValue() noexcept = default;
			KeyValue(key_type const &key, value_type const value) : key_{key}, value_{value} {}
			KeyValue(key_type &&key, value_type const value) noexcept : key_{std::move(key)}, value_{value} {}

			auto operator<=>(KeyValue const &) const noexcept = default;
		};

		struct OnlyKey {
			key_type key_;

			OnlyKey() noexcept = default;
			OnlyKey(key_type const &key, value_type) : key_{key} {}
			OnlyKey(key_type &&key, value_type) noexcept : key_{std::move(key)} {}

			auto operator<=>(OnlyKey const &) const noexcept = default;
		};

		using KV_t = std::conditional_t<HypertrieTrait_bool_valued<htt_t>, OnlyKey, KeyValue>;
		KV_t inner_;

		inline constexpr value_type check_value(value_type const value) {
			if (value == value_type{}) [[unlikely]] {
				throw std::logic_error{"value must not be zero equivalent."};
			}

			return value;
		}
	public:
		NonZeroEntry() noexcept = default;
		NonZeroEntry(key_type const &key, value_type const value) : inner_{key, check_value(value)} {}
		NonZeroEntry(key_type &&key, value_type const value) : inner_{std::move(key), check_value(value)} {}

		NonZeroEntry(std::initializer_list<key_part_type> const key, value_type const value) : inner_{key, check_value(value)} {}
		NonZeroEntry(std::initializer_list<key_part_type> const key) requires (HypertrieTrait_bool_valued<htt_t>) : inner_{key, true} {}

		[[nodiscard]] static NonZeroEntry make_with_defaulted_key(size_t const size, value_type const value) noexcept {
			return NonZeroEntry{key_type::make_defaulted(size), value};
		}

		[[nodiscard]] NonZeroEntry make_with_filled_key(size_t const size, key_part_type const &key_part, value_type const value) noexcept {
			return NonZeroEntry{key_type::make_filled(size, key_part), value};
		}

		[[nodiscard]] key_type const &key() const noexcept { return inner_.key_; }
		[[nodiscard]] key_type &key() noexcept { return inner_.key_; }

		[[nodiscard]] value_type &value_mut() noexcept requires (!HypertrieTrait_bool_valued<htt_t>) {
			return inner_.value_;
		}

		[[nodiscard]] value_type value() const noexcept {
			if constexpr (HypertrieTrait_bool_valued<htt_t>) {
				return true;
			} else {
				return inner_.value_;
			}
		}

		void set_value(value_type const new_value) requires (!HypertrieTrait_bool_valued<htt_t>) {
			inner_.value_ = check_value(new_value);
		}

		[[nodiscard]] size_type size() const noexcept { return inner_.key_.size(); }
		[[nodiscard]] bool empty() const noexcept { return inner_.key_.empty(); }

		key_part_type operator[](size_type const key_ix) const noexcept { return inner_.key_[key_ix]; }
		key_part_type &operator[](size_type const key_ix) noexcept { return inner_.key_[key_ix]; }

		[[nodiscard]] std::tuple<key_type const &, value_type> as_tuple() const noexcept {
			return std::forward_as_tuple(inner_.key_, value());
		}

		[[nodiscard]] std::tuple<key_type &, value_type> as_tuple() noexcept {
			return std::forward_as_tuple(inner_.key_, value());
		}

		auto operator<=>(NonZeroEntry const &rhs) const noexcept = default;

		friend std::ostream &operator<<(std::ostream &os, NonZeroEntry const &entry) noexcept {
			os << entry.key() << " -> " << entry.value();
			return os;
		}

		friend std::string to_string(NonZeroEntry const &entry) noexcept {
			std::ostringstream oss;
			oss << entry;
			return oss.str();
		}
	};

	template<typename T>
	struct is_NonZeroEntry : std::false_type {
	};

	template<HypertrieTrait htt_t>
	struct is_NonZeroEntry<NonZeroEntry<htt_t>> : std::true_type {
	};

	template<typename T>
	inline constexpr bool is_NonZeroEntry_v = is_NonZeroEntry<T>::value;

	template<HypertrieTrait htt_t_>
	class Entry {
	public:
		using htt_t = htt_t_;
		using key_part_type = typename htt_t::key_part_type;
		using key_type = Key<htt_t>;
		using value_type = typename htt_t::value_type;
		using size_type = typename key_type::size_type;

	private:
		key_type key_;
		value_type value_{1};

	public:
		Entry() noexcept = default;
		Entry(key_type const &key, value_type const value) : key_{key}, value_{value} {}
		Entry(key_type &&key, value_type const value) noexcept : key_{std::move(key)}, value_{value} {}

		Entry(std::initializer_list<key_part_type> const key, value_type const value) : key_{key}, value_{value} {}

		Entry(NonZeroEntry<htt_t> const &non_zero_entry) : Entry{non_zero_entry.key(), non_zero_entry.value()} {}
		Entry(NonZeroEntry<htt_t> &&non_zero_entry) : Entry{std::move(non_zero_entry.key()), non_zero_entry.value()} {}

		[[nodiscard]] static Entry make_with_defaulted_key(size_t const size, value_type const value) {
			return Entry{key_type::make_defaulted(size), value};
		}

		[[nodiscard]] static Entry make_with_filled_key(size_t const size, key_part_type const &key_part, value_type const value) {
			return Entry{key_type::make_filled(size, key_part), value};
		}

		[[nodiscard]] key_type const &key() const noexcept { return key_; }
		[[nodiscard]] key_type &key() noexcept { return key_; }

		[[nodiscard]] value_type value() const noexcept { return value_; }
		void set_value(value_type const new_value) { value_ = new_value; }

		[[nodiscard]] size_type size() const noexcept { return key_.size(); }
		[[nodiscard]] bool empty() const noexcept { return key_.empty(); }

		key_part_type operator[](size_type const key_ix) const noexcept { return key_[key_ix]; }
		key_part_type &operator[](size_type const key_ix) noexcept { return key_[key_ix]; }

		[[nodiscard]] std::tuple<Key<htt_t> const &, value_type> as_tuple() const noexcept {
			return std::forward_as_tuple(key_, value_);
		}

		[[nodiscard]] std::tuple<Key<htt_t> &, value_type> as_tuple() noexcept {
			return std::forward_as_tuple(key_, value_);
		}

		auto operator<=>(Entry const &rhs) const noexcept = default;

		friend std::ostream &operator<<(std::ostream &os, Entry const &entry) noexcept {
			os << entry.key_ << " -> " << entry.value_;
			return os;
		}

		friend std::string to_string(Entry const &entry) noexcept {
			std::ostringstream oss;
			oss << entry;
			return oss.str();
		}
	};

	template<typename T>
	struct is_Entry : std::false_type {
	};

	template<HypertrieTrait htt_t>
	struct is_Entry<Entry<htt_t>> : std::true_type {
	};

	template<typename T>
	inline constexpr bool is_Entry_v = is_Entry<T>::value;

}// namespace dice::hypertrie

namespace dice::hash {
	template<::dice::hypertrie::HypertrieTrait htt_t>
	struct is_ordered_container<::dice::hypertrie::Key<htt_t>> : std::true_type {};

	template<::dice::hypertrie::HypertrieTrait htt_t>
	struct is_ordered_container<::dice::hypertrie::SliceKey<htt_t>> : std::true_type {};

	template<typename Policy, ::dice::hypertrie::HypertrieTrait htt_t>
	struct dice_hash_overload<Policy, ::dice::hypertrie::NonZeroEntry<htt_t>> {
		static std::size_t dice_hash(::dice::hypertrie::NonZeroEntry<htt_t> const &entry) noexcept {
			auto const value = entry.value();
			return dice_hash_templates<Policy>::dice_hash(std::tie(entry.key(), value));
		}
	};

	template<typename Policy, ::dice::hypertrie::HypertrieTrait htt_t>
	struct dice_hash_overload<Policy, ::dice::hypertrie::Entry<htt_t>> {
		static std::size_t dice_hash(::dice::hypertrie::Entry<htt_t> const &entry) noexcept {
			auto const value = entry.value();
			return dice_hash_templates<Policy>::dice_hash(std::tie(entry.key(), value));
		}
	};
}// namespace dice::hash

#endif//HYPERTRIE_KEY_HPP