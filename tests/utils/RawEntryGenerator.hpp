#ifndef HYPERTRIE_RAWENTRYGENERATOR_HPP
#define HYPERTRIE_RAWENTRYGENERATOR_HPP

#include <tsl/sparse_set.h>

#include "AssetGenerator.hpp"

#include <Dice/hypertrie/internal/raw/node/SingleEntry.hpp>

namespace hypertrie::tests::utils {

	template<size_t depth, internal::raw::HypertrieCoreTrait tri_t>
	class RawEntryGenerator : public AssetGenerator {
	public:
		using tri = tri_t;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using RawKey_t = ::hypertrie::internal::raw::RawKey<depth, tri>;
		using SingleEntry_t = ::hypertrie::internal::raw::SingleEntry<depth, tri>;

	protected:
		value_type value_min_;
		value_type value_max_;
		key_part_type key_part_min_;
		key_part_type key_part_max_;


		using dist_value_type = std::conditional_t<(std::is_same_v<value_type, bool>), unsigned char, value_type>;

		uniform_dist<key_part_type> key_part_dist;
		uniform_dist<dist_value_type> value_dist;

	public:
		explicit RawEntryGenerator(key_part_type key_part_min = std::numeric_limits<key_part_type>::min(),
								   key_part_type key_part_max = std::numeric_limits<key_part_type>::max(),
								   value_type valueMin = std::numeric_limits<value_type>::min(),
								   value_type valueMax = std::numeric_limits<value_type>::max())
			: value_min_(valueMin), value_max_(valueMax),
			  key_part_min_(key_part_min), key_part_max_(key_part_max),
			  key_part_dist{key_part_min, key_part_max}, value_dist{value_min_, value_max_} {}

		value_type getValueMin() const {
			return value_min_;
		}
		void setValueMinMax(value_type value_min, value_type value_max) {
			this->value_min_ = value_min;
			this->value_max_ = value_max;
			value_dist = uniform_dist<dist_value_type>{value_min, value_max};
		}

		void setKeyPartMinMax(key_part_type key_part_min, key_part_type key_part_max) {
			this->key_part_min_ = key_part_min;
			this->key_part_max_ = key_part_max;
			key_part_dist = uniform_dist<key_part_type>{key_part_min, key_part_max};
		}

		key_part_type key_part() {
			auto key_part_ = key_part_dist(rand);
			// if the least significant bit is the tagging bit, we shift the value by 1.
			if constexpr (tri::key_part_tagging_bit == 0)
				return key_part_ << 1;
			else if constexpr (tri::key_part_tagging_bit > 0)
				return key_part_ & reinterpret_cast<key_part_type>(~(1UL << tri::key_part_tagging_bit));
			else
				return key_part_dist(rand);
		}

		RawKey_t key() {
			RawKey_t key_{};
			std::generate(key_.begin(), key_.end(), [&]() { return key_part(); });
			return key_;
		}

		value_type value() {
			if constexpr (std::is_same_v<key_part_type, bool>) return true;
			value_type value_;
			do {
				value_ = value_dist(rand);
			} while (value_ == value_type{});
			return value_;
		}

		SingleEntry_t entry() {
			return std::pair{key(), value()};
		}

		std::vector<SingleEntry_t> entries(size_t size) {
			std::vector<SingleEntry_t> entries_(size);
			tsl::sparse_set<RawKey_t, Dice::hash::DiceHashMartinus<RawKey_t>> seen;
			size_t pos = 0;
			size_t stall = 0;
			while (pos < size) {
				static RawKey_t key;
				key = this->key();
				if (not seen.contains(key)) {
					entries_[pos++] = SingleEntry_t{key, value()};
					seen.insert(key);
					stall = 0;
				}
				if (stall++ == 4)
					break;
			}
			return entries_;
		}
	};

}// namespace hypertrie::tests::utils
#endif//HYPERTRIE_RAWENTRYGENERATOR_HPP
