#ifndef HYPERTRIE_FULLNODE_HPP
#define HYPERTRIE_FULLNODE_HPP

#include <cstddef>

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/ReferenceCounted.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleEntry.hpp>
#include <Dice/hypertrie/internal/raw/node/Valued.hpp>
#include <Dice/hypertrie/internal/raw/node/WithEdges.hpp>

namespace hypertrie::internal::raw {


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
					TensorHash<depth, tri> &hash = this->edges(pos)[key[pos]];
					hash.changeValue(sub_key, old_value, new_value);
				}
		}

		[[nodiscard]] inline size_t size() const noexcept { return size_; }

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


		FullNode(size_t ref_count, const allocator_type &alloc) noexcept
			: ReferenceCounted(ref_count), WithEdges<1UL, tri_t>(alloc) {}

		// TODO: should work automatically?
		//		FullNode(const FullNode &other) noexcept
		//			: ReferenceCounted(other.ref_count), WithEdges<1UL, tri_t>(other) {
		//			for (size_t pos : iter::range(1UL))
		//				for (const auto &entry : other)
		//					if constexpr (tri::is_bool_valued)
		//						this->edges(pos).insert(*entry);
		//					else
		//						this->edges(pos)[entry.first] = entry.second;
		//		}

		FullNode(const RawKey_t &key,
				 [[maybe_unused]] value_type value,
				 const RawKey_t &second_key,
				 [[maybe_unused]] value_type second_value,
				 size_t ref_count = 0) noexcept
			: ReferenceCounted(ref_count),
			  WithEdges<1UL, tri_t>([&]() {
				  if constexpr (std::is_same_v<value_type, bool>)
					  return EdgesType{{{key[0]},
										{second_key[0]}}};
				  else
					  return EdgesType{{{key[0], value},
										{second_key[0], second_value}}};
			  }()) {}

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
}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_FULLNODE_HPP
