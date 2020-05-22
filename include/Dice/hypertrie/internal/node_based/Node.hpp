#ifndef HYPERTRIE_NODE_HPP
#define HYPERTRIE_NODE_HPP

#include <range.hpp>
#include "Dice/hypertrie/internal/util/PosType.hpp"
#include "Dice/hypertrie/internal/node_based/Hypertrie_internal_traits.hpp"
#include "Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp"

namespace hypertrie::internal::node_based {

	using NodeCompression = bool;
	constexpr NodeCompression COMPRESSED = true;
	constexpr NodeCompression UNCOMPRESSED = false;


	template<size_t depth,
			NodeCompression compressed,
			HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			typename  = typename std::enable_if_t<(depth >= 1)>>
	struct Node;

	// compressed nodes with boolean value_type
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, COMPRESSED, tri_t, typename std::enable_if_t<(
			(depth >= 1) and std::is_same_v<typename tri_t::value_type, bool>
	)>> {
		using tri = tri_t;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
	private:
		RawKey key_;
		size_t ref_count_ = 0;
	public:
		Node() = default;

		Node(const RawKey &key, size_t refCount = 0)
				: key_(key), ref_count_(refCount) {}

		RawKey &key() { return this->key_; }

		const RawKey &key() const { return this->key_; }

		size_t &ref_count() { return this->ref_count_; }

		const size_t &ref_count() const { return this->ref_count_; }

		[[nodiscard]] constexpr size_t size() const noexcept { return 1; }
	};

	// compressed nodes with non-boolean value_type
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, COMPRESSED, tri_t, typename std::enable_if_t<(
			(depth >= 1) and not std::is_same_v<typename tri_t::value_type, bool>
	)>> {
		using tri = tri_t;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
	private:
		RawKey key_;
		value_type value_;
		size_t ref_count_ = 0;
	public:
		Node(const RawKey &key, value_type value, size_t refCount = 0)
				: key_(key), value_(value), ref_count_(refCount) {}

		RawKey &key() { return this->key_; }

		const RawKey &key() const { return this->key_; }

		value_type &value() { return this->value_; }

		const value_type &value() const { return this->value_; }

		size_t &ref_count() { return this->ref_count_; }

		const size_t &ref_count() const { return this->ref_count_; }

		[[nodiscard]] constexpr size_t size() const noexcept { return 1; }
	};

	// uncompressed depth >= 2
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, UNCOMPRESSED, tri_t, typename std::enable_if_t<(
			(depth >= 2)
	)>> {
		using tri = tri_t;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
		using ChildrenType = typename tri::template map_type<typename tri::key_part_type, TaggedNodeHash>;
		using EdgesType = std::array<ChildrenType, depth>;

	private:
		static constexpr const auto subkey = &tri::template subkey<depth>;

		EdgesType edges_;
		size_t size_ = 0;
		size_t ref_count_ = 0;
	public:

		Node(size_t ref_count = 0) : edges_{}, ref_count_{ref_count} {}

		Node(const RawKey &key, value_type value, const RawKey &second_key, value_type second_value, size_t ref_count = 0)
				: size_{2}, ref_count_{ref_count} {
			for (const size_t pos : iter::range(depth))
				edges_[pos] = (key[pos] != second_key[pos]) ?
							  ChildrenType{
									  {key[pos],        TaggedNodeHash::getCompressedNodeHash<depth>(subkey(key, pos),
																									 value)},
									  {second_key[pos], TaggedNodeHash::getCompressedNodeHash<depth>(
											  subkey(second_key, pos),
											  second_value)},
							  } :
							  ChildrenType{
									  {key[pos], TaggedNodeHash::getTwoEntriesNodeHash<depth>(subkey(key, pos), value,
																							  subkey(second_key, pos),
																							  second_value)}};
		}

		void insertEntry(const RawKey &key, value_type value) {
			for (const size_t pos : iter::range(depth)) {
				auto &children = edges_[pos];
				auto key_part = key[pos];
				if (auto found = children.find(key_part); found != children.end()) {
					found.value().addEntry(subkey(key, pos), value);
				} else {
					children[key_part] = TaggedNodeHash::getCompressedNodeHash<depth>(subkey(key, pos), value);
				}
			}
		}

		EdgesType &edges() { return this->edges_; }

		const EdgesType &edges() const { return this->edges_; }

		ChildrenType &edges(pos_type pos) { return this->edges_[pos]; }

		const ChildrenType &edges(pos_type pos) const { return this->edges_[pos]; }

		size_t &ref_count() { return this->ref_count_; }

		const size_t &ref_count() const { return this->ref_count_; }

		[[nodiscard]] inline size_t size() const noexcept { return size_; }
	};

	// uncompressed depth == 1
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, UNCOMPRESSED, tri_t, typename std::enable_if_t<(
			(depth == 1)
	)>> {
		using tri = tri_t;

		using key_part_type = typename tri::key_part_type;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
		static constexpr const auto subkey = &tri::template subkey<depth>;
		// use a set to for value_type bool, otherwise a map
		using EdgesType = std::conditional_t<tri::is_bool_valued,
				typename tri::template set_type<key_part_type>,
				typename tri::template map_type<key_part_type, value_type>
		>;


		EdgesType edges_;
		size_t ref_count_ = 0;

		Node(size_t ref_count = 0) : edges_{}, ref_count_{ref_count} {}

		Node(const RawKey &key, [[maybe_unused]]value_type value, const RawKey &second_key,
			 [[maybe_unused]]value_type second_value, size_t ref_count = 0) : ref_count_{ref_count} {
			if constexpr(std::is_same_v<value_type, bool>)
				edges_ = EdgesType{
						{key[0]},
						{second_key[0]}
				};
			else
				edges_ = EdgesType{
						{key[0],        value},
						{second_key[0], value}
				};
		}

		void insertEntry(const RawKey &key, value_type value) {
			edges_[key[0]] = value;
		}

		EdgesType &edges() { return this->edges_; }

		const EdgesType &edges() const { return this->edges_; }

		EdgesType &edges([[maybe_unused]] pos_type pos) { return this->edges_; }

		const EdgesType &edges([[maybe_unused]] pos_type pos) const { return this->edges_; }

		size_t &ref_count() { return this->ref_count_; }

		const size_t &ref_count() const { return this->ref_count_; }

		[[nodiscard]] inline size_t size() const noexcept { return edges_.size(); }
	};

	template<size_t depth,
			HypertrieInternalTrait tri = Hypertrie_internal_t<>>
	using CompressedNode = Node<depth, COMPRESSED, tri>;

	template<size_t depth,
			HypertrieInternalTrait tri = Hypertrie_internal_t<>>
	using UncompressedNode = Node<depth, UNCOMPRESSED, tri>;


}

#endif //HYPERTRIE_NODE_HPP
