#ifndef HYPERTRIE_IDENTIFIER_HPP
#define HYPERTRIE_IDENTIFIER_HPP

#include "Dice/hypertrie/internal/raw/node/SingleEntry.hpp"


namespace Dice::hash {
	template<typename Policy, size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct dice_hash_overload<Policy, ::hypertrie::internal::raw::SingleEntry<depth, tri>> {
		static std::size_t dice_hash(::hypertrie::internal::raw::SingleEntry<depth, tri> const &entry) noexcept {
			return dice_hash_templates<Policy>::dice_hash(std::make_tuple(entry.key(), entry.value()));
		}
	};
}// namespace Dice::hash

namespace hypertrie::internal::raw {


	/**
	 * The hash of an tensor is represented by the XOR of the hashes of its non-zero entries (key, value).
	 * Entries can be added or removed iteratively. The hash stores an tag in the least significant bit that determines if it stores a
	 * compressed node.
	 *
	 * Note: (a xor b xor b) = a (see also https://en.wikipedia.org/wiki/XOR_cipher )
	 *
	 */
	template<size_t depth, HypertrieCoreTrait tri_t>
	class Identifier_impl {
	public:
		using tri = tri_t;
		using value_type = typename tri::value_type;

	protected:
		template<typename Entry>
		static constexpr bool is_entry_type =
				std::is_same_v<std::decay_t<Entry>, SingleEntry<depth, tri>> or
				std::is_same_v<std::decay_t<Entry>, SingleEntry<depth, tri_with_stl_alloc<tri>>>;


		/**
		 * hash value
		 */
		size_t hash_ = seed_;


		/**
		 * Position of the tag
		 */
		static constexpr const size_t tag_pos = (tri::taggable_key_part) ? tri::key_part_tagging_bit : 63UL;

		static constexpr const size_t seed_ = size_t(0)           // put the seed here
											  & ~(1UL << tag_pos);// the tagging bit is set to 0
		/**
		 * Tag value if the hash represents an uncompressed node.
		 */
		static const constexpr bool full_node_tag = false;
		/**
		 * Tag value if the hash represents a compressed node.
		 */
		static const constexpr bool single_entry_node_tag = true;

		constexpr static inline size_t tag_as_sen(size_t hash) noexcept {
			return hash | (1UL << tag_pos);
		}

		constexpr static inline size_t tag_as_fn(size_t hash) noexcept {
			return hash & ~(1UL << tag_pos);
		}

		/**
		 * Hashes the given entry and combines it with an invertible function with the seed.
		 * This function is self-inverse, i.e., Applying this function twice is the identity function:
		 * `seed == hash_and_combine(entry, hash_and_combine(entry, seed)`
		 * If no seed is provided, the default seed is used.
		 * @param entry the entry to hash and combine
		 * @param seed the seed to be combined with
		 * @return the combined hash
		 */
		template<typename Entry>
		static inline size_t hash_and_combine(Entry const &entry, size_t seed = seed_) noexcept requires is_entry_type<Entry> {
			using EntryHash = Dice::hash::DiceHashwyhash<Entry>;
			return EntryHash::hash_invertible_combine({seed, EntryHash()(entry)});
		}


		template<typename Iterable>
		static inline size_t hash_and_combine(Iterable entries) noexcept requires std::ranges::range<Iterable> and is_entry_type<std::ranges::range_value_t<Iterable>> {
			if (entries.begin() == entries.end())
				return seed_;
			size_t hash = seed_;
			size_t n = 0;
			for (const auto &entry : entries) {
				hash = hash_and_combine(entry, hash);
				n++;
			}
			if (n == 1UL)
				return tag_as_sen(hash);
			else
				return tag_as_fn(hash);
		}

		explicit Identifier_impl(size_t hash) noexcept : hash_{hash} {}

	public:
		Identifier_impl() = default;
		/*
		 * Constructs a single entry node.
		 */
		template<typename Entry>
		explicit Identifier_impl(Entry const &entry) noexcept requires is_entry_type<Entry> : hash_{tag_as_sen(hash_and_combine(entry))} {}
		/**
		 * Constructs any node.
		 * @param entries MUST NOT contain duplicates. This is not checked. The user is responsible to eliminate duplicates.
		 */
		template<typename Iterable>
		Identifier_impl(Iterable entries) noexcept requires std::ranges::range<Iterable> : hash_(hash_and_combine(entries)) {}

		/**
		 * The internally used value.
		 */
		[[nodiscard]] const size_t &hash() const noexcept {
			return this->hash_;
		}

		/**
		 * The internally used value.
		 */
		[[nodiscard]] size_t &hash() noexcept {
			return this->hash_;
		}

		/**
		 * Checks if hash is tagged as representing a single entry node.
		 */
		[[nodiscard]] inline bool is_sen() const noexcept {
			return not empty() and bool(this->hash_ & (1UL << tag_pos));
		}

