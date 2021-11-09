#ifndef HYPERTRIE_PLAINTENSORHASH_HPP
#define HYPERTRIE_PLAINTENSORHASH_HPP

#include "Dice/hypertrie/internal/raw/node/TensorHash.hpp"

namespace hypertrie::internal::raw {

	/**
	 * The hash of an tensor is represented by the XOR of the hashes of its non-zero entries (key, value).
	 * Entries can be added or removed iteratively.
	 * PlainTensorHash does not store a tagging bit (which TensorHash does).
	 *
	 * Note: (a xor b xor b) = a (see also https://en.wikipedia.org/wiki/XOR_cipher )
	 *
	 */
	class PlainTensorHash {

		/**
		 * Hasher for an entry.
		 */
		template<size_t depth, typename key_part_type, typename V>
		//using EntryHash = robin_hood::hash<std::tuple<RawKey<depth, key_part_type>, V>>;
		using EntryHash = Dice::hash::DiceHash<std::tuple<RawKey<depth, key_part_type>, V>>;

		/**
		 * Bit representation of the hash.
		 */
		using NodeHashBitSet = std::bitset<sizeof(RawTensorHash) * 8>;

		/**
		 * hash value
		 */
		RawTensorHash hash_{};

	public:

		/*
		 * Constructors
		 */

		/**
		 * all zero hash.
		 */
		PlainTensorHash() {}

		PlainTensorHash(const PlainTensorHash &other)  noexcept: hash_(other.hash_) {}

		PlainTensorHash(const size_t &hash)  noexcept: hash_(hash) {}

		PlainTensorHash(PlainTensorHash &&other)  noexcept= default;

		PlainTensorHash &operator=(const PlainTensorHash &)  noexcept= default;
		PlainTensorHash &operator=(PlainTensorHash &&)  noexcept= default;

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
			return reinterpret_cast<const NodeHashBitSet&>(this->hash_);
		}
		/**
		 * Bit representation of the hash.
		 */
		[[nodiscard]] auto &bitset() noexcept {
			return reinterpret_cast<NodeHashBitSet&>(this->hash_);
		}

		/**
		 * Checks if hash is tagged as representing a compressed node. (always fals)
		 */
		[[nodiscard]] constexpr bool isCompressed() const noexcept {
			return false;
		}

		/**
		 * Checks if hash is tagged as representing an uncompressed node. (always true)
		 */
		[[nodiscard]] constexpr bool isUncompressed() const noexcept {
			return true;
		}

		/*
		 * Methods
		 */

		/**
		 * Changes the value of an entry.
		 * In this function it is not checked whether the given key and old_value was added to this hash before.
		 * This must be checked before.
		 * @tparam depth key depth/length
		 * @tparam key_part_type type of the parts of the key
		 * @tparam value_type type of the value
		 * @param key key to be changed
		 * @param old_value the old value the key had
		 * @param new_value the new value the key will have
		 * @return reference to self
		 */
		template<size_t depth, typename key_part_type, typename value_type>
		inline auto
		changeValue(const RawKey<depth, key_part_type> &key, const value_type &old_value, const value_type &new_value)  noexcept {
			hash_ = hash_ xor EntryHash<depth, key_part_type, value_type>()({key, old_value}) xor EntryHash<depth, key_part_type, value_type>()({key, new_value});
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
		template<size_t depth, typename key_part_type, typename value_type>
		inline auto addFirstEntry(const RawKey<depth, key_part_type> &key, const value_type &value)  noexcept {
			hash_ = hash_ xor EntryHash<depth, key_part_type, value_type>()({key, value});
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
		template<size_t depth, typename key_part_type, typename value_type>
		inline auto addEntry(const RawKey<depth, key_part_type> &key, const value_type &value) noexcept {
			hash_ = hash_ xor EntryHash<depth, key_part_type, value_type>()({key, value});
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
		template<size_t depth, typename key_part_type, typename value_type>
		inline auto
		removeEntry(const RawKey<depth, key_part_type> &key, const value_type &value, [[maybe_unused]] bool make_compressed) noexcept {
			hash_ = hash_ xor EntryHash<depth, key_part_type, value_type>()({key, value});
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
		template<size_t depth, typename key_part_type, typename value_type>
		static auto
		getCompressedNodeHash(const RawKey<depth, key_part_type> &key, const value_type &value) noexcept -> PlainTensorHash {
			return PlainTensorHash().addFirstEntry(key, value);
		}

		bool operator<(const PlainTensorHash &other) const noexcept {
			return this->hash_ < other.hash_;
		}

		bool operator==(const PlainTensorHash &other) const noexcept {
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

		explicit operator std::string() const noexcept {
			return fmt::format("u!_{:#019}", size_t(this->hash_ & (~size_t(1))));
		}

		friend std::ostream &operator<<(std::ostream &os, const PlainTensorHash &hash) {
			os << (std::string) hash;
			return os;
		}
	};

}// namespace hypertrie::internal

template<>
struct fmt::formatter<hypertrie::internal::raw::PlainTensorHash> {
	auto parse(format_parse_context &ctx) {
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const hypertrie::internal::raw::PlainTensorHash &hash, FormatContext &ctx) {
			return fmt::format_to(ctx.out(), "{}", (std::string) hash);
	}
};

namespace std {
	template<>
	struct hash<::hypertrie::internal::raw::PlainTensorHash> {
		size_t operator()(const ::hypertrie::internal::raw::PlainTensorHash &hash) const noexcept {
			return hash.hash();
		}
	};
}// namespace std

template<> std::size_t Dice::hash::dice_hash(::hypertrie::internal::raw::PlainTensorHash const &hash) noexcept {
        return hash.hash();
}
#endif//HYPERTRIE_PLAINTENSORHASH_HPP
