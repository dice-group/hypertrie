#ifndef HYPERTRIE_ASSETGENERATOR_HPP
#define HYPERTRIE_ASSETGENERATOR_HPP

#include <random>
#include <algorithm>
#include <set>

#include <Dice/hypertrie/internal/util/RawKey.hpp>
#include <Dice/hypertrie/internal/util/Key.hpp>

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
		std::mt19937_64 rand = std::mt19937_64{42};



	public:
		inline void reset() {
			rand = std::mt19937_64{42};
		}
	};

	class LongGenerator : public AssetGenerator {
		uniform_dist<long> dist;

	public:
		explicit LongGenerator(long max ) : dist{0, max} {}
		LongGenerator(long min, long max ) : dist{min, max} {}

		auto operator()(){
			return dist(rand);
		}
	};

	template<size_t depth, typename key_part_type, typename value_type,
			 size_t unused_lsb_bits = 0>
	class RawGenerator : public AssetGenerator {
	protected:
		using RawKey = hypertrie::internal::RawKey<depth, key_part_type>;
		value_type value_min;
		value_type value_max;
		key_part_type key_part_min;
		key_part_type key_part_max;


		using dist_value_type = std::conditional_t<(std::is_same_v<value_type, bool>), unsigned char, value_type>;

		uniform_dist<key_part_type> key_part_dist;
		uniform_dist<dist_value_type> value_dist;

	public:
		RawGenerator(key_part_type key_part_min = std::numeric_limits<key_part_type>::min(),
					 key_part_type key_part_max = std::numeric_limits<key_part_type>::max(),
					 value_type valueMin = std::numeric_limits<value_type>::min(),
					 value_type valueMax = std::numeric_limits<value_type>::max())
			: value_min(valueMin), value_max(valueMax),
			  key_part_min(key_part_min), key_part_max(key_part_max),
			  key_part_dist{key_part_min, key_part_max}, value_dist{value_min, value_max} {}

		value_type getValueMin() const {
			return value_min;
		}
		void setValueMinMax(value_type value_min, value_type value_max) {
			this->value_min = value_min;
			this->value_max = value_max;
			value_dist = uniform_dist<dist_value_type>{value_min, value_max};
		}

		void setKeyPartMinMax(key_part_type key_part_min, key_part_type key_part_max) {
			this->key_part_min = key_part_min;
			this->key_part_max = key_part_max;
			key_part_dist = uniform_dist<key_part_type>{key_part_min, key_part_max};
		}

		auto key_part() {
			return key_part_dist(rand) << unused_lsb_bits;
		}

		auto key() {
			RawKey key_{};
			std::generate(key_.begin(), key_.end(), [&]() { return key_part(); });
			return key_;
		}

		auto value() {
			if constexpr (std::is_same_v<key_part_type, bool>) return true;
			value_type value_;
			do {
				value_ = value_dist(rand);
			} while (value_ == value_type{});
			return value_;
		}

		auto entry() {
			return std::pair{key(), value()};
		}

		auto keys(size_t size) {
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
		auto entries(size_t size) {
			std::set<RawKey> unique_keys;
			std::set<std::pair<RawKey, value_type>> entry_set;
			while (entry_set.size() < size) {
				auto next_entry = this->entry();
				if (unique_keys.count(next_entry.first))
					continue;
				unique_keys.insert(next_entry.first);
				entry_set.insert(next_entry);
			}
			return entry_set;
		}
	};

	template<typename key_part_type, typename value_type, size_t unused_lsb_bits = 0>
	class EntryGenerator : public RawGenerator<1, key_part_type, value_type, unused_lsb_bits> {
		using super = RawGenerator<1, key_part_type, value_type, unused_lsb_bits>;

		using Key = hypertrie::Key<key_part_type>;

	public:
		EntryGenerator(key_part_type min = std::numeric_limits<key_part_type>::min(),
					   key_part_type max = std::numeric_limits<key_part_type>::max(),
					   value_type valueMin = std::numeric_limits<value_type>::min(),
					   value_type valueMax = std::numeric_limits<value_type>::max())
			: RawGenerator<1, key_part_type, value_type>(min, max, valueMin, valueMax) {}

		auto key(const size_t depth = 1) {
			Key key_(depth);
			std::generate(key_.begin(), key_.end(), [&]() { return this->key_part(); });
			return key_;
		}

		auto entry(const size_t depth = 1) {
			return std::pair{key(depth), this->value()};
		}

		auto keys(size_t size) {
			auto raw_key_set = super::keys(size);
			std::set<Key> key_set;
			for (auto raw_key : raw_key_set) {
				key_set.insert({raw_key.begin(), raw_key.end()});
			}
			return key_set;
		}

		auto entries(const size_t &size, const size_t &depth = 1) {
			std::set<Key> unique_keys;
			std::set<std::pair<Key, value_type>> entry_set;
			while (entry_set.size() < size) {
				auto next_entry = this->entry(depth);
				if (unique_keys.count(next_entry.first))
					continue;
				unique_keys.insert(next_entry.first);
				entry_set.insert(next_entry);
			}
			return entry_set;
		}
	};
}

#endif //HYPERTRIE_ASSETGENERATOR_HPP
