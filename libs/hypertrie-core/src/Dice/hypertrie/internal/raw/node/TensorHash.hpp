#ifndef HYPERTRIE_TENSORHASH_HPP
#define HYPERTRIE_TENSORHASH_HPP

#include <bitset>
#include <compare>

#include "Dice/hypertrie/internal/raw/RawKey.hpp"

#include <Dice/hash/DiceHash.hpp>

namespace hypertrie::internal::raw {

	/**
	 * primitive type representation of TensorHash
	 */
	using RawTensorHash = size_t;

	/**
	 * The hash of an tensor is represented by the XOR of the hashes of its non-zero entries (key, value).
	 * Entries can be added or removed iteratively. The hash stores an tag in the least significant bit that determines if it stores a
	 * compressed node.
	 *
	 * Note: (a xor b xor b) = a (see also https://en.wikipedia.org/wiki/XOR_cipher )
	 *
	 */
	// TODO: document changed template parameters
	template<size_t depth, HypertrieCoreTrait tri_t>
	class TensorHash {
	public:
		using tri = tri_t;
		using RawKey = RawKey<depth, tri>;
		using value_type = typename tri::value_type;

	private:
		/**
		 * Hasher for an entry.
		 */
		using EntryHash = Dice::hash::DiceHash<std::tuple<RawKey, value_type>>;

		/**
		 * Bit representation of the hash.
		 */
		using NodeHashBitSet = std::bitset<sizeof(RawTensorHash) * 8>;

		/**
		 * hash value
		 */
		RawTensorHash hash_;

	public:
		/**
		 * Position of the tag in bitset()
		 */
		static const constexpr size_t compression_tag_pos = 0;
		/**
		 * Tag value if the hash represents an uncompressed node.
		 */
		static const constexpr bool uncompressed_tag = false;
		/**
		 * Tag value if the hash represents a compressed node.
		 */
		static const constexpr bool compressed_tag = true;

		/*
		 * Constructors
		 */

		/**
		 * all zero hash.
		 */
		TensorHash() noexcept = default;
		explicit TensorHash(const size_t &hash) noexcept : hash_(hash) {}

		/*
		 * Member Access
		 */

		/**
		 * The internally used value.
		 */
		[[nodiscard]] const RawTensorHash &hash() const noexcept {
			return this->hash_;
		}

		/**
		 * The internally used value.
		 */
		[[nodiscard]] RawTensorHash &hash() noexcept {
			return this->hash_;
		}

		/**
		 * Bit representation of the hash.
		 */
		[[nodiscard]] const auto &bitset() const noexcept {
			return reinterpret_cast<const NodeHashBitSet &>(this->hash_);
		}
		/**
		 * Bit representation of the hash.
		 */
		[[nodiscard]] auto &bitset() noexcept {
			return reinterpret_cast<NodeHashBitSet &>(this->hash_);
		}

		/**
		 * Checks if hash is tagged as representing a compressed node.
		 */
		[[nodiscard]] inline bool isCompressed() const noexcept {
			return bitset()[compression_tag_pos];
		}

		/**
		 * Checks if hash is tagged as representing an uncompressed node.
		 */
		[[nodiscard]] inline bool isUncompressed() const noexcept {
			return not bitset()[compression_tag_pos];
		}

		/*
		 * Methods
		 */

		/**
		 * Changes the value of an entry.
		 * In this function it is not checked whether the given key and old_value was added to this hash before.
		 * This must be checked before.
		 * @param key key to be changed
		 * @param old_value the old value the key had
		 * @param new_value the new value the key will have
		 * @return reference to self
		 */
		inline auto
		changeValue(RawKey const &key, value_type const &old_value, value_type const &new_value) noexcept {
			const bool tag = bitset()[compression_tag_pos];
			hash_ = hash_ xor EntryHash()({key, old_value}) xor EntryHash()({key, new_value});
			bitset()[compression_tag_pos] = tag;
			return *this;
		}

		/**
		 * Adds a first entry (key, value) to an empty, compressed node.
		 * Must only be called an empty, compressed nodes.
		 * @tparam depth  key depth/length
		 * @tparam key_part_type type of the parts of the key
		 * @tparam value_type type of the value
		 * @param key key to be added
		 * @param value value to be added
		 * @return reference to self
		 */
		inline auto addFirstEntry(RawKey const &key, value_type const &value) noexcept {
			hash_ = hash_ xor EntryHash()({key, value});
			bitset()[compression_tag_pos] = compressed_tag;
			return *this;
		}

		/**
		 * Adds an entry (key, value).
		 * @tparam depth  key depth/length
		 * @tparam key_part_type type of the parts of the key
		 * @tparam value_type type of the value
		 * @param key key to be added
		 * @param value value to be added
		 * @return reference to self
		 */
		inline auto addEntry(RawKey const &key, value_type const &value) noexcept {
			hash_ = hash_ xor EntryHash()({key, value});
			bitset()[compression_tag_pos] = uncompressed_tag;
			return *this;
		}

		/**
		 * Adds an entry (key, value).
		 * @tparam depth  key depth/length
		 * @tparam key_part_type type of the parts of the key
		 * @tparam value_type type of the value
		 * @param key key to be removed
		 * @param value value to be removed
		 * @param make_compressed if the node should be compressed afterwards
		 * @return reference to self
		 */
		inline auto
		removeEntry(RawKey const &key, value_type const &value, bool make_compressed) noexcept {
			assert(isUncompressed());
			hash_ = hash_ xor EntryHash()({key, value});
			bitset()[compression_tag_pos] = make_compressed;
			return *this;
		}

		/**
		 * Creates a hash for a compressed node containing one entry.
		 * @tparam depth  key depth/length
		 * @tparam key_part_type type of the parts of the key
		 * @tparam value_type type of the value
		 * @param key key to be added
		 * @param value value to be added
		 */
		static auto
		getCompressedNodeHash(RawKey const &key, value_type const &value) noexcept -> TensorHash {
			return TensorHash().addFirstEntry(key, value);
		}

		bool operator<(const TensorHash &other) const noexcept {
			return this->hash_ < other.hash_;
		}

		bool operator==(const TensorHash &other) const noexcept {
			return this->hash_ == other.hash_;
		}

		/**
		 * A hash is emtpy if it does not represent any node, i.e. it is all zero.
		 */
		[[nodiscard]] bool empty() const noexcept {
			return not this->operator bool();
		}

		/**
		 * A hash is false if it does not represent any node, i.e. it is all zero.
		 * @see empty()
		 */
		explicit operator bool() const noexcept {
			return this->hash_ != RawTensorHash{};
		}

		explicit operator size_t() const noexcept {
			return hash_;
		}
	};

}// namespace hypertrie::internal::raw

namespace std {

	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct hash<::hypertrie::internal::raw::TensorHash<depth, tri>> {
		size_t operator()(const ::hypertrie::internal::raw::TensorHash<depth, tri> &hash) const noexcept {
			return hash.hash();
		}
	};
}// namespace std

namespace Dice::hash {
	template<typename Policy, size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct dice_hash_overload<Policy, ::hypertrie::internal::raw::TensorHash<depth, tri>> {
		static std::size_t dice_hash(::hypertrie::internal::raw::TensorHash<depth, tri> const &hash) noexcept {
			return hash.hash();
		}
	};
}// namespace Dice::hash

#endif//HYPERTRIE_TENSORHASH_HPP