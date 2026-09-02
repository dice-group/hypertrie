#ifndef HYPERTRIE_RAWIDENTIFIER_HPP
#define HYPERTRIE_RAWIDENTIFIER_HPP

#include <execution>
#include <numeric>

#include "dice/hypertrie/Hypertrie_default_traits.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntry.hpp"

namespace dice::hypertrie::internal::raw {
	/**
	 * The kinds of nodes
	 */
	enum struct IdentifierTag : uint8_t {
		Indeterminate = 0b00, ///< Unknown Node Kind
		FN            = 0b01, ///< Full Node
		SEN           = 0b10, ///< Single Entry Node
		XN            = 0b11, ///< Cartesian Node
	};

	/**
	  * Identifiers are used to identify nodes.
	  * An identifier encodes the following information:
	  * 	- the type of node (SEN, FN or XN)
	  * 	- the size of the node
	  * 	- the combined hash of all entries in the node
	  */
	template<size_t depth, HypertrieTrait htt_t>
	struct RawIdentifier {
		using internal_hash_type = size_t;

		/**
		 * Seed. If the hash is equal to the seed it represents an empty node
		 */
		static constexpr internal_hash_type seed = 0; // default seed can be changed here

	private:
		internal_hash_type hash_ = seed;
		size_t size_ : 62 = 0;
		IdentifierTag tag_ : 2 = IdentifierTag::Indeterminate;


		template<typename T>
		using Hash = dice::hash::DiceHashwyhash<T>;

		/**
		 * Hashes the given entry and combines it with an invertible function with the seed.
		 * This function is self-inverse, i.e., Applying this function twice is the identity function:
		 * `seed == hash_and_combine(entry, hash_and_combine(entry, seed)`
		 * If no seed is provided, the default seed is used.
		 * @param entry the entry to hash and combine
		 * @param seed the seed to be combined with
		 * @return the combined hash
		 */
		[[nodiscard]] static internal_hash_type hash_and_combine(SingleEntry<depth, htt_t> const &entry, internal_hash_type hash = seed) noexcept {
			return Hash<size_t>::hash_invertible_combine({hash, Hash<SingleEntry<depth, htt_t>>{}(entry)});
		}

	public:
		RawIdentifier() = default;
		RawIdentifier(internal_hash_type hash, size_t size, IdentifierTag tag) noexcept : hash_{hash},
																						  size_{size},
																						  tag_{tag} {
			// require unique (apart from tag) null-id representation
			assert((hash == seed && size_ == 0) || (hash != seed && size_ != 0));
		}

		/*
		 * Constructs an identifier for a single entry.
		 */
		explicit RawIdentifier(SingleEntry<depth, htt_t> const &entry) noexcept : hash_{hash_and_combine(entry)},
																				  size_{1},
																				  tag_{IdentifierTag::SEN} {
			assert(entry.value() != typename htt_t::value_type{});
		}

		/**
		 * Constructs an RawIdentifier for a node represented by the entries provided.
		 * @param entries MUST NOT contain duplicates. This is not checked. The caller is responsible to eliminate duplicates beforehand.
		 */
		template<typename Iterable>
			requires std::ranges::range<Iterable> && std::is_convertible_v<std::ranges::range_value_t<Iterable>, SingleEntry<depth, htt_t>>
		explicit RawIdentifier(Iterable entries, IdentifierTag const hint = IdentifierTag::Indeterminate) noexcept {
			if (entries.begin() == entries.end()) {
				// size == 0, ignoring hint
				hash_ = seed;
				size_ = 0;
				tag_ = IdentifierTag::Indeterminate;
				return;
			}

			if (std::next(entries.begin()) == entries.end()) {
				// size == 1, ignoring hint
				hash_ = hash_and_combine(*entries.begin());
				size_ = 1;
				tag_ = IdentifierTag::SEN;
				return;
			}

			assert(hint != IdentifierTag::SEN);

			auto const hash_combine = [](size_t const h1, size_t const h2) noexcept {
				return Hash<size_t>::hash_invertible_combine({h1, h2});
			};

			auto const hash_entry = [](SingleEntry<depth, htt_t> const &entry) noexcept {
				return Hash<SingleEntry<depth, htt_t>>{}(entry);
			};

			// TODO: benchmark if we need to manually decide on parallelism
			hash_ = std::transform_reduce(std::execution::unseq,
										  entries.begin(),
										  entries.end(),
										  seed,
										  hash_combine,
										  hash_entry);
			size_ = std::distance(entries.begin(), entries.end());
			tag_ = hint;
		}

		static internal_hash_type hash_single_entry(SingleEntry<depth, htt_t> const &se) noexcept {
			return hash_and_combine(se);
		}

		/**
		 * @return internal hash
		 */
		[[nodiscard]] size_t hash() const noexcept {
			return hash_;
		}

		/**
		 * @return size of the identified node
		 */
		[[nodiscard]] size_t size() const noexcept {
			return size_;
		}

		/**
		 * @return kind of the identified node
		 */
		[[nodiscard]] IdentifierTag tag() const noexcept {
			return tag_;
		}

		/**
		 * @return if this identifies a node of unknown kind
		 */
		[[nodiscard]] bool is_indeterminate() const noexcept {
			return tag_ == IdentifierTag::Indeterminate;
		}

		/**
		 * @return if this identifies a full node.
		 */
		[[nodiscard]] bool is_fn() const noexcept {
			return tag_ == IdentifierTag::FN;
		}