		/**
		 * Checks if hash is tagged as representing a full node.
		 */
		[[nodiscard]] inline bool is_fn() const noexcept {
			return not empty() and bool(~this->hash_ & (1UL << tag_pos));
		}

		/*
		 * Methods
		 */

		/**
		 * Changes the value of an entry.
		 * In this function it is not checked whether the given key and old_value was added to this hash before.
		 * This must be checked before.
		 * @param old_entry the old value the key had
		 * @param new_value the new value the key will have
		 * @return reference to self
		 */
		template<typename Entry>
		inline auto changeValue(Entry const &old_entry, value_type const &new_value) noexcept requires is_entry_type<Entry> {
			bool is_sen_ = is_sen();

			using Entry_stl_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;

			Entry_stl_t new_entry{old_entry.key(), new_value};

			hash_ = hash_and_combine<Entry>(old_entry, hash_and_combine<Entry_stl_t>(new_entry, hash_));
			if (is_sen_)
				hash_ = tag_as_sen(hash_);
			else
				hash_ = tag_as_fn(hash_);
			return *this;
		}

		inline auto combine(Identifier_impl const &other) noexcept {
			if (this->empty())
				hash_ = other.hash_;
			else if (not other.empty()) {
				using Hash = Dice::hash::DiceHashwyhash<size_t>;

				hash_ = tag_as_fn(Hash::hash_invertible_combine({seed_, hash_, other.hash_}));
			}
			return *this;
		}

		/**
		 * Adds an entry. Identifier MUST NOT be empty before. The identifier will afterwards identify a full node.
		 * @param entry the entry to be added
		 * @return reference to self
		 */
		template<typename Entry>
		inline auto addEntry(Entry const &entry) noexcept requires is_entry_type<Entry> {
			assert(not empty());
			hash_ = tag_as_fn(hash_and_combine(entry, hash_));
			return *this;
		}

		/**
		 * Removes an entry. Identifier MUST identify a full node before. There is no (way to) check if the entry is actually contained. The user has to ensure this.
		 * @tparam entry the entry to be removed
		 * @param becomes_sen set to 1 if Identifier identifies exactly 2 entries before applying
		 * @return reference to self
		 */
		template<typename Entry>
		inline auto
		removeEntry(Entry const &entry, bool becomes_sen = false) noexcept requires is_entry_type<Entry> {
			assert(is_fn());
			hash_ = hash_and_combine(entry, hash_);
			if (becomes_sen)
				hash_ = tag_as_sen(hash_);
			else
				hash_ = tag_as_fn(hash_);
			return *this;
		}


		bool operator<(const Identifier_impl &other) const noexcept {
			return this->hash_ < other.hash_;
		}

		bool operator==(const Identifier_impl &other) const noexcept {
			return this->hash_ == other.hash_;
		}

		/**
		 * A hash is emtpy if it does not represent any node, i.e. it is all zero.
		 */
		[[nodiscard]] bool empty() const noexcept {
			return hash_ == seed_;
		}

		/**
		 * A hash is false if it does not represent any node, i.e. it is all zero.
		 * @see empty()
		 */
		explicit operator bool() const noexcept {
			return not empty();
		}

		explicit operator size_t() const noexcept {
			return hash_;
		}
	};

	template<size_t depth, HypertrieCoreTrait tri>
	class Identifier : public Identifier_impl<depth, tri> {
		using Identifier_impl_t = Identifier_impl<depth, tri>;

	protected:
		template<typename Entry>
		static constexpr bool is_entry_type = Identifier_impl_t::template is_entry_type<Entry>;

	public:
		Identifier() = default;

		/*
		 * Constructs a single entry node.
		 */
		template<typename Entry>
		Identifier(Entry const &entry) noexcept requires is_entry_type<Entry> : Identifier_impl_t{entry} {}
		/**
		 * Constructs any node.
		 * @param entries MUST NOT contain duplicates. This is not checked. The user is responsible to eliminate duplicates.
		 */
		template<typename Iterable>
		Identifier(Iterable entries) noexcept requires std::ranges::range<Iterable> : Identifier_impl_t{entries} {}


		template<typename Entry>
		Identifier(std::initializer_list<Entry> entries) noexcept requires is_entry_type<Entry> : Identifier_impl_t{entries} {}

		Identifier(Identifier_impl_t const &impl) noexcept  : Identifier_impl_t{impl} {}
	};

	template<HypertrieCoreTrait_bool_valued_and_taggable_key_part tri_t>
	class Identifier<1UL, tri_t> : public Identifier_impl<1UL, tri_t> {
		using Identifier_impl_t = Identifier_impl<1UL, tri_t>;

		using tri = tri_t;
		static constexpr size_t depth = 1UL;
		using value_type = typename tri::value_type;

