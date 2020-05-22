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

	struct ReferenceCounted {
	protected:
		size_t ref_count_ = 0;
	public:
		ReferenceCounted() {}

		ReferenceCounted(size_t ref_count) : ref_count_(ref_count) {}

		size_t &ref_count() { return this->ref_count_; }

		const size_t &ref_count() const { return this->ref_count_; }
	};

	template<size_t depth, HypertrieInternalTrait tri>
	struct Compressed {
		using RawKey = typename tri::template RawKey<depth>;
	protected:
		RawKey key_{};
	public:
		Compressed() {}

		Compressed(RawKey key) : key_(key) {}

		RawKey &key() { return this->key_; }

		const RawKey &key() const { return this->key_; }

		[[nodiscard]] constexpr size_t size() const noexcept { return 1; }
	};

	template<HypertrieInternalTrait tri>
	struct Valued {
		using value_type = typename tri::value_type;
	protected:
		value_type value_;
	public:
		Valued() {}

		Valued(value_type value) : value_(value) {}

		value_type &value() { return this->value_; }

		const value_type &value() const { return this->value_; }
	};

	template<size_t depth, HypertrieInternalTrait tri>
	struct WithEdges {

		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
		using key_part_type = typename tri::key_part_type;
		template<typename K, typename V>
		using map_type = typename tri::template map_type<K, V>;

		using ChildrenType = std::conditional_t<(depth > 1),
				typename tri::template map_type<typename tri::key_part_type, TaggedNodeHash>,
				std::conditional_t<tri::is_bool_valued,
						typename tri::template set_type<key_part_type>,
						typename tri::template map_type<key_part_type, value_type>>
		>;

		using EdgesType = std::conditional_t<(depth > 1),
				std::array<ChildrenType, depth>,
				ChildrenType>;

	protected:
		EdgesType edges_;
	public:
		WithEdges() : edges_{} {}

		WithEdges(EdgesType edges) : edges_{edges} {}

		void insertEntry(const RawKey &key, value_type value);

		EdgesType &edges() { return this->edges_; }

		const EdgesType &edges() const { return this->edges_; }

		ChildrenType &edges(pos_type pos) {
			if constexpr(depth > 1) return this->edges_[pos]; else return this->edges_;
		}

		const ChildrenType &edges(pos_type pos) const {
			if constexpr(depth > 1) return this->edges_[pos]; else return this->edges_;
		}
	};


	template<size_t depth,
			NodeCompression compressed,
			HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			typename  = typename std::enable_if_t<(depth >= 1)>>
	struct Node;

	// compressed nodes with boolean value_type
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, COMPRESSED, tri_t, typename std::enable_if_t<(
			(depth >= 1) and tri_t::is_bool_valued
	)>> : public ReferenceCounted, public Compressed<depth, tri_t> {
		using tri = tri_t;
		using RawKey = typename Compressed<depth, tri_t>::RawKey;
	public:
		Node() = default;

		Node(const RawKey &key, size_t ref_count = 0)
				: ReferenceCounted(ref_count), Compressed<depth, tri_t>(key) {}
	};

	// compressed nodes with non-boolean value_type
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, COMPRESSED, tri_t, std::enable_if_t<(
			(depth >= 1) and not tri_t::is_bool_valued
	)>> : public ReferenceCounted, public Compressed<depth, tri_t>, Valued<tri_t> {
		using tri = tri_t;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
	public:
		Node(const RawKey &key, value_type value, size_t ref_count = 0)
				: ReferenceCounted(ref_count), Compressed<depth, tri_t>(key), Valued<tri_t>(value) {}
	};

	// uncompressed depth >= 2
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, UNCOMPRESSED, tri_t, typename std::enable_if_t<(
			(depth >= 2)
	)>> : public ReferenceCounted, public WithEdges<depth, tri_t> {
		using tri = tri_t;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
		using ChildrenType = typename WithEdges<depth, tri_t>::ChildrenType;
		using EdgesType = typename WithEdges<depth, tri_t>::EdgesType;

	private:
		static constexpr const auto subkey = &tri::template subkey<depth>;
		size_t size_ = 0;
	public:

		Node(size_t ref_count = 0) : ReferenceCounted(ref_count) {}

		Node(const RawKey &key, value_type value, const RawKey &second_key, value_type second_value,
			 size_t ref_count = 0)
				: size_{2}, ReferenceCounted(ref_count) {
			for (const size_t pos : iter::range(depth))
				this->edges(pos) = (key[pos] != second_key[pos]) ?
								   ChildrenType{
										   {key[pos],        TaggedNodeHash::getCompressedNodeHash<depth>(
												   subkey(key, pos),
												   value)},
										   {second_key[pos], TaggedNodeHash::getCompressedNodeHash<depth>(
												   subkey(second_key, pos),
												   second_value)},
								   } :
								   ChildrenType{
										   {key[pos], TaggedNodeHash::getTwoEntriesNodeHash<depth>(subkey(key, pos),
																								   value,
																								   subkey(second_key,
																										  pos),
																								   second_value)}};
		}

		void insertEntry(const RawKey &key, value_type value) {
			for (const size_t pos : iter::range(depth)) {
				auto &children = this->edges(pos);
				auto key_part = key[pos];
				if (auto found = children.find(key_part); found != children.end()) {
					found.value().addEntry(subkey(key, pos), value);
				} else {
					children[key_part] = TaggedNodeHash::getCompressedNodeHash<depth>(subkey(key, pos), value);
				}
			}
		}

		[[nodiscard]] inline size_t size() const noexcept { return size_; }
	};

	// uncompressed depth == 1
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, UNCOMPRESSED, tri_t, typename std::enable_if_t<(
			(depth == 1)
	)>> : public ReferenceCounted, public WithEdges<depth, tri_t> {
		using tri = tri_t;

		using key_part_type = typename tri::key_part_type;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
		static constexpr const auto subkey = &tri::template subkey<depth>;
		// use a set to for value_type bool, otherwise a map
		using EdgesType = typename WithEdges<depth, tri_t>::ChildrenType;


		Node(size_t ref_count = 0) : ReferenceCounted(ref_count) {}

		Node(const RawKey &key, [[maybe_unused]]value_type value, const RawKey &second_key,
			 [[maybe_unused]]value_type second_value, size_t ref_count = 0)
				: ReferenceCounted(ref_count),
				  WithEdges<depth, tri_t>([&]() {
					  if constexpr(std::is_same_v<value_type, bool>)
						  return EdgesType{
								  {key[0]},
								  {second_key[0]}
						  };
					  else
						  return EdgesType{
								  {key[0],        value},
								  {second_key[0], value}
						  };
				  }()) {}

		void insertEntry(const RawKey &key, [[maybe_unused]] value_type value) {
			if constexpr(tri::is_bool_valued)
				this->edges().insert(key[0]);
			else
			this->edges()[key[0]] = value;
		}

		[[nodiscard]] inline size_t size() const noexcept { return this->edges().size(); }
	};

	template<size_t depth,
			HypertrieInternalTrait tri = Hypertrie_internal_t<>>
	using CompressedNode = Node<depth, COMPRESSED, tri>;

	template<size_t depth,
			HypertrieInternalTrait tri = Hypertrie_internal_t<>>
	using UncompressedNode = Node<depth, UNCOMPRESSED, tri>;


}

#endif //HYPERTRIE_NODE_HPP
