#ifndef HYPERTRIE_TAGGEDNODEHASH_HPP
#define HYPERTRIE_TAGGEDNODEHASH_HPP

#include <compare>

#include <absl/hash/hash.h>
#include <bitset>

namespace hypertrie::internal::node_based {

	using NodeHash = size_t;

	class TaggedNodeHash {
		union {
			NodeHash thash_{};
			std::bitset<sizeof(NodeHash)*8> thash_bits_;
		};


		static constexpr size_t tag_mask = size_t(1);
		static constexpr size_t notag_mask = ~size_t(1);
		static constexpr size_t uncompressed_tag = size_t(0);
		static constexpr size_t compressed_tag = size_t(1);
		static constexpr size_t compression_tag_pos = 0;

	public:
		TaggedNodeHash() {}

		explicit TaggedNodeHash(NodeHash thash) : thash_(thash) {}

		/**
		 * Create an initial hash for an empty hypertrie. Empty tensors are stored in uncompressed nodes.
		 * @param depth depth of the hypertrie
		 */
		explicit TaggedNodeHash(pos_type depth) : thash_(absl::Hash<pos_type>()(depth) | uncompressed_tag) {}

		template<typename K, typename V>
		TaggedNodeHash(const K &key ,const V &value) : thash_((absl::Hash<pos_type>()(key) ^ absl::Hash<pos_type>()(value)) & notag_mask) {}


		[[nodiscard]] inline bool isCompressed() const {
			return thash_bits_[compression_tag_pos];
		}

		[[nodiscard]] inline bool isUncompressed() const {
			return not thash_bits_[compression_tag_pos];
		}

		template<typename V>
		inline void changeValue(const V &old_value, const V & new_value) {
			const bool tag = thash_bits_[compression_tag_pos];
			thash_ = thash_ ^ absl::Hash<pos_type>()(old_value) ^ absl::Hash<pos_type>()(new_value);
			thash_bits_[compression_tag_pos] = tag;
		}

		template<typename K, typename V>
		inline void addFirstEntry(const K &key, const V & value) {
			thash_ = thash_ ^ absl::Hash<pos_type>()(key) ^ absl::Hash<pos_type>()(value);
			thash_bits_[compression_tag_pos] = true;
		}

		template<typename K, typename V>
		inline void addEntry(const K &key, const V & value) {
			thash_ = thash_ ^ absl::Hash<pos_type>()(key) ^ absl::Hash<pos_type>()(value);
			thash_bits_[compression_tag_pos] = false;
		}

		template<typename K, typename V>
		inline void removeEntry(const K &key, const V & value, bool has_exactly_2_entries) {
			assert(isUncompressed());
			thash_ = thash_ ^ absl::Hash<pos_type>()(key) ^ absl::Hash<pos_type>()(value);
			thash_bits_[compression_tag_pos] = has_exactly_2_entries;
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

		operator bool() const {
			return this->thash_ != NodeHash{};
		}
	};


}
#endif //HYPERTRIE_TAGGEDNODEHASH_HPP
