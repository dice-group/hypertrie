#ifndef HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_CONTAINER_HPP
#define HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_CONTAINER_HPP

#include <dice/hypertrie/internal/raw/node/NodePtr.hpp>
#include <dice/hypertrie/internal/raw/node/RawIdentifier.hpp>

#include <ankerl/unordered_dense.h>

namespace dice::hypertrie::internal::raw::node_context::common_detail {

	template<typename T>
	struct Hash : ::ankerl::unordered_dense::hash<T> {
		static_assert(requires { typename ::ankerl::unordered_dense::hash<T>::is_avalanching; },
					  "Map or Set populated with key type that has non-avalanching hash. "
					  "Note: Performance degradation imminent. "
					  "Hint: Specialize dice::hypertrie::internal::raw::node_context::common_detail::Hash for your type. "
					  "Hint: See https://github.com/martinus/unordered_dense#31-hash for more details.");
	};

	template<size_t depth, HypertrieTrait htt_t>
	struct Hash<RawIdentifier<depth, htt_t>> {
		using is_avalanching = void;

		size_t operator()(RawIdentifier<depth, htt_t> const id) const noexcept {
			return hash::dice_hash_templates<hash::Policies::wyhash>::dice_hash(id.hash());
		}
	};

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct Hash<NodePtr<depth, htt_t, allocator_type>> {
		using is_avalanching = void;

		size_t operator()(NodePtr<depth, htt_t, allocator_type> const &ptr) const noexcept {
			return ptr.template hash<hash::Policies::wyhash>();
		}
	};

	template<typename T>
	struct Hash<boost::interprocess::offset_ptr<T>> {
		using is_avalanching = void;

		size_t operator()(boost::interprocess::offset_ptr<T> const &ptr) const noexcept {
			return hash::dice_hash_templates<hash::Policies::wyhash>::dice_hash(std::to_address(ptr));
		}
	};

	template<size_t depth, HypertrieTrait htt_t>
	struct Hash<SingleEntry<depth, htt_t>> {
		using is_avalanching = void;

		size_t operator()(SingleEntry<depth, htt_t> const &se) const noexcept {
			return hash::dice_hash_templates<hash::Policies::wyhash>::dice_hash(se);
		}
	};

	template<typename K, typename V, typename H = Hash<K>, typename Eq = std::equal_to<K>>
	using Map = ::ankerl::unordered_dense::map<K, V, H, Eq>;

	template<typename V, typename H = Hash<V>, typename Eq = std::equal_to<V>>
	using Set = ::ankerl::unordered_dense::set<V, H, Eq>;

} // namespace dice::hypertrie::internal::raw::node_context::common_detail

#endif // HYPERTRIE_RAWNODECONTEXT_COMMON_DETAIL_CONTAINER_HPP
