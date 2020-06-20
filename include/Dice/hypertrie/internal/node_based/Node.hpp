#ifndef HYPERTRIE_NODE_HPP
#define HYPERTRIE_NODE_HPP

#include "Dice/hypertrie/internal/node_based/Hypertrie_internal_traits.hpp"
#include "Dice/hypertrie/internal/node_based/NodeCompression.hpp"
#include "Dice/hypertrie/internal/node_based/TaggedNodeHash.hpp"
#include "Dice/hypertrie/internal/util/PosType.hpp"
#include <range.hpp>

namespace hypertrie::internal::node_based {

	/**
	 * A super class to provide counting references in a Node.
	 */
	struct ReferenceCounted {
	protected:
		size_t ref_count_ = 0;

	public:
		/**
		 * Default constructor. ref_count is set to 0.
		 */
		ReferenceCounted() {}

		/**
		 * ref_count is set to the given value.
		 * @param ref_count ref_count value
		 */
		ReferenceCounted(size_t ref_count) : ref_count_(ref_count) {}
		/**
		 * Modifiable reference to ref_count.
		 * @return
		 */
		size_t &ref_count() { return this->ref_count_; }

		/**
		 * Constant reference to ref_count.
		 * @return
		 */
		const size_t &ref_count() const { return this->ref_count_; }
	};

	/**
	 * A super class to provide a single key in a Node.
	 * @tparam depth depth of the key
	 * @tparam tri HypertrieInternalTrait that defines node parameters
	 */
	template<size_t depth, HypertrieInternalTrait tri>
	struct Compressed {
		using RawKey = typename tri::template RawKey<depth>;

	protected:
		RawKey key_;

	public:
		/**
		 * Default constructor fills the key with 0, 0.0, true (value initialization)
		 */
		Compressed() : key_{} {}

		/**
		 * Uses the provided RawKey as key.
		 * @param key
		 */
		Compressed(RawKey key) : key_(key) {}

		/**
		 * Modifiable reference to key.
		 * @return
		 */
		RawKey &key() { return this->key_; }

		/**
		 * Constant reference to key.
		 * @return
		 */
		const RawKey &key() const { return this->key_; }

		/**
		 * Size of this node (It is always 1).
		 * @return
		 */
		[[nodiscard]] constexpr size_t size() const noexcept { return 1; }
	};

	/**
	 * A super class to provide a single value in a Node.
	 * @tparam tri HypertrieInternalTrait that defines node parameters
	 */
	template<HypertrieInternalTrait tri>
	struct Valued {
		using value_type = typename tri::value_type;

	protected:
		value_type value_;

	public:
		/**
		 * Default constructor sets value to 0, 0.0, true (value initialization)
		 */
		Valued() : value_{} {}

		/**
		 * Uses the provided value.
		 * @param key
		 */
		Valued(value_type value) : value_(value) {}

		/**
		 * Modifiable reference to value.
		 * @return
		 */
		value_type &value() { return this->value_; }

		/**
		 * Constant reference to value.
		 * @return
		 */
		const value_type &value() const { return this->value_; }
	};

	template<size_t depth, HypertrieInternalTrait tri>
	struct WithEdges {

		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
		using key_part_type = typename tri::key_part_type;
		template<typename K, typename V>
		using map_type = typename tri::template map_type<K, V>;

		using ChildType = std::conditional_t<(depth > 1),
											 TaggedNodeHash,
											 value_type>;

		using ChildrenType = std::conditional_t<((depth == 1) and tri::is_bool_valued),
												typename tri::template set_type<key_part_type>,
												typename tri::template map_type<typename tri::key_part_type, ChildType>>;

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

		ChildrenType &edges(size_t pos) {
			assert(pos < depth);
			if constexpr (depth > 1)
				return this->edges_[pos];
			else
				return this->edges_;
		}

		const ChildrenType &edges(size_t pos) const {
			assert(pos < depth);
			if constexpr (depth > 1)
				return this->edges_[pos];
			else
				return this->edges_;
		}

		std::pair<bool, typename ChildrenType::iterator> find(size_t pos, key_part_type key_part) {
			auto found = this->edges(pos).find(key_part);
			return {found != this->edges(pos).end(), found};
		}

