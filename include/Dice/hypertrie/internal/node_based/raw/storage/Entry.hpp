#ifndef HYPERTRIE_ENTRY_HPP
#define HYPERTRIE_ENTRY_HPP

#include "Dice/hypertrie/internal/node_based/raw/Hypertrie_internal_traits.hpp"


namespace hypertrie::internal::node_based {

	template<size_t depth, HypertrieInternalTrait tri_t>
	struct RawEntry_t{
		using tri = tri_t;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using RawKey = typename tri::template RawKey<depth>;

		class RawNonBoolEntry;

		using RawBoolEntry = RawKey;

		using RawEntry = std::conditional_t<(tri::is_bool_valued), RawBoolEntry, RawNonBoolEntry>;

		class RawNonBoolEntry  {
			RawKey key_;
			value_type value_;

		public:
			RawNonBoolEntry() : key_{}, value_{} {}
			RawNonBoolEntry(const RawKey &key, [[maybe_unused]] value_type value = value_type(1)) : key_{key}, value_(value) {}

			key_part_type &operator[](size_t pos) {
				return key_[pos];
			}

			const key_part_type &operator[](size_t pos) const {
				return key_[pos];
			}
			friend struct RawEntry_t;

		};


		static RawEntry make_Entry(RawKey key, const value_type value) {
			if constexpr (tri::is_bool_valued)
				return {key};
			else
				return {key, value};
		}

		static RawKey &key(RawEntry &entry) {
			if constexpr (tri::is_bool_valued)
				return entry;
			else
				return entry.key_;
		}

		static const RawKey &key(const RawEntry &entry) {
			if constexpr (tri::is_bool_valued)
				return entry;
			else
				return entry.key_;
		}

		static value_type &value(RawNonBoolEntry &entry) {
			return entry.value_;
		}

		static const value_type &value(const RawNonBoolEntry &entry) {
			return entry.value_;
		}

		static value_type value([[maybe_unused]] const RawKey &entry) {
			return true;
		}

	};




}

#endif//HYPERTRIE_ENTRY_HPP
