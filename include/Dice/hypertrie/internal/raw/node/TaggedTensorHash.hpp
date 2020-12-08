#ifndef HYPERTRIE_KEYPARTNODEHASHVARIANT_HPP
#define HYPERTRIE_KEYPARTNODEHASHVARIANT_HPP

#include <bitset>
#include <compare>

#include <fmt/ostream.h>

#include "Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp"
#include "Dice/hypertrie/internal/util/PosType.hpp"
#include "Dice/hypertrie/internal/util/RawKey.hpp"
#include "Dice/hypertrie/internal/raw/node/TensorHash.hpp"

namespace hypertrie::internal::raw {



	/**
	 * Tagged Tensor Hash is a wrapper around TensorHash.
	 * If tri::is_bool_valued and tri::is_lsb_unused it stores instead of a compressed nodes hash directly the key-part of a depth 1 node.
	 * @tparam tri
	 */
	template<HypertrieInternalTrait tri>
	class TaggedTensorHash {
		TensorHash hash_{};

	public:
		using tr = tri;

		using key_part_type = typename  tr::key_part_type;

		TaggedTensorHash() = default;

		TaggedTensorHash(const TensorHash &node_hash)  noexcept: hash_(node_hash) {}

		TaggedTensorHash(const key_part_type &key_part)  noexcept: hash_(reinterpret_cast<RawTensorHash>(key_part)) {
			hash_.bitset()[TensorHash::compression_tag_pos] = TensorHash::compressed_tag;
		}

		TaggedTensorHash & operator= (const TensorHash &node_hash) {
			hash_ = node_hash;
			return *this;
		}

	public:
		/**
		 * Checks if hash is tagged as representing a compressed node.
		 * @return
		 */
		[[nodiscard]] inline bool isCompressed() const noexcept {
			return bitset()[TensorHash::compression_tag_pos];
		}

		[[nodiscard]] inline key_part_type getKeyPart() const noexcept {
			TensorHash hash_copy = hash_;
			hash_copy.bitset()[TensorHash::compression_tag_pos] = TensorHash::uncompressed_tag;
			return reinterpret_cast<key_part_type>(hash_copy.hash());
		}

		/**
		 * Checks if hash is tagged as representing an uncompressed node.
		 * @return
		 */
		[[nodiscard]] inline bool isUncompressed() const noexcept {
			return not bitset()[TensorHash::compression_tag_pos];
		}

		[[nodiscard]] inline const TensorHash &getTaggedNodeHash() const noexcept {
			return this->hash_;
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
		inline auto addFirstEntry(const RawKey<1, key_part_type> &key, [[maybe_unused]] const bool &value)  noexcept {
			hash_.hash() = static_cast<key_part_type> (key[0]);
			hash_.bitset()[TensorHash::compression_tag_pos] = TensorHash::compressed_tag;
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
		inline auto addEntry(const RawKey<1, key_part_type> &key, [[maybe_unused]] const bool &value) noexcept {
			assert(value);
			if (hash_.isCompressed()) {
				hash_ = TensorHash::getCompressedNodeHash(getKeyPart(), true);
			}
			hash_.addEntry(key, value);
			return *this;
		}

		bool operator<(const TaggedTensorHash &other) const noexcept {
			return this->hash_ < other.hash_;
		}

		bool operator==(const TaggedTensorHash &other) const noexcept {
			return this->hash_ == other.hash_;
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
			return bool(this->hash_);
		}

		explicit operator TensorHash() const noexcept {
			return this->hash_;
		}

		/**
		 * The internally used hash.
		 * @return
		 */
		[[nodiscard]] const RawTensorHash &hash() const noexcept {
			return this->hash_.hash();
		}

		/**
		 * The union bitset of the internally used hash.
		 * @return
		 */
		[[nodiscard]] const auto &bitset() const noexcept {
			return this->hash_.bitset();
		}

		explicit operator std::string() const noexcept {
			if (this->isCompressed())
				return fmt::format("{}", getKeyPart());
			else
				return fmt::format("{}", this->hash_);
		}

		friend std::ostream &operator<<(std::ostream &os, const TaggedTensorHash &hash) {
			os << (std::string) hash;
			return os;
		}
	};

}// namespace hypertrie::internal



template<hypertrie::internal::raw::HypertrieInternalTrait tri>
struct fmt::formatter<hypertrie::internal::raw::TaggedTensorHash<tri>> {
	auto parse(format_parse_context &ctx) {
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const hypertrie::internal::raw::TaggedTensorHash<tri> &hash, FormatContext &ctx) {
		return fmt::format_to(ctx.out(), "{}", (std::string) hash);
	}
};

namespace std {
	template<hypertrie::internal::raw::HypertrieInternalTrait tri>
	struct hash<::hypertrie::internal::raw::TaggedTensorHash<tri>> {
		size_t operator()(const ::hypertrie::internal::raw::TaggedTensorHash<tri> &hash) const noexcept {
			return hash.hash();
		}
	};
}// namespace std
#endif//HYPERTRIE_KEYPARTNODEHASHVARIANT_HPP
