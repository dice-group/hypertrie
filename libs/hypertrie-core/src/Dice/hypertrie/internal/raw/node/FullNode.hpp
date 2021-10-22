#ifndef HYPERTRIE_FULLNODE_HPP
#define HYPERTRIE_FULLNODE_HPP

#include <cstddef>

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/ReferenceCounted.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleKey.hpp>
#include <Dice/hypertrie/internal/raw/node/Valued.hpp>
#include <Dice/hypertrie/internal/raw/node/WithEdges.hpp>

namespace Dice::hypertrie::internal::raw {


	template<size_t depth, HypertrieCoreTrait tri_t>
	struct FullNode : public ReferenceCounted, public WithEdges<depth, tri_t> {
		using tri = tri_t;
		using RawKey_t = RawKey<depth, tri_t>;
		using value_type = typename tri::value_type;
		using allocator_type = typename tri::allocator_type;
		using WithEdges_t = WithEdges<depth, tri_t>;
		using map_alloc = typename WithEdges_t::collection_alloc;

	private:
		size_t size_ = 0;

	public:
		FullNode(size_t ref_count, const allocator_type &alloc) noexcept
			: ReferenceCounted(ref_count), WithEdges<depth, tri_t>(alloc) {}


		void change_value(const RawKey_t &key, value_type old_value, value_type new_value) noexcept {
			if constexpr (not tri::is_bool_valued)
				for (const size_t pos : iter::range(depth)) {
					auto sub_key = key.subkey(pos);
					RawIdentifier<depth, tri> &identifier = this->edges(pos)[key[pos]];
					identifier.changeValue(sub_key, old_value, new_value);
				}
		}

		[[nodiscard]] inline size_t size() const noexcept { return size_; }
		[[nodiscard]] inline size_t &size()  noexcept { return size_; }

		auto operator==(const FullNode &other) const noexcept {
			// stored sizes are unequal
			if (this->size() != other.size())
				return false;


			auto min_size = std::numeric_limits<size_t>::max();
			pos_type min_pos = 0;
			// check if the other node maps by each position exactly the same amount of children
			// + find position with the least children
			for (pos_type pos : iter::range(depth)) {
				const auto size_at_pos = this->edges(pos).size();
				if (size_at_pos != other.edges(pos).size())
					return false;
				if (size_at_pos < min_size) {
					min_size = size_at_pos;
					min_pos = pos;
				}
			}
			// check if children for that position are equal
			return this->edges(min_pos) == other.edges(min_pos);
		}
	};

	template<HypertrieCoreTrait tri_t>
	struct FullNode<1UL, tri_t> : public ReferenceCounted, public WithEdges<1UL, tri_t> {
		using tri = tri_t;
		using RawKey_t = RawKey<1UL, tri_t>;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using allocator_type = typename tri::allocator_type;
		using WithEdges_t = WithEdges<1UL, tri_t>;
		using map_alloc = typename WithEdges_t::collection_alloc;

		// use a set to for value_type bool, otherwise a map
		using EdgesType = typename WithEdges<1UL, tri_t>::ChildrenType;

		FullNode() = default;

		FullNode(size_t ref_count, const allocator_type &alloc) noexcept
			: ReferenceCounted(ref_count), WithEdges<1UL, tri_t>(alloc) {}

	private:
		template<class T>
		void init(std::initializer_list<T> elements) noexcept {
			this->edges().reserve(elements.size());
			for (auto &e : elements)
				if constexpr (std::is_same_v<std::decay_t<T>, RawKey_t>)
					insert_or_assign(e);
				else
					insert_or_assign(e.first, e.second);
		}

	public:
		FullNode(std::initializer_list<RawKey_t> keys,
				 size_t ref_count,
				 const allocator_type &alloc) noexcept
			: FullNode(ref_count, alloc) {
			init(std::move(keys));
		}

		FullNode(std::initializer_list<std::pair<RawKey_t, value_type>> entries,
				 size_t ref_count,
				 const allocator_type &alloc) noexcept
			: FullNode(ref_count, alloc) {
			init(std::move(entries));
		}

		inline void insert_or_assign(const RawKey_t &key, value_type value = value_type{1}) noexcept {
			insert_or_assign(key[0], value);
		}

		inline void insert_or_assign(key_part_type key_part, value_type value = value_type{1}) noexcept {
			if constexpr (tri::is_bool_valued)
				this->edges().insert(key_part);
			else
				this->edges().insert_or_assign(key_part, value);
		}

		inline void erase(const RawKey_t &key) noexcept {
			erase(key[0]);
		}

		inline void erase(key_part_type key_part) noexcept {
			this->edges().erase(key_part);
		}

		void change_value(const RawKey_t &key,
						  [[maybe_unused]] value_type old_value,
						  value_type new_value) noexcept {
			if constexpr (not tri::is_bool_valued)
				this->edges(0)[key[0]] = new_value;
		}
		[[nodiscard]] inline size_t size() const noexcept { return this->edges().size(); }

		auto operator==(const FullNode &other) const noexcept {
			// stored sizes are unequal
			if (this->size() != other.size())
				return false;

			// check if children for that position are equal
			return this->edges() == other.edges();
		}
	};
}// namespace Dice::hypertrie::internal::raw

#endif//HYPERTRIE_FULLNODE_HPP
