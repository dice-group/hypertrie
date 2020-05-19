#ifndef HYPERTRIE_NODE_HPP
#define HYPERTRIE_NODE_HPP

#include <range.hpp>
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"
#include "TaggedNodeHash.hpp"

namespace hypertrie::internal::node_based {


	template<pos_type depth,
			bool compressed,
			typename tri_t = Hypertrie_internal_t<>,
			typename  = typename std::enable_if_t<(depth >= 1)>>
	struct Node;

	// compressed nodes with boolean value_type
	template<pos_type depth, typename tri_t>
	struct Node<depth, true, tri_t, typename std::enable_if_t<(
			(depth >= 1) and std::is_same_v<typename tri_t::value_type, bool>
	)>> {
		using tri = tri_t;

		std::array<typename tri::key_part, depth> key_;
		size_t ref_count_ = 1;

		Node(const std::array<typename tri::key_part, depth> &key,
			 size_t refCount = 0)
				: key_(key), ref_count_(refCount) {}

		[[nodiscard]] constexpr size_t size() const noexcept { return 1; }
	};

	// compressed nodes with non-boolean value_type
	template<pos_type depth, typename tri_t>
	struct Node<depth, true, tri_t, typename std::enable_if_t<(
			(depth >= 1) and not std::is_same_v<typename tri_t::value_type, bool>
	)>> {
		using tri = tri_t;

		std::array<typename tri::key_part, depth> key_;
		typename tri::value_type value_;
		size_t ref_count_;

		Node(const std::array<typename tri::key_part, depth> &key,
			 typename tri::value_type value,
			 size_t refCount = 0)
				: key_(key), value_(value), ref_count_(refCount) {}

		[[nodiscard]] constexpr size_t size() const noexcept { return 1; }
	};

	// uncompressed depth >= 2
	template<pos_type depth, typename tri_t>
	struct Node<depth, false, tri_t, typename std::enable_if_t<(
			(depth >= 2)
	)>> {
		using tri = tri_t;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type_t;
		static auto subkey = &tri::template subkey<depth>;

		using ChildrenType = typename tri::template map_type<typename tri::key_part_type, TaggedNodeHash>;
		using EdgesType = std::array<ChildrenType, depth>;

		EdgesType edges_;
		size_t size_ = 0;
		size_t ref_count_;

		Node() : edges_{}, ref_count_{}{}

		Node(const RawKey & key, value_type value, const RawKey & second_key, value_type second_value, size_t ref_count)
				: size_{2}, ref_count_{ref_count} {
			for (const pos_type pos : iter::range(depth))
				edges_[pos] = (key[pos] != second_key[pos]) ?
							  ChildrenType{
									  {key[pos],        TaggedNodeHash::getCompressedNodeHash(pos, key, value)},
									  {second_key[pos], TaggedNodeHash::getCompressedNodeHash(pos, second_key,
																							  second_value)},
							  } :
							  ChildrenType{
									  {key[pos], TaggedNodeHash::getTwoEntriesNodeHash(pos, key, value, second_key,
																					   second_value)}};
		}
		
		void insertEntry(const RawKey & key, value_type value) {
			for (const pos_type pos : iter::range(depth)){
				auto &children = edges_[pos];
				auto key_part = key[pos];
				if (auto found = children.find(key_part); found != children.end()){
					found.value().addEntry(subkey(key, pos), value);
				} else {
					children[key_part] = TaggedNodeHash::getCompressedNodeHash(pos, key, value);
				}
			}
		}

		[[nodiscard]] inline size_t size() const noexcept { return size_; }
	};

	// uncompressed depth == 1
	template<pos_type depth, typename tri_t>
	struct Node<depth, false, tri_t, typename std::enable_if_t<(
			(depth == 1) and not std::is_same_v<typename tri_t::value_type, bool>
	)>> {
		using tri = tri_t;

		using key_part_type = typename tri::key_part_type_t;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type_t;
		static auto subkey = &tri::template subkey<depth>;
		// use a set to for value_type bool, otherwise a map
		using EdgesType = std::conditional_t<std::is_same_v<value_type, bool>,
				typename tri::template set_type<key_part_type>,
				typename tri::template map_type<key_part_type, value_type>
		>;


		EdgesType edges_;
		size_t ref_count_;

		Node(const RawKey & key, [[maybe_unused]]value_type value, const RawKey & second_key, [[maybe_unused]]value_type second_value, size_t ref_count) : ref_count_{ref_count} {
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

		[[nodiscard]] inline size_t size() const noexcept { return edges_.size(); }
	};


}

#endif //HYPERTRIE_NODE_HPP