	protected:
		template<typename Entry>
		static constexpr bool is_entry_type = Identifier_impl_t::template is_entry_type<Entry>;

		template<typename Entry>
		size_t encode_single_entry(Entry const &entry) requires is_entry_type<Entry> {
			auto hash = reinterpret_cast<size_t>(entry.key()[0]);
			assert(not bool(hash & (1UL << Identifier_impl_t::tag_pos)));// assert that tagging bit is 0
			return Identifier_impl_t::tag_as_sen(hash);
		}

		template<class Entry>
		static inline size_t hash_and_combine(std::initializer_list<Entry> entries) requires is_entry_type<Entry> {
			if (entries.size() == 0)
				return Identifier_impl_t::seed_;
			if (entries.size() == 1) {
				auto hash = reinterpret_cast<size_t>(entries.begin()->key()[0]);
				assert(not bool(hash & (1UL << Identifier_impl_t::tag_pos)));// assert that tagging bit is 0
				return Identifier_impl_t::tag_as_sen(hash);
			} else {
				size_t hash = Identifier_impl_t::seed_;
				for (const auto &entry : entries)
					hash = Identifier_impl_t::hash_and_combine(entry, hash);
				return Identifier_impl_t::tag_as_fn(hash);
			}
		}

	public :

		Identifier() = default;
		/*
		 * Constructs a single entry node.
		 */
		template<class Entry>
		Identifier(Entry const &entry) noexcept requires is_entry_type<Entry> : Identifier_impl_t{encode_single_entry(entry)} {}
		/**
		 * Constructs any node.
		 * @param entries MUST NOT contain duplicates. This is not checked. The user is responsible to eliminate duplicates.
		 */
		template<typename Iterable>
		Identifier(Iterable entries) noexcept requires std::ranges::range<Iterable> : Identifier_impl_t{hash_and_combine(entries)} {}

		template<typename Entry>
		Identifier(std::initializer_list<Entry> entries) noexcept requires is_entry_type<Entry> : Identifier_impl_t{hash_and_combine(entries)} {}


		/*
		 * Methods
		 */

		/**
		 * Changes the value of an entry.
		 * Use this function only on identifiers for full nodes.
		 * This must be checked before.
		 * @param old_entry the old value the key had
		 * @param new_value the new value the key will have
		 * @return reference to self
		 */
		template<class Entry>
		inline auto
		changeValue([[maybe_unused]] Entry const &old_entry, [[maybe_unused]] value_type const &new_value) noexcept requires is_entry_type<Entry> {
			assert(false);// The value of Boolean-valued entries cannot be changed.
		}

		inline SingleEntry<depth, tri_with_stl_alloc<tri>> get_entry() const noexcept {
			assert(this->is_sen());
			auto key_part = reinterpret_cast<typename tri::key_part_type>(this->tag_as_fn(this->hash_));
			return SingleEntry<depth, tri_with_stl_alloc<tri>>(RawKey<depth, tri_with_stl_alloc<tri>>{{key_part}});
		}

		/**
		 * Adds an entry. Identifier MUST NOT be empty before. The identifier will afterwards identify a full node.
		 * @param entry the entry to be added
		 * @return reference to self
		 */
		template<class Entry>
		inline auto addEntry(Entry const &entry) noexcept requires is_entry_type<Entry> {
			assert(not this->empty());
			if (this->is_sen()) {
				this->hash_ = this->tag_as_fn(Identifier_impl_t::hash_and_combine(this->get_entry(), Identifier_impl_t::hash_and_combine(entry, Identifier_impl_t::seed_)));
			} else {
				this->hash_ = this->tag_as_fn(Identifier_impl_t::hash_and_combine(entry, this->hash_));
			}
			return *this;
		}

		/**
		 * Removes an entry. Identifier MUST identify a full node with 3 or more entries.
		 * @tparam entry the entry to be removed
		 * @return reference to self
		 */
		template<class Entry>
		inline auto
		removeEntry(Entry const &entry) noexcept requires is_entry_type<Entry> {
			assert(this->is_fn());
			this->hash_ = this->tag_as_fn(Identifier_impl_t::hash_and_combine(entry, this->hash_));
			return *this;
		}
	};

}// namespace hypertrie::internal::raw

namespace Dice::hash {
	template<typename Policy, size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct dice_hash_overload<Policy, ::hypertrie::internal::raw::Identifier<depth, tri>> {
		static std::size_t dice_hash(::hypertrie::internal::raw::Identifier<depth, tri> const &identifier) noexcept {
			return identifier.hash();
		}
	};
}// namespace Dice::hash

namespace std {
	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct hash<::hypertrie::internal::raw::Identifier<depth, tri>> {
		size_t operator()(const ::hypertrie::internal::raw::Identifier<depth, tri> &identifier) const noexcept {
			return identifier.hash();
		}
	};
}// namespace std
#endif//HYPERTRIE_IDENTIFIER_HPP