		std::pair<bool, typename ChildrenType::const_iterator> find(size_t pos, key_part_type key_part) const {
			auto found = this->edges(pos).find(key_part);
			return {found != this->edges(pos).end(), found};
		}

		ChildType child(size_t pos, key_part_type key_part) const {
			if (auto [found, iter] = this->find(pos, key_part); found) {
				if constexpr ((depth == 1) and tri::is_bool_valued)
					return true;
				else
					return iter->second;
			} else {
				return ChildType{};// 0, 0.0, false
			}
		}
	};


	template<size_t depth,
			 NodeCompression compressed,
			 HypertrieInternalTrait tri_t = Hypertrie_internal_t<>,
			 typename = typename std::enable_if_t<(depth >= 1)>>
	struct Node;

	// compressed nodes with boolean value_type
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, NodeCompression::compressed, tri_t, typename std::enable_if_t<((depth >= 1) and tri_t::is_bool_valued)>> : public ReferenceCounted, public Compressed<depth, tri_t> {
		using tri = tri_t;
		using RawKey = typename Compressed<depth, tri_t>::RawKey;

	public:
		Node() = default;

		Node(const RawKey &key, size_t ref_count = 0)
			: ReferenceCounted(ref_count), Compressed<depth, tri_t>(key) {}

		constexpr bool value() const { return true; }

		auto print() const {
			return fmt::format("<Node depth = {}, compressed> {{\n"
							   "\t{},\n"
							   "\tref_count = {} }}",
							   depth,
							   this->key(),
							   this->ref_count());
		}
	};

	// compressed nodes with non-boolean value_type
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, NodeCompression::compressed, tri_t, std::enable_if_t<((depth >= 1) and not tri_t::is_bool_valued)>> : public ReferenceCounted, public Compressed<depth, tri_t>, Valued<tri_t> {
		using tri = tri_t;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;

	public:
		Node(const RawKey &key, value_type value, size_t ref_count = 0)
			: ReferenceCounted(ref_count), Compressed<depth, tri_t>(key), Valued<tri_t>(value) {}

		auto print() const {
			return fmt::format("<Node depth = {}, compressed> {{\n"
							   "\t{} -> {},\n"
							   "\tref_count = {} }}",
							   depth,
							   this->key(), this->value(),
							   this->ref_count());
		}
	};

	// uncompressed depth >= 2
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, NodeCompression::uncompressed, tri_t, typename std::enable_if_t<((depth >= 2))>> : public ReferenceCounted, public WithEdges<depth, tri_t> {
		using tri = tri_t;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
		using ChildrenType = typename WithEdges<depth, tri_t>::ChildrenType;
		using EdgesType = typename WithEdges<depth, tri_t>::EdgesType;

	private:
		static constexpr const auto subkey = &tri::template subkey<depth>;
		static constexpr const auto deref = &tri::template deref<typename ChildrenType::key_type, typename ChildrenType::mapped_type>;
		size_t size_ = 0;

	public:
		Node(size_t ref_count = 0) : ReferenceCounted(ref_count) {}

		Node(const RawKey &key, value_type value, const RawKey &second_key, value_type second_value,
			 size_t ref_count = 0)
			: ReferenceCounted(ref_count), size_{2} {
			for (const size_t pos : iter::range(depth))
				this->edges(pos) = (key[pos] != second_key[pos])
										   ? ChildrenType{{{key[pos], TaggedNodeHash::getCompressedNodeHash(subkey(key, pos), value)},
														   {second_key[pos], TaggedNodeHash::getCompressedNodeHash(subkey(second_key, pos), second_value)}}}
										   : ChildrenType{{{key[pos], TaggedNodeHash::getTwoEntriesNodeHash(
																			  subkey(key, pos), value,
																			  subkey(second_key, pos), second_value)}}};
		}

		void change_value(const RawKey &key, value_type old_value, value_type new_value) {
			if constexpr (not tri::is_bool_valued)
				for (const size_t pos : iter::range(depth)) {
					auto sub_key = subkey(key, pos);
					TaggedNodeHash &hash = this->edges(pos)[key[pos]];
					hash.changeValue(sub_key, old_value, new_value);
				}
		}

		void insertEntry(const RawKey &key, value_type value) {
			this->size_++;
			for (const size_t pos : iter::range(depth)) {
				auto &children = this->edges(pos);
				auto key_part = key[pos];
				if (typename ChildrenType::iterator found = children.find(key_part); found != children.end()) {
					deref(found).addEntry(subkey(key, pos), value);
				} else {
					children[key_part] = TaggedNodeHash::getCompressedNodeHash(subkey(key, pos), value);
				}
			}
		}

		[[nodiscard]] inline size_t size() const noexcept { return size_; }

		auto print() const {
			fmt::memory_buffer out;
			fmt::format_to(out, "<Node depth = {}, uncompressed> {{", depth);
			for (size_t pos : iter::range(depth)) {
				fmt::format_to(out, "\n\tedges[{}] = {}", pos, this->edges(pos));
			}
			fmt::format_to(out, ",\n\tref_count = {}", this->ref_count());
			fmt::format_to(out, " }}");
			return fmt::to_string(out);
		}
	};

