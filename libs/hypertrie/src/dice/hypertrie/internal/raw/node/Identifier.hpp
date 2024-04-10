#ifndef HYPERTRIE_IDENTIFIER_HPP
#define HYPERTRIE_IDENTIFIER_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"
#include "dice/hypertrie/internal/util/UnsafeCast.hpp"

namespace dice::hash {
	template<typename Policy, size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct dice_hash_overload<Policy, ::dice::hypertrie::internal::raw::SingleEntry<depth, htt_t>> {
		static std::size_t dice_hash(::dice::hypertrie::internal::raw::SingleEntry<depth, htt_t> const &entry) noexcept {
			return dice_hash_templates<Policy>::dice_hash(std::make_tuple(entry.key(), entry.value()));
		}
	};
}// namespace dice::hash


namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t>
	class RawIdentifier;


	template<HypertrieTrait htt_t>
	class Identifier {
	public:
		using value_type = typename htt_t::value_type;

	protected:
		/**
		 * hash value
		 */
		size_t hash_ = seed_;


	public:
		/**
		 * Position of the tag
		 */
		static constexpr const size_t tag_pos = (htt_t::taggable_key_part) ? htt_t::key_part_tagging_bit : 63UL;

		/**
		 * Seed. If the hash is equal to the seed it represents an empty node
		 */
		static constexpr const size_t seed_ = size_t(0)           // default seed can be changed here
											  & ~(1UL << tag_pos);// the tagging bit is set to 0

		explicit Identifier(size_t hash) noexcept : hash_(hash) {}


		Identifier() = default;

		/**
		 * internal representation
		 */
		[[nodiscard]] const size_t &hash() const noexcept { return this->hash_; }

		/**
		 * internal representation
		 */
		[[nodiscard]] size_t &hash() noexcept { return this->hash_; }

		/**
		 * If this identifies a single entry node.
		 */
		[[nodiscard]] inline bool is_sen() const noexcept {
			return not empty() and bool(this->hash_ & (1UL << tag_pos));
		}

		/**
		 * If this identifies a full node.
		 */
		[[nodiscard]] inline bool is_fn() const noexcept {
			return not empty() and bool(~this->hash_ & (1UL << tag_pos));
		}

		[[nodiscard]] auto operator<=>(const Identifier &other) const noexcept = default;

		/**
		 * An identifier is emtpy if it does not represent any node.
		 */
		[[nodiscard]] bool empty() const noexcept { return hash_ == seed_; }

		/**
		 * An identifier is false if it does not represent any node.
		 * @see empty()
		 */
		explicit operator bool() const noexcept { return not empty(); }

		explicit operator size_t() const noexcept { return hash_; }

		template<size_t depth>
		explicit operator RawIdentifier<depth, htt_t>() const noexcept { return RawIdentifier<depth, htt_t>{hash_}; }
	};


	/**
	  * <div>Identifier</div>s are used to identify nodes.
	  * An identifier encodes information about if it represents a SingleEntryNode (SEN) or a FullNode (FN).
	  * For HypertrieCoreTrait_bool_valued_and_taggable_key_part and depth = 1, the SingleEntry is not stored in a separate node but within the Identifier.
	  * The Identifier is implemented by combining the hashes of the <div>SingleEntry</div>s with xor. (see also https://en.wikipedia.org/wiki/XOR_cipher )
	  * <p>Note: (a xor b xor b) = a</p>
	  * @tparam depth
	  * @tparam tri_t
	  */
	template<size_t depth, HypertrieTrait htt_t>
	class RawIdentifier : public Identifier<htt_t> {
		using super_t = Identifier<htt_t>;

	public:
		using value_type = typename htt_t::value_type;
		/**
		 * A node is stored "in-place", i.e. in the identifier, if it fits in. That is the case for Boolean-valued single-entry nodes of depth-1 where the key_part is marked as taggable (HypertrieCoreTrait_bool_valued_and_taggable_key_part).
		 */
		static constexpr bool in_place_node = HypertrieTrait_bool_valued_and_taggable_key_part<htt_t> and depth == 1UL;

	protected:
		using Entry = SingleEntry<depth, htt_t>;

	public:
		/**
		 * Position of the tag
		 */
		static constexpr const size_t tag_pos = super_t::tag_pos;

		static constexpr const size_t seed_ = super_t::seed_;

	protected:
		[[nodiscard]] static constexpr size_t tag_as_sen(size_t hash) noexcept {
			return hash | (1UL << tag_pos);
		}

		[[nodiscard]] static constexpr size_t tag_as_fn(size_t hash) noexcept {
			return hash & ~(1UL << tag_pos);
		}

		static size_t encode_single_entry(Entry const &entry) noexcept {
			if constexpr (in_place_node) {
				auto hash = util::unsafe_copy_cast<size_t>(entry.key()[0]);
				assert(not bool(hash & (1UL << tag_pos)));// assert that tagging bit is 0
				return tag_as_sen(hash);
			} else {
				return tag_as_sen(hash_and_combine(entry));
			}
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
		[[nodiscard]] static size_t hash_and_combine(Entry const &entry, size_t seed = seed_) noexcept {
			using EntryHash = dice::hash::DiceHashMartinus<Entry>;
			return EntryHash::hash_invertible_combine({seed, EntryHash()(entry)});
		}


		template<typename Iterable>
		static size_t hash_iterable(Iterable entries) noexcept requires std::ranges::range<Iterable> and std::is_convertible_v<std::ranges::range_value_t<Iterable>, Entry> {
			if (entries.begin() == entries.end())
				return seed_;

			if constexpr (in_place_node)
				if (entries.begin() + 1 == entries.end())
					return encode_single_entry(*entries.begin());

			size_t hash = seed_;
			size_t n = 0;
			for (const auto &entry : entries) {
				hash = hash_and_combine(entry, hash);
				n++;
			}
			if constexpr (not in_place_node)
				if (n == 1UL)
					return tag_as_sen(hash);
			return tag_as_fn(hash);
		}

		explicit RawIdentifier(size_t hash) noexcept : super_t{hash} {}
		friend Identifier<htt_t>;

	public:
		RawIdentifier() = default;
		/*
		 * Constructs an identifier for a single entry.
		 */
		explicit RawIdentifier(Entry const &entry) noexcept : super_t{encode_single_entry(entry)} {}

		/**
		 * Constructs an RawIdentifier for a node represented by the entries provided.
		 * @param entries MUST NOT contain duplicates. This is not checked. The caller is responsible to eliminate duplicates beforehand.
		 */
		template<typename Iterable>
		explicit RawIdentifier(Iterable entries) noexcept requires std::ranges::range<Iterable> and std::is_convertible_v<std::ranges::range_value_t<Iterable>, Entry> : super_t(hash_iterable(entries)) {}

		/**
		 * Constructs an RawIdentifier for a node represented by the entries provided.
		 * @param entries MUST NOT contain duplicates. This is not checked. The caller is responsible to eliminate duplicates beforehand.
		 */
		RawIdentifier(std::initializer_list<Entry> entries) noexcept : super_t(hash_iterable(entries)) {}

		/**
		 * Changes the value of an entry.<br/>
		 * The caller is responsible to guarantee that old_entry is an entry of the node this identifies.
		 * @param old_entry the old entry
		 * @param new_value the new value to set for entry
		 * @return reference to this
		 */
		RawIdentifier &changeValue(Entry const &old_entry, value_type const &new_value) noexcept {
			static_assert(not std::is_same_v<typename htt_t::value_type, bool>,
						  "The value of an SingleEntry in an RawIdentifier cannot be changed if it is Boolean because there is only one value allowed (true).");
			assert(new_value != value_type{});
			bool is_sen_ = this->is_sen();

			this->hash_ = hash_and_combine(old_entry,
										   hash_and_combine(/* new entry */ Entry(old_entry.key(), new_value), this->hash_));
			if (is_sen_)
				this->hash_ = tag_as_sen(this->hash_);
			else
				this->hash_ = tag_as_fn(this->hash_);
			return *this;
		}

		/**
		 * Combines the hashes of this and another identifier.
		 * The caller must ensure that this and other share no common entries. Common entries are otherwise removed from the combined hash.
		 * @param other the other identifier
		 * @return reference to this
		 */
		RawIdentifier &combine(RawIdentifier const &other) noexcept {
			if (this->empty())
				this->hash_ = other.hash_;
			else if (not other.empty()) {
				using Hash = dice::hash::DiceHashMartinus<size_t>;

				if constexpr (in_place_node) {
					auto left_hash = (this->is_sen()) ? hash_and_combine(get_entry()) : this->hash_;
					auto right_hash = (other.is_sen()) ? hash_and_combine(other.get_entry()) : other.hash_;
					this->hash_ = tag_as_fn(Hash::hash_invertible_combine({seed_, left_hash, right_hash}));
				} else {
					this->hash_ = tag_as_fn(Hash::hash_invertible_combine({seed_, this->hash_, other.hash_}));
				}
			}
			return *this;
		}

		/**
		 * Extract the SingleEntry of an in_place_node.
		 * @return an single Entry allocated with std::allocator
		 */
		// TODO: is it required that this is a template function? template<void>
		[[nodiscard]] Entry get_entry() const noexcept {
			static_assert(in_place_node, "get_entry must only be called on on identifiers which store in_place_node.");
			assert(this->is_sen());
			auto key_part = util::unsafe_copy_cast<typename htt_t::key_part_type>(this->tag_as_fn(this->hash_));
			return Entry{RawKey<depth, htt_t>({key_part})};
		}

		/**
		 * Adds an entry. The caller is responsible that the entry is not already represented by this.
		 * @param entry entry to be added
		 * @return reference to this
		 */
		RawIdentifier &addEntry(Entry const &entry) noexcept {
			if (this->empty())
				this->hash_ = encode_single_entry(entry);
			else if (this->is_sen()) {
				if constexpr (in_place_node)
					this->hash_ = tag_as_fn(hash_and_combine(this->get_entry(), hash_and_combine(entry, seed_)));
				else
					this->hash_ = tag_as_fn(hash_and_combine(entry, this->hash_));
			} else {
				this->hash_ = tag_as_fn(hash_and_combine(entry, this->hash_));
			}
			return *this;
		}

		/**
		 * Removes an entry. RawIdentifier MUST identify a full node before. There is no (way to) check if the entry is actually contained. The user has to ensure this.
		 * @tparam entry the entry to be removed
		 * @param becomes_sen set to 1 if RawIdentifier identifies exactly 2 entries before applying. MUST be false if in_place_node
		 * @return reference to self
		 */
		RawIdentifier &removeEntry(Entry const &entry, bool becomes_sen = false) noexcept {
			assert(this->is_fn());
			if constexpr (in_place_node)
				assert(not becomes_sen);
			this->hash_ = hash_and_combine(entry, this->hash_);
			if (becomes_sen)
				this->hash_ = tag_as_sen(this->hash_);
			else
				this->hash_ = tag_as_fn(this->hash_);
			return *this;
		}

		[[nodiscard]] auto operator<=>(const RawIdentifier &other) const noexcept = default;
	};

}// namespace dice::hypertrie::internal::raw


namespace dice::hash {
	template<typename Policy, size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct dice_hash_overload<Policy, ::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t>> {
		static std::size_t dice_hash(::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t> const &identifier) noexcept {
			return identifier.hash();
		}
	};
}// namespace dice::hash

namespace std {
	template<size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct hash<::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t>> {
		size_t operator()(const ::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t> &identifier) const noexcept {
			return identifier.hash();
		}
	};
}// namespace std

namespace dice::hash {
	template<typename Policy, ::dice::hypertrie::HypertrieTrait htt_t>
	struct dice_hash_overload<Policy, ::dice::hypertrie::internal::raw::Identifier<htt_t>> {
		static std::size_t dice_hash(::dice::hypertrie::internal::raw::Identifier<htt_t> const &identifier) noexcept {
			return identifier.hash();
		}
	};
}// namespace dice::hash

namespace std {
	template<::dice::hypertrie::HypertrieTrait htt_t>
	struct hash<::dice::hypertrie::internal::raw::Identifier<htt_t>> {
		size_t operator()(const ::dice::hypertrie::internal::raw::Identifier<htt_t> &identifier) const noexcept {
			return identifier.hash();
		}
	};
}// namespace std
#endif//HYPERTRIE_IDENTIFIER_HPP
