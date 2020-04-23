#ifndef HYPERTRIE_TAGGEDNODEHASH_HPP
#define HYPERTRIE_TAGGEDNODEHASH_HPP

#include <absl/hash/hash.h>

namespace hypertrie::internal::node_based {

	using NodeHash = size_t;

	class TaggedNodeHash {
		NodeHash thash_;

		static constexpr size_t tag_mask = size_t(1);
		static constexpr size_t nontag_mask = ~size_t(1);
		static constexpr size_t uncompressed_tag = size_t(0);
		static constexpr size_t compressed_tag = size_t(1);

	public:
		TaggedNodeHash(NodeHash thash) : thash_(thash) {}

		/**
		 * Create an initial hash for an empty hypertrie. Empty tensors are stored in uncompressed nodes.
		 * @param depth depth of the hypertrie
		 */
		explicit TaggedNodeHash(pos_type depth) : thash_(absl::Hash<pos_type>()(depth) | uncompressed_tag) {}


		[[nodiscard]] inline bool isCompressed() const {
			return (thash_ & tag_mask) == compressed_tag;
		}

		[[nodiscard]] inline bool isUncompressed() const {
			return (thash_ & tag_mask) == uncompressed_tag;
		}

		template<pos_type depth,
				typename tri = Hypertrie_internal_t<>>
		inline void addKey(const typename tri::template RawKey<depth> &key) {
			size_t key_hash = absl::Hash<pos_type>()(depth)(key);
			thash_ = (thash_ ^ key_hash) // xor for combining the hash
					 | compressed_tag;  // or for setting compressed tag
		}
	};


}
#endif //HYPERTRIE_TAGGEDNODEHASH_HPP