		/**
		 * @return if this identifies a single entry node.
		 */
		[[nodiscard]] bool is_sen() const noexcept {
			return tag_ == IdentifierTag::SEN;
		}

		/**
		 * @return if this identifies a cartesian node
		 */
		[[nodiscard]] bool is_xn() const noexcept {
			return tag_ == IdentifierTag::XN;
		}

		/**
		 * @return if this does not identify any node
		 */
		[[nodiscard]] bool empty() const noexcept {
			return hash_ == seed && size_ == 0;
		}

		[[nodiscard]] RawIdentifier retag(IdentifierTag const tag) const noexcept {
			return RawIdentifier{hash_, size_, tag};
		}

		[[nodiscard]] RawIdentifier retag_as_indeterminate() const noexcept {
			return retag(IdentifierTag::Indeterminate);
		}

		[[nodiscard]] RawIdentifier retag_as_xn() const noexcept {
			return retag(IdentifierTag::XN);
		}

		[[nodiscard]] RawIdentifier retag_as_sen() const noexcept {
			return retag(IdentifierTag::SEN);
		}

		[[nodiscard]] RawIdentifier retag_as_fn() const noexcept {
			return retag(IdentifierTag::FN);
		}

		/**
		 * Changes the value of an entry.<br/>
		 * The caller is responsible to guarantee that old_entry is an entry of the node this identifies.
		 * @param old_entry the old entry
		 * @param new_value the new value to set for entry
		 * @return reference to this
		 */
		RawIdentifier &change_value(SingleEntry<depth, htt_t> const &old_entry, typename htt_t::value_type new_value) noexcept {
			static_assert(!HypertrieTrait_bool_valued<htt_t>,
						  "The value of an SingleEntry in an RawIdentifier cannot be changed if it is Boolean because there is only one value allowed (true).");
			assert(new_value != typename htt_t::value_type{});

			hash_ = hash_and_combine(old_entry,
									 hash_and_combine(/* new entry */ SingleEntry<depth, htt_t>{old_entry.key(), new_value}, hash_));
			return *this;
		}

		/**
		 * Combines the hashes of this and another identifier.
		 * The caller must ensure that this and other share no common entries. Common entries are otherwise removed from the combined hash.
		 * @param other the other identifier
		 * @return reference to this
		 */
		RawIdentifier &combine_add(RawIdentifier const &other, IdentifierTag const new_tag = IdentifierTag::Indeterminate) noexcept {
			hash_ = Hash<size_t>::hash_invertible_combine({hash_, other.hash_});
			size_ += other.size_;
			tag_ = new_tag;
			return *this;
		}

		RawIdentifier &combine_remove(RawIdentifier const &other, IdentifierTag const new_tag = IdentifierTag::Indeterminate) noexcept {
			hash_ = Hash<size_t>::hash_invertible_combine({hash_, other.hash_});
			size_ -= other.size_;
			tag_ = new_tag;
			return *this;
		}

		/**
		 * Adds an entry. The caller is responsible that the entry is not already represented by this.
		 * @param entry entry to be added
		 * @return reference to this
		 */
		RawIdentifier &add_entry(SingleEntry<depth, htt_t> const &entry, IdentifierTag const new_tag = IdentifierTag::Indeterminate) noexcept {
			hash_ = hash_and_combine(entry, hash_);
			size_ += 1;
			tag_ = new_tag;
			return *this;
		}

		/**
		 * Removes an entry. RawIdentifier MUST identify a full node before. There is no (way to) check if the entry is actually contained. The user has to ensure this.
		 * @tparam entry the entry to be removed
		 * @param becomes_sen set to 1 if RawIdentifier identifies exactly 2 entries before applying. MUST be false if in_place_node
		 * @return reference to self
		 */
		RawIdentifier &remove_entry(SingleEntry<depth, htt_t> const &entry, IdentifierTag const new_tag = IdentifierTag::Indeterminate) noexcept {
			assert(!is_sen());
			hash_ = hash_and_combine(entry, hash_);
			size_ -= 1;
			tag_ = new_tag;
			return *this;
		}

		bool operator==(RawIdentifier const &other) const noexcept = default;
		bool operator!=(RawIdentifier const &other) const noexcept = default;
	};

	// Sanity check for Identifier layout
	static_assert(sizeof(RawIdentifier<3, tagged_bool_Hypertrie_trait>) == sizeof(size_t) * 2);

}// namespace dice::hypertrie::internal::raw

namespace dice::hash {
	template<typename Policy>
	struct dice_hash_overload<Policy, ::dice::hypertrie::internal::raw::IdentifierTag> {
		static std::size_t dice_hash(::dice::hypertrie::internal::raw::IdentifierTag const tag) noexcept {
			return dice_hash_templates<Policy>::dice_hash(static_cast<std::underlying_type_t<::dice::hypertrie::internal::raw::IdentifierTag>>(tag));
		}
	};

	template<typename Policy, size_t depth, ::dice::hypertrie::HypertrieTrait htt_t>
	struct dice_hash_overload<Policy, ::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t>> {
		static std::size_t dice_hash(::dice::hypertrie::internal::raw::RawIdentifier<depth, htt_t> const &identifier) noexcept {
			return dice_hash_templates<Policy>::dice_hash(std::make_tuple(identifier.hash(), identifier.size(), identifier.tag()));
		}
	};
}// namespace dice::hash

#endif//HYPERTRIE_RAWIDENTIFIER_HPP
