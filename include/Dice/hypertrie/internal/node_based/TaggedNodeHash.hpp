#ifndef HYPERTRIE_TAGGEDNODEHASH_HPP
#define HYPERTRIE_TAGGEDNODEHASH_HPP

#include <compare>

#include <absl/hash/hash.h>

namespace hypertrie::internal::node_based {

	using NodeHash = size_t;

	class TaggedNodeHash {
		NodeHash thash_{};

		static constexpr size_t tag_mask = size_t(1);
		static constexpr size_t notag_mask = ~size_t(1);
		static constexpr size_t uncompressed_tag = size_t(0);
		static constexpr size_t compressed_tag = size_t(1);

	public:
		TaggedNodeHash() = default;

		explicit TaggedNodeHash(NodeHash thash) : thash_(thash) {}

		/**
		 * Create an initial hash for an empty hypertrie. Empty tensors are stored in uncompressed nodes.
		 * @param depth depth of the hypertrie
		 */
		explicit TaggedNodeHash(pos_type depth) : thash_(absl::Hash<pos_type>()(depth) | uncompressed_tag) {}

		template<typename K, typename V>
		TaggedNodeHash(const K &key ,const V &value) : thash_((absl::Hash<pos_type>()(key) ^ absl::Hash<pos_type>()(value)) & notag_mask) {}


		[[nodiscard]] inline bool isCompressed() const {
			return (thash_ & tag_mask) == compressed_tag;
		}

		[[nodiscard]] inline bool isUncompressed() const {
			return (thash_ & tag_mask) == uncompressed_tag;
		}

		template<typename T>
		inline void addToHash(const T &hashable) {
			size_t key_hash = absl::Hash<pos_type>()(hashable);
			thash_ = (thash_ ^ key_hash) // xor for combining the hash
					 | compressed_tag;  // or for setting compressed tag
		}

		template<typename T>
		inline void removeFromHash(const T &hashable, bool has2entries) {
			size_t key_hash = absl::Hash<pos_type>()(hashable);
			thash_ = (thash_ ^ key_hash); // xor for combining the hash
			if (has2entries)
				thash_ &= notag_mask; // for setting uncompressed tag
		}

		auto operator<=>(const TaggedNodeHash &other) const  = default;


	};


}
#endif //HYPERTRIE_TAGGEDNODEHASH_HPP
