#ifndef HYPERTRIE_TAGGEDNODEHASH_HPP
#define HYPERTRIE_TAGGEDNODEHASH_HPP

#include <bitset>
#include <compare>

#include <absl/hash/hash.h>
#include <fmt/ostream.h>

#include "Dice/hypertrie/internal/util/PosType.hpp"
#include "Dice/hypertrie/internal/util/RawKey.hpp"

namespace hypertrie::internal::node_based {

	using NodeHash = size_t;

	/**
	 * The hash of a node of a hypertrie is stored in a TaggedNodeHash. It hashes all (key, value) of non-zero entries.
	 * Entries can be added or removed iteratively. The hash stores an additional tag that determines if it stores a
	 * compressed node.
	 */
	class TaggedNodeHash {

		using NodeHashBitSet = std::bitset<sizeof(NodeHash) * 8>;
		NodeHash thash_{};

		template<size_t depth, typename key_part_type, typename V>
		using EntryHash = absl::Hash<std::tuple<RawKey<depth, key_part_type>, V>>;

	public:
		/**
		 * The union bitset of the internally used hash.
		 * @return
		 */
		[[nodiscard]] const auto &bitset() const noexcept {
			return reinterpret_cast<const NodeHashBitSet&>(this->thash_);
		}

		[[nodiscard]] auto &bitset() noexcept {
			return reinterpret_cast<NodeHashBitSet&>(this->thash_);
		}

		/**
		 * Tag value if the hash represents an uncompressed node.
		 */
		static constexpr bool uncompressed_tag = false;
		/**
		 * Tag value if the hash represents a compressed node.
		 */
		static constexpr bool compressed_tag = true;
		/**
		 * Position of the tag in bitset()
		 */
		static constexpr size_t compression_tag_pos = 0;

		/**
		 * Default constructor returning an empty hash (all zero).
		 */
		TaggedNodeHash() {}

		TaggedNodeHash(const TaggedNodeHash &other)  noexcept: thash_(other.thash_) {}

		TaggedNodeHash(TaggedNodeHash &&other)  noexcept= default;

		TaggedNodeHash &operator=(const TaggedNodeHash &)  noexcept= default;
		TaggedNodeHash &operator=(TaggedNodeHash &&)  noexcept= default;

	public:
		/**
		 * Checks if hash is tagged as representing a compressed node.
		 * @return
		 */
		[[nodiscard]] inline bool isCompressed() const noexcept {
			return bitset()[compression_tag_pos];
		}