	// uncompressed depth == 1
	template<size_t depth, HypertrieInternalTrait tri_t>
	struct Node<depth, NodeCompression::uncompressed, tri_t, typename std::enable_if_t<((depth == 1))>> : public ReferenceCounted, public WithEdges<depth, tri_t> {
		using tri = tri_t;

		using key_part_type = typename tri::key_part_type;
		using RawKey = typename tri::template RawKey<depth>;
		using value_type = typename tri::value_type;
		static constexpr const auto subkey = &tri::template subkey<depth>;
		// use a set to for value_type bool, otherwise a map
		using EdgesType = typename WithEdges<depth, tri_t>::ChildrenType;


		Node(size_t ref_count = 0) : ReferenceCounted(ref_count) {}

		Node(const RawKey &key, [[maybe_unused]] value_type value, const RawKey &second_key,
			 [[maybe_unused]] value_type second_value, size_t ref_count = 0)
			: ReferenceCounted(ref_count),
			  WithEdges<depth, tri_t>([&]() {
				  if constexpr (std::is_same_v<value_type, bool>)
					  return EdgesType{{{key[0]},
										{second_key[0]}}};
				  else
					  return EdgesType{{{key[0], value},
										{second_key[0], second_value}}};
			  }()) {}

		void insertEntry(const RawKey &key, [[maybe_unused]] value_type value) {
			if constexpr (tri::is_bool_valued)
				this->edges().insert(key[0]);
			else
				this->edges()[key[0]] = value;
		}

		void change_value(const RawKey &key, [[maybe_unused]] value_type old_value, value_type new_value) {
			if constexpr (not tri::is_bool_valued)
				this->edges(0)[key[0]] = new_value;
		}

		[[nodiscard]] inline size_t size() const noexcept { return this->edges().size(); }

		auto print() const {
			return fmt::format("<Node depth = {}, uncompressed> {{\n"
							   "\tedges[0] = {},\n"
							   "\tref_count = {} }}",
							   depth,
							   this->edges(0),
							   this->ref_count());
		}
	};

	template<size_t depth,
			 HypertrieInternalTrait tri = Hypertrie_internal_t<>>
	using CompressedNode = Node<depth, NodeCompression::compressed, tri>;

	template<size_t depth,
			 HypertrieInternalTrait tri = Hypertrie_internal_t<>>
	using UncompressedNode = Node<depth, NodeCompression::uncompressed, tri>;


}// namespace hypertrie::internal::node_based

template<size_t depth, NodeCompression compressed, hypertrie::internal::node_based::HypertrieInternalTrait tri_t, typename enabled>
std::ostream &operator<<(std::ostream &os, const hypertrie::internal::node_based::Node<depth, compressed, tri_t, enabled> &node) {
	return os << fmt::format("{}", node.print());
}

template<size_t depth, NodeCompression compressed, hypertrie::internal::node_based::HypertrieInternalTrait tri_t, typename enabled>
struct fmt::formatter<hypertrie::internal::node_based::Node<depth, compressed, tri_t, enabled>> {
private:
	using node_type = hypertrie::internal::node_based::Node<depth, compressed, tri_t, enabled>;

public:
	auto parse(format_parse_context &ctx) { return ctx.begin(); }

	template<typename FormatContext>
	auto format(const node_type &node, FormatContext &ctx) { return fmt::format_to(ctx.out(), "{}", node.print()); }
};

#endif//HYPERTRIE_NODE_HPP
