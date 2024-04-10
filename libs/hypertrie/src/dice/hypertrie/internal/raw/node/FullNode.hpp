#ifndef HYPERTRIE_FULLNODE_HPP
#define HYPERTRIE_FULLNODE_HPP

#include <cstddef>

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/ReferenceCounted.hpp"
#include "dice/hypertrie/internal/raw/node/SingleKey.hpp"
#include "dice/hypertrie/internal/raw/node/Valued.hpp"
#include "dice/hypertrie/internal/raw/node/WithEdges.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct FullNode : public ReferenceCounted, public WithEdges<depth, htt_t, allocator_type> {

		using RawKey_t = RawKey<depth, htt_t>;
		using value_type = typename htt_t::value_type;
		using WithEdges_t = WithEdges<depth, htt_t, allocator_type>;
		using map_alloc = typename WithEdges_t::collection_alloc;

	private:
		size_t size_ = 0;

	public:
		FullNode(size_t ref_count, const allocator_type &alloc) noexcept
			: ReferenceCounted(ref_count), WithEdges_t(alloc) {}


		void change_value(const RawKey_t &key, value_type old_value, value_type new_value) noexcept {
			if constexpr (not htt_t::is_bool_valued)
				for (size_t pos = 0; pos < depth; ++pos) {
					auto sub_key = key.subkey(pos);
					RawIdentifier<depth, htt_t> &identifier = this->edges(pos)[key[pos]];
					identifier.changeValue(sub_key, old_value, new_value);
				}
		}

		[[nodiscard]] size_t size() const noexcept { return size_; }
		[[nodiscard]] size_t &size() noexcept { return size_; }

		[[nodiscard]] bool operator==(const FullNode &other) const noexcept {
			// stored sizes are unequal
			if (this->size() != other.size())
				return false;
			auto min_size = std::numeric_limits<size_t>::max();
			pos_type min_pos = 0;
			// check if the other node maps by each position exactly the same amount of children
			// + find position with the least children
			for (size_t pos = 0; pos < depth; ++pos) {
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

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct FullNode<1UL, htt_t, allocator_type> : public ReferenceCounted, public WithEdges<1UL, htt_t, allocator_type> {

		using RawKey_t = RawKey<1UL, htt_t>;
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		using WithEdges_t = WithEdges<1UL, htt_t, allocator_type>;
		using map_alloc = typename WithEdges_t::collection_alloc;

		// use a set to for value_type bool, otherwise a map
		using EdgesType = typename WithEdges_t::ChildrenType;

		FullNode() = default;

		FullNode(size_t ref_count, const allocator_type &alloc) noexcept
			: ReferenceCounted(ref_count), WithEdges_t(alloc) {}

	private:
		//TODO: init list creates a array, gives two pointer into that array
		template<class T>
		void init(std::initializer_list<T> elements) noexcept {
			this->edges().reserve(elements.size());
			for (auto &e : elements) {
				if constexpr (std::is_same_v<std::decay_t<T>, RawKey_t>) {
					insert_or_assign(e);
				} else {
					insert_or_assign(e.first, e.second);
				}
			}
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
			init(entries);
		}

		void insert_or_assign(const RawKey_t &key, value_type value = value_type{1}) noexcept {
			insert_or_assign(key[0], value);
		}

		void insert_or_assign(key_part_type key_part, value_type value = value_type{1}) noexcept {
			if constexpr (htt_t::is_bool_valued)
				this->edges().insert(key_part);
			else
				this->edges().insert_or_assign(key_part, value);
		}

		void erase(const RawKey_t &key) noexcept {
			erase(key[0]);
		}

		void erase(key_part_type key_part) noexcept {
			this->edges().erase(key_part);
		}

		void change_value(const RawKey_t &key,
						  [[maybe_unused]] value_type old_value,
						  value_type new_value) noexcept {
			if constexpr (not htt_t::is_bool_valued)
				this->edges(0)[key[0]] = new_value;
		}

		[[nodiscard]] size_t size() const noexcept { return this->edges().size(); }

		[[nodiscard]] bool operator==(const FullNode &other) const noexcept {
			// stored sizes are unequal
			if (this->size() != other.size())
				return false;

			// check if children for that position are equal
			return this->edges() == other.edges();
		}
	};
}// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_FULLNODE_HPP
