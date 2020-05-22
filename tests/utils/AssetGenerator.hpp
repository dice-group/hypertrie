#ifndef HYPERTRIE_ASSETGENERATOR_HPP
#define HYPERTRIE_ASSETGENERATOR_HPP

#include <random>
#include <algorithm>

#include <Dice/hypertrie/internal/util/RawKey.hpp>
#include <Dice/hypertrie/internal/util/Key.hpp>

namespace hypertrie::tests::utils {

	class AssetGenerator {
	protected:
		std::mt19937_64 rand = std::mt19937_64{42};

	public:
		inline void reset() {
			rand = std::mt19937_64{42};
		}
	};

	class LongGenerator : public AssetGenerator {
		std::uniform_int_distribution<long> dist;

	public:
		explicit LongGenerator(long max ) : dist{0, max} {}
		LongGenerator(long min, long max ) : dist{min, max} {}

		auto operator()(){
			return dist(rand);
		}
	};

	template<size_t depth, typename key_part_type, typename value_type,
			key_part_type min = std::numeric_limits<key_part_type>::min(),
			key_part_type max = std::numeric_limits<key_part_type>::max()>
	class RawGenerator : public AssetGenerator {

		using RawKey = hypertrie::internal::RawKey<depth, key_part_type>;
		value_type value_min = std::numeric_limits<value_type>::min();
		value_type value_max = std::numeric_limits<value_type>::max();
		std::uniform_int_distribution<key_part_type> key_part_dist{min, max};
		std::uniform_int_distribution<value_type> value_dist{value_min, value_max};
	public:
		auto key() {
			RawKey key_{};
			std::generate(key_.begin(), key_.end(), [&]() { return key_part_dist(rand); });
			return key_;
		}

		auto value() {
			if constexpr(std::is_same_v<key_part_type, bool>) return true;
			value_type value_;
			do {
				value_ = value_dist(rand);
			} while (value_ == value_type{});
			return value_;
		}

		auto entry() {
			return std::pair{key(), value()};
		}

		auto entries(size_t size) {
			std::set<std::pair<RawKey, value_type>> entry_set;
			while (entry_set.size() < size) {
				entry_set.insert(this->entry());
			}
			return entry_set;
		}
	};
}

#endif //HYPERTRIE_ASSETGENERATOR_HPP
