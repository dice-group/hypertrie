#ifndef HYPERTRIE_NODE_HPP
#define HYPERTRIE_NODE_HPP

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
		using ChildrenType = typename tri::template map_type<typename tri::key_part_type, TaggedNodeHash>;
		using EdgesType = std::array<ChildrenType, depth>;

		EdgesType edges_;
		size_t size_ = 0;
		size_t ref_count_;

		[[nodiscard]] inline size_t size() const noexcept { return size_; }
	};

	// uncompressed depth == 1
	template<pos_type depth, typename tri_t>
	struct Node<depth, false, tri_t, typename std::enable_if_t<(
			(depth == 1) and not std::is_same_v<typename tri_t::value_type, bool>
	)>> {
		using tri = tri_t;
		// use a set to for value_type bool, otherwise a map
		using EdgesType = std::conditional_t<std::is_same_v<typename tri_t::value_type, bool>,
				typename tri::template set_type<typename tri::key_part_type>,
				typename tri::template map_type<typename tri::key_part_type, typename tri::value_type>
		>;


		EdgesType edges_;
		size_t ref_count_;

		[[nodiscard]] inline size_t size() const noexcept { return edges_.size(); }
	};


}

#endif //HYPERTRIE_NODE_HPP
