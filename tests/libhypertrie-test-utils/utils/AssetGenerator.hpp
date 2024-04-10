#ifndef HYPERTRIE_ASSETGENERATOR_HPP
#define HYPERTRIE_ASSETGENERATOR_HPP

#include <dice/hypertrie/Key.hpp>
#include <dice/hypertrie/internal/raw/RawKey.hpp>

#include <dice/hash/DiceHash.hpp>

#include <algorithm>
#include <concepts>
#include <random>
#include <unordered_map>
#include <unordered_set>

namespace dice::hypertrie::tests::utils {

	template<typename T>
	concept Integral = std::is_integral<T>::value;


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
		inline void reset() noexcept {
			rand = std::mt19937_64{42};
		}
	};

	template<Integral int_type>
	class IntegralUniform : public AssetGenerator {
		uniform_dist<int_type> dist;

	public:
		explicit IntegralUniform(int_type max = std::numeric_limits<int_type>::max()) noexcept : dist{0, max} {}
		IntegralUniform(int_type min, int_type max) noexcept : dist{min, max} {}

		void set_min_max(int_type min, int_type max) {
			dist = {min, max};
		}

		auto operator()() noexcept {
			return dist(rand);
		}
	};

	template<Integral int_type>
	class IntegralBinomial : public AssetGenerator {
		std::binomial_distribution<int_type> dist;

	public:
		explicit IntegralBinomial(int_type max = std::numeric_limits<int_type>::max()) noexcept : dist{0, max} {}
		IntegralBinomial(int_type min, int_type max) noexcept : dist{min, max} {}

		void set_min_max(int_type min, int_type max) {
			dist = {min, max};
		}

		auto operator()() noexcept {
			return dist(rand);
		}
	};

	template<size_t depth, HypertrieTrait htt_t>
	class RawGenerator : public AssetGenerator {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		using RawKey = ::dice::hypertrie::internal::raw::RawKey<depth, htt_t>;

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
			if constexpr (htt_t::key_part_tagging_bit == 0)
				return key_part_ << 1;
			else if constexpr (htt_t::key_part_tagging_bit > 0)
				return key_part_ & reinterpret_cast<key_part_type>(~(1UL << htt_t::key_part_tagging_bit));
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

	template<HypertrieTrait htt_t>
	class EntryGenerator : public RawGenerator<1, htt_t> {

		using super = RawGenerator<1, htt_t>;
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		using RawKey = typename super::RawKey;


	public:
		explicit EntryGenerator(key_part_type min = std::numeric_limits<key_part_type>::min(),
								key_part_type max = std::numeric_limits<key_part_type>::max(),
								value_type valueMin = std::numeric_limits<value_type>::min(),
								value_type valueMax = std::numeric_limits<value_type>::max())
			: RawGenerator<1, htt_t>(min, max, valueMin, valueMax) {}

		auto key(const size_t depth = 1) {
			Key<htt_t> key_(depth);
			std::generate(key_.begin(), key_.end(), [&]() { return this->key_part(); });
			return key_;
		}

		auto entry(const size_t depth = 1) {
			return std::pair{key(depth), this->value()};
		}

		auto keys(size_t size, const size_t depth = 1) {
			std::unordered_set<Key<htt_t>, dice::hash::DiceHashxxh3<Key<htt_t>>> key_set;
			while (key_set.size() < size) {
				key_set.insert(this->key(depth));
			}
			return key_set;
		}

		auto entries(const size_t size, const size_t depth = 1) {
			std::unordered_map<Key<htt_t>, value_type, dice::hash::DiceHashxxh3<Key<htt_t>>> entry_map;
			while (entry_map.size() < size) {
				auto next_entry = this->entry(depth);
				if (entry_map.count(next_entry.first))
					continue;
				entry_map.insert(next_entry);
			}
			return entry_map;
		}
	};
}// namespace dice::hypertrie::tests::utils

#endif//HYPERTRIE_ASSETGENERATOR_HPP
