#ifndef HYPERTRIE_RAWENTRYGENERATOR_HPP
#define HYPERTRIE_RAWENTRYGENERATOR_HPP

#include <dice/sparse-map/sparse_set.hpp>

#include "AssetGenerator.hpp"

#include <dice/hypertrie/internal/raw/node/SingleEntry.hpp>

namespace dice::hypertrie::tests::utils {

	template<size_t depth, HypertrieTrait htt_t>
	class RawEntryGenerator : public AssetGenerator {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		using RawKey_t = ::dice::hypertrie::internal::raw::RawKey<depth, htt_t>;
		using SingleEntry_t = ::dice::hypertrie::internal::raw::SingleEntry<depth, htt_t>;

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

		void setValueMinMax(value_type value_min, value_type value_max) {
			this->value_min_ = value_min;
			this->value_max_ = value_max;
			value_dist = uniform_dist<dist_value_type>{value_min, value_max};
		}
		void wind(size_t times) {
			this->rand.discard(times);
		}

		void setKeyPartMinMax(key_part_type key_part_min, key_part_type key_part_max) {
			this->key_part_min_ = key_part_min;
			this->key_part_max_ = key_part_max;
			key_part_dist = uniform_dist<key_part_type>{key_part_min, key_part_max};
		}

		const value_type &getValueMin() const {
			return value_min_;
		}
		const value_type &getValueMax() const {
			return value_max_;
		}
		const key_part_type &getKeyPartMin() const {
			return key_part_min_;
		}
		const key_part_type &getKeyPartMax() const {
			return key_part_max_;
		}

		key_part_type key_part() {
			auto key_part_ = key_part_dist(rand);
			// if the least significant bit is the tagging bit, we shift the value by 1.
			if constexpr (htt_t::key_part_tagging_bit == 0)
				return key_part_ << 1;
			else if constexpr (htt_t::key_part_tagging_bit > 0)
				return key_part_ & reinterpret_cast<key_part_type>(~(1UL << htt_t::key_part_tagging_bit));
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
			return {key(), value()};
		}

		std::vector<SingleEntry_t> entries(size_t size) {
			std::vector<SingleEntry_t> entries_;
			entries_.reserve(size);
			dice::sparse_map::sparse_set<RawKey_t, dice::hash::DiceHashMartinus<RawKey_t>> seen;
			size_t stall = 0;
			while (entries_.size() < size) {
				static RawKey_t key;
				key = this->key();
				if (not seen.contains(key)) {
					entries_.emplace_back(key, value());
					seen.insert(key);
					stall = 0;
				}
				if (stall++ == 10)
					break;
			}
			return entries_;
		}
	};

}// namespace dice::hypertrie::tests::utils
#endif//HYPERTRIE_RAWENTRYGENERATOR_HPP
