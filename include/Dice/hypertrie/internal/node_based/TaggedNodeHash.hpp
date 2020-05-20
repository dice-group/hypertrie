#ifndef HYPERTRIE_TAGGEDNODEHASH_HPP
#define HYPERTRIE_TAGGEDNODEHASH_HPP

#include <compare>
#include <bitset>

#include <absl/hash/hash.h>

#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"

namespace hypertrie::internal::node_based {

	using NodeHash = size_t;

	class TaggedNodeHash {
		union {
			NodeHash thash_{};
			std::bitset<sizeof(NodeHash) * 8> thash_bits_;
		};


		static constexpr size_t tag_mask = size_t(1);
		static constexpr size_t notag_mask = ~size_t(1);
		static constexpr size_t uncompressed_tag = size_t(0);
		static constexpr size_t compressed_tag = size_t(1);
		static constexpr size_t compression_tag_pos = 0;

	public:
		TaggedNodeHash() {}

		explicit TaggedNodeHash(NodeHash thash) : thash_(thash) {}

	private:
		/**
		 * Create an initial hash for an empty hypertrie. Empty tensors are stored in uncompressed nodes.
		 * @param depth depth of the hypertrie
		 */
		explicit TaggedNodeHash(pos_type depth) : thash_(absl::Hash<pos_type>()(depth) | uncompressed_tag) {}

	public:

		[[nodiscard]] inline bool isCompressed() const {
			return thash_bits_[compression_tag_pos];
		}

		[[nodiscard]] inline bool isUncompressed() const {
			return not thash_bits_[compression_tag_pos];
		}

		template<typename V>
		inline void changeValue(const V &old_value, const V & new_value) {
			const bool tag = thash_bits_[compression_tag_pos];
			thash_ = thash_ ^ absl::Hash<V>()(old_value) ^ absl::Hash<V>()(new_value);
			thash_bits_[compression_tag_pos] = tag;
		}

		template<typename K, typename V>
		inline void addFirstEntry(const K &key, const V & value) {
			thash_ = thash_ ^ absl::Hash<K>()(key) ^ absl::Hash<V>()(value);
			thash_bits_[compression_tag_pos] = true;
		}

		template<typename K, typename V>
		inline void addEntry(const K &key, const V & value) {
			thash_ = thash_ ^ absl::Hash<K>()(key) ^ absl::Hash<V>()(value);
			thash_bits_[compression_tag_pos] = false;
		}

		template<typename K, typename V>
		inline void removeEntry(const K &key, const V & value, bool has_exactly_2_entries) {
			assert(isUncompressed());
			thash_ = thash_ ^ absl::Hash<K>()(key) ^ absl::Hash<V>()(value);
			thash_bits_[compression_tag_pos] = has_exactly_2_entries;
		}

		template<typename K, typename V>
		static auto getCompressedNodeHash(const pos_type  &depth, const K &key, const V & value) -> TaggedNodeHash{
			auto hash = TaggedNodeHash(depth);
			hash.addEntry(key, value);
			return hash;
		}

		template<pos_type depth, typename K, typename V>
		static auto getCompressedNodeHash(const K &key, const V & value) -> TaggedNodeHash{
			auto hash = TaggedNodeHash(depth);
			hash.addEntry(key, value);
			return hash;
		}

		static auto getEmptyNodeHash(const pos_type  &depth) -> TaggedNodeHash{
			return  TaggedNodeHash(depth);
		}

		template<pos_type depth>
		static auto getEmptyNodeHash() -> TaggedNodeHash{
			return  TaggedNodeHash(depth);
		}

		template<typename K, typename V>
		static auto getTwoEntriesNodeHash(const pos_type  &depth, const K &key, const V & value,
										  const K &second_key, const V & second_value) -> TaggedNodeHash{
			auto hash = TaggedNodeHash(depth);
			hash.addEntry(key, value);
			hash.addEntry(second_key, second_value);
			return hash;
		}

		template<typename K, typename V, pos_type depth>
		static auto getTwoEntriesNodeHash(const K &key, const V & value,
										  const K &second_key, const V & second_value) -> TaggedNodeHash{
			auto hash = TaggedNodeHash(depth);
			hash.addEntry(key, value);
			hash.addEntry(second_key, second_value);
			return hash;
		}

		auto operator<=>(const TaggedNodeHash &other) const {
			return this->thash_ <=> other.thash_;
		}

		auto operator<(const TaggedNodeHash &other) const {
			return this->thash_ < other.thash_;
		}

		auto operator==(const TaggedNodeHash &other) const {
			return this->thash_ == other.thash_;
		}

		auto operator!=(const TaggedNodeHash &other) const {
			return this->thash_ != other.thash_;
		}

		[[nodiscard]] bool empty() const noexcept {
			return this->operator bool();
		}

		operator bool() const noexcept {
			return this->thash_ != NodeHash{};
		}

		[[nodiscard]] const NodeHash &hash() const noexcept  {
			return this->thash_;
		}
	};


}
#endif //HYPERTRIE_TAGGEDNODEHASH_HPP
