#ifndef HYPERTRIE_ASSETGENERATOR_HPP
#define HYPERTRIE_ASSETGENERATOR_HPP

#include <algorithm>
#include <random>
#include <set>

#include <Dice/hypertrie/Key.hpp>
#include <Dice/hypertrie/internal/raw/RawKey.hpp>

namespace hypertrie::tests::utils {

	/**
	 * float / Integral independent uniform dist
	 */
	template<class T>
	using uniform_dist = std::conditional_t<(std::is_floating_point<T>::value),
											std::uniform_real_distribution<T>,
											std::uniform_int_distribution<T>>;

	class AssetGenerator {
	protected:
		std::mt19937_64 rand{42};


	public:
		inline void reset() {
			rand = std::mt19937_64{42};
		}
	};

	class LongGenerator : public AssetGenerator {
		uniform_dist<long> dist;

	public:
		explicit LongGenerator(long max) : dist{0, max} {}
		LongGenerator(long min, long max) : dist{min, max} {}

		auto operator()() {
			return dist(rand);
		}
	};

	template<size_t depth, internal::raw::HypertrieCoreTrait tri_t>
	class RawGenerator : public AssetGenerator {
	public:
		using tri = tri_t;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using RawKey = ::hypertrie::internal::raw::RawKey<depth, tri>;

	protected:
		value_type value_min_;
		value_type value_max_;
		key_part_type key_part_min_;
		key_part_type key_part_max_;


		using dist_value_type = std::conditional_t<(std::is_same_v<value_type, bool>), unsigned char, value_type>;

		uniform_dist<key_part_type> key_part_dist;
		uniform_dist<dist_value_type> value_dist;

	public:
		explicit RawGenerator(key_part_type key_part_min = std::numeric_limits<key_part_type>::min(),
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

		RawKey key() {
			RawKey key_{};
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

		std::pair<RawKey, value_type> entry() {
			return std::pair{key(), value()};
		}

		std::set<RawKey> keys(size_t size) {
			std::set<RawKey> key_set;
			while (key_set.size() < size) {
				key_set.insert(this->key());
			}
			return key_set;
		}

		/**
		 * Generates a set of Entries (key-value-pairs) of given size. It does contain only entries with pair-wise different keys.
		 * @param size
		 * @return
		 */
		std::map<RawKey, value_type> entries(size_t size) {
			std::map<RawKey, value_type> entry_map;
			while (entry_map.size() < size) {
				auto next_entry = this->entry();
				if (entry_map.count(next_entry.first))
					continue;
				entry_map.insert(next_entry);
			}
			return entry_map;
		}
	};

	template<internal::raw::HypertrieCoreTrait tri_t>
	class EntryGenerator : public RawGenerator<1, tri_t> {
		using tri = tri_t;
		using super = RawGenerator<1, tri>;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using RawKey = typename super::RawKey;


		using Key = hypertrie::Key<tri>;

	public:
		explicit EntryGenerator(key_part_type min = std::numeric_limits<key_part_type>::min(),
								key_part_type max = std::numeric_limits<key_part_type>::max(),
								value_type valueMin = std::numeric_limits<value_type>::min(),
								value_type valueMax = std::numeric_limits<value_type>::max())
			: RawGenerator<1, tri>(min, max, valueMin, valueMax) {}

		auto key(const size_t depth = 1) {
			Key key_(depth);
			std::generate(key_.begin(), key_.end(), [&]() { return this->key_part(); });
			return key_;
		}

		auto entry(const size_t depth = 1) {
			return std::pair{key(depth), this->value()};
		}

		auto keys(size_t size, const size_t depth = 1) {
			std::set<Key> key_set;
			while (key_set.size() < size) {
				key_set.insert(this->key(depth));
			}
			return key_set;
		}

		auto entries(const size_t &size, const size_t &depth = 1) {
			std::map<Key, value_type> entry_map;
			while (entry_map.size() < size) {
				auto next_entry = this->entry(depth);
				if (entry_map.count(next_entry.first))
					continue;
				entry_map.insert(next_entry);
			}
			return entry_map;
		}
	};
}// namespace hypertrie::tests::utils

#endif//HYPERTRIE_ASSETGENERATOR_HPP
