#ifndef HYPERTRIE_KEYPARTNODEHASHVARIANT_HPP
#define HYPERTRIE_KEYPARTNODEHASHVARIANT_HPP

#include <bitset>
#include <compare>

#include "Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp"
#include "Dice/hypertrie/internal/raw/node/TensorHash.hpp"

namespace hypertrie::internal::raw {


	/**
	 * Tagged Tensor Hash is a wrapper around TensorHash.
	 * If tri::is_bool_valued and tri::is_lsb_unused it stores instead of a compressed nodes hash directly the key-part of a depth 1 node.
	 * @tparam tri
	 */
	template<size_t depth, HypertrieCoreTrait tri_t>
	class Hash_or_InplaceNode {
	public:
		using tri = tri_t;
		using TensorHash_t = TensorHash<depth, tri>;
		using key_part_type = typename tri::key_part_type;

		TensorHash_t hash_;

		Hash_or_InplaceNode() noexcept = default;

		explicit Hash_or_InplaceNode(const TensorHash_t &node_hash) noexcept : hash_(node_hash) {}

		explicit Hash_or_InplaceNode(const key_part_type &key_part) noexcept : hash_(reinterpret_cast<RawTensorHash>(key_part)) {
			// set sen bit
			hash_.hash() |= (size_t(1) << TensorHash_t::tag_pos);
		}

		Hash_or_InplaceNode &operator=(const TensorHash_t &node_hash) noexcept {
			hash_ = node_hash;
			return *this;
		}

	public:
		/**
		 * Checks if hash is tagged as representing a single entry node.
		 * @return
		 */
		[[nodiscard]] inline bool is_sen() const noexcept {
			return bool(hash_ & (size_t(1) << TensorHash_t::tag_pos));
		}

		[[nodiscard]] inline key_part_type getKeyPart() const noexcept {
			TensorHash hash_copy = hash_;
			hash_copy = hash_copy & ~(size_t(1) << TensorHash_t::tag_pos);
			return reinterpret_cast<key_part_type>(hash_copy.hash());
		}

		/**
		 * Checks if hash is tagged as representing a full node.
		 * @return
		 */
		[[nodiscard]] inline bool is_fn() const noexcept {
			return not is_sen();
		}

		[[nodiscard]] inline const TensorHash_t &getTaggedNodeHash() const noexcept {
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
		inline auto addFirstEntry(const RawKey<1, tri> &key, [[maybe_unused]] const bool &value) noexcept {
			hash_ = reinterpret_cast<RawTensorHash>(key[0]);
			hash_.hash() |= (1UL << TensorHash_t::tag_pos);
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
		inline auto addEntry(const RawKey<1, tri> &key, [[maybe_unused]] const bool &value) noexcept {
			assert(value);
			if (hash_.is_sen()) {
				hash_ = TensorHash_t::getCompressedNodeHash(getKeyPart(), true);
			}
			hash_.addEntry(key, value);
			return *this;
		}

		bool operator<(const Hash_or_InplaceNode &other) const noexcept {
			return this->hash_ < other.hash_;
		}

		bool operator==(const Hash_or_InplaceNode &other) const noexcept {
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

		explicit operator TensorHash_t() const noexcept {
			return this->hash_;
		}

		/**
		 * The internally used hash.
		 * @return
		 */
		[[nodiscard]] const RawTensorHash &hash() const noexcept {
			return this->hash_.hash();
		}
	};

}// namespace hypertrie::internal::raw

namespace std {

	template<size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct hash<::hypertrie::internal::raw::Hash_or_InplaceNode<depth, tri>> {
		size_t operator()(const ::hypertrie::internal::raw::Hash_or_InplaceNode<depth, tri> &hash) const noexcept {
			return hash.hash();
		}
	};
}// namespace std

namespace Dice::hash {
	template<typename Policy, size_t depth, ::hypertrie::internal::raw::HypertrieCoreTrait tri>
	struct dice_hash_overload<Policy, ::hypertrie::internal::raw::Hash_or_InplaceNode<depth, tri>> {
		static std::size_t dice_hash(::hypertrie::internal::raw::Hash_or_InplaceNode<depth, tri> const &hash) noexcept {
			return hash.hash();
		}
	};
}// namespace Dice::hash

#endif//HYPERTRIE_KEYPARTNODEHASHVARIANT_HPP
