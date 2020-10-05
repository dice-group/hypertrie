#ifndef HYPERTRIE_TESTSLICE_HPP
#define HYPERTRIE_TESTSLICE_HPP

#include <Dice/hypertrie/internal/raw/storage/NodeContext.hpp>
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <itertools.hpp>
#include <tsl/sparse_map.h>

namespace hypertrie::tests::raw::node_context::slicing {

	using namespace hypertrie::internal::raw;


	template<HypertrieInternalTrait tri, size_t depth, size_t fixed_depth>
	class TestSlice {

		/*
		 * definitions
		 */
		constexpr static const size_t result_depth = depth - fixed_depth;

		using RawSliceKey = typename tri::template RawSliceKey<fixed_depth>;
		using RawKey = typename tri::template RawKey<depth>;
		using RawResultKey = typename tri::template RawKey<result_depth>;
		using value_type = typename tri::value_type;

		/*
		 * fields
		 */

		std::set<std::pair<RawKey, value_type>> const *entries;

		RawSliceKey raw_slice_key;

		std::map<RawResultKey, value_type> slice_entries;

	public:
		TestSlice(const std::set<std::pair<RawKey, value_type>> &entries, const RawSliceKey &raw_slice_key)
			: entries(&entries), raw_slice_key(raw_slice_key) {
			for (const auto &[key, value] : *this->entries)
				if (raw_slice_key.satisfiedBy(key))
					slice_entries[raw_slice_key.slice(key)] = value;
		}


		bool empty() const noexcept {
			return slice_entries.empty();
		}

		size_t size() const noexcept {
			return slice_entries.size();
		}

		bool isManaged(NodeContext<depth, tri> &context) const {
			if (this->size() == 0) {
				return false;
			} else if (this->size() > 1)
				return true;
			else {
				// size == 1
				if constexpr (result_depth == 1 and tri::is_bool_valued and tri::is_lsb_unused)
					return false;
				else {
					const auto &[key, value] = *slice_entries.begin();

					return not context.storage.template getNode<result_depth>(
									TensorHash::getCompressedNodeHash(key, value))
							.empty();
				}
			}
		}

		const auto &sliceEntries() {
			return slice_entries;
		}
	};
}// namespace hypertrie::tests::raw::node_context::slicing

#endif//HYPERTRIE_TESTSLICE_HPP