		/**
		 * Checks if hash is tagged as representing an uncompressed node.
		 * @return
		 */
		[[nodiscard]] inline bool isUncompressed() const noexcept {
			return not bitset()[compression_tag_pos];
		}

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
		 * @return reference to slef
		 */
		template<size_t depth, typename key_part_type, typename value_type>
		inline auto
		changeValue(const RawKey<depth, key_part_type> &key, const value_type &old_value, const value_type &new_value)  noexcept {
			const bool tag = bitset()[compression_tag_pos];
			thash_ = thash_ xor EntryHash<depth, key_part_type, value_type>()({key, old_value}) xor EntryHash<depth, key_part_type, value_type>()({key, new_value});
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
		 * @return reference to slef
		 */
		template<size_t depth, typename key_part_type, typename value_type>
		inline auto addFirstEntry(const RawKey<depth, key_part_type> &key, const value_type &value)  noexcept {
			thash_ = thash_ xor EntryHash<depth, key_part_type, value_type>()({key, value});
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
		 * @return reference to slef
		 */
		template<size_t depth, typename key_part_type, typename value_type>
		inline auto addEntry(const RawKey<depth, key_part_type> &key, const value_type &value) noexcept {
			thash_ = thash_ xor EntryHash<depth, key_part_type, value_type>()({key, value});
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
		 * @return reference to slef
		 */
		template<size_t depth, typename key_part_type, typename value_type>
		inline auto
		removeEntry(const RawKey<depth, key_part_type> &key, const value_type &value, bool make_compressed)  noexcept {
			assert(isUncompressed());
			thash_ = thash_ xor EntryHash<depth, key_part_type, value_type>()({key, value});
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
		 * @return
		 */
		template<size_t depth, typename key_part_type, typename value_type>
		static auto
		getCompressedNodeHash(const RawKey<depth, key_part_type> &key, const value_type &value) noexcept -> TaggedNodeHash {
			auto hash = getRawEmptyNodeHash<depth>();
			hash.addFirstEntry(key, value);
			return hash;
		}

	private:
		/**
		 * Creates an empty node hash. Does not initialize compression tag.
		 * @tparam depth  key depth/length
		 * @return
		 */
		template<size_t depth>
		static auto getRawEmptyNodeHash() noexcept -> TaggedNodeHash {
			auto hash = TaggedNodeHash();
			hash.thash_ = absl::Hash<pos_type>()(depth);
			return hash;
		}

	public:

		/**
		 * Creates an empty, uncompressed node hash.
		 * @tparam depth  key depth/length
		 * @return
		 */
		template<size_t depth>
		static auto getUncompressedEmptyNodeHash() noexcept -> TaggedNodeHash {
			TaggedNodeHash hash = getRawEmptyNodeHash<depth>();
			hash.bitset()[compression_tag_pos] = uncompressed_tag;
			return hash;
		}

		/**
		 * Creates a hash for a compressed node containing two entry.
		 * @tparam depth  key depth/length
		 * @tparam key_part_type type of the parts of the key
		 * @tparam value_type type of the value
		 * @param key key to be added
		 * @param value value to be added
	  	 * @param second_key 2. key to be added
		 * @param second_value 2. value to be added
		 * @return
		 */
		template<size_t depth, typename key_part_type, typename V>
		static auto getTwoEntriesNodeHash(const RawKey<depth, key_part_type> &key, const V &value,
										  const RawKey<depth, key_part_type> &second_key,
										  const V &second_value)  noexcept -> TaggedNodeHash {
			auto hash = getUncompressedEmptyNodeHash<depth>();
			hash.addEntry(key, value);
			hash.addEntry(second_key, second_value);
			return hash;
		}

		bool operator<(const TaggedNodeHash &other) const noexcept {
			return this->thash_ < other.thash_;
		}

		bool operator==(const TaggedNodeHash &other) const noexcept {
			return this->thash_ == other.thash_;
		}

		/**
		 * A hash is emtpy if it does not represent any node, i.e. it is all zero.
		 * @return
		 */
		[[nodiscard]] bool empty() const noexcept {
			return not this->operator bool();
		}

		/**
		 * A hash is false if it does not represent any node, i.e. it is all zero.
		 * @see empty()
		 * @return
		 */
		explicit operator bool() const noexcept {
			return this->thash_ != NodeHash{};
		}

		/**
		 * The internally used hash.
		 * @return
		 */
		[[nodiscard]] const NodeHash &hash() const noexcept {
			return this->thash_;
		}

		[[nodiscard]] NodeHash &hash() noexcept {
			return this->thash_;
		}

		explicit operator std::string() const noexcept {
			const char compression = (this->isCompressed()) ? 'c' : 'u';
			return fmt::format("{}_{:#019}", compression, size_t(this->thash_ & (~size_t(1))));
		}

		friend std::ostream &operator<<(std::ostream &os, const TaggedNodeHash &hash) {
			os << (std::string) hash;
			return os;
		}
	};

}// namespace hypertrie::internal::node_based

template<>
struct fmt::formatter<hypertrie::internal::node_based::TaggedNodeHash> {
	auto parse(format_parse_context &ctx) {
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const hypertrie::internal::node_based::TaggedNodeHash &hash, FormatContext &ctx) {
			return fmt::format_to(ctx.out(), "{}", (std::string) hash);
	}
};

namespace std {
	template<>
	struct hash<::hypertrie::internal::node_based::TaggedNodeHash> {
		size_t operator()(const ::hypertrie::internal::node_based::TaggedNodeHash &hash) const noexcept {
			return hash.hash();
		}
	};
}// namespace std
#endif//HYPERTRIE_TAGGEDNODEHASH_HPP
