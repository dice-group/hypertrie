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
	};

}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_FULLNODE_HPP
