#ifndef HYPERTRIE_DIAGONALTESTDATAGENERATOR_HPP
#define HYPERTRIE_DIAGONALTESTDATAGENERATOR_HPP

#include "AssetGenerator.hpp"
#include <set>
namespace hypertrie::tests::utils {

	template<size_t depth, typename key_part_type, typename value_type>
	struct DiagonalTestData {
		using Key = hypertrie::Key<key_part_type>;
		using DiagonalPositions = std::vector<size_t>;
		using Entry = std::pair<Key, value_type>;


		DiagonalPositions diagonal_positions;
		std::map<Key, value_type> tensor_entries;
		std::map<key_part_type,
				 std::map<Key, value_type>>
				diagonal_entries;

		bool validate() {
			if (depth > diagonal_positions.size())
				return false;
			else if (std::set<Entry>{diagonal_positions}.size() != diagonal_positions.size())
				return false;
			else if ([&]() { for(auto pos : diagonal_positions) if ( pos >= depth ) return true; return false; }())
				return false;
			else
				return true;
		}
	};

	template<size_t depth, typename key_part_type, typename value_type, size_t unused_lsb_bits = 0>
	class DiagonalTestDataGenerator : public RawGenerator<depth, key_part_type, value_type, unused_lsb_bits> {
		using super = RawGenerator<depth, key_part_type, value_type, unused_lsb_bits>;

		using RawKey = hypertrie::internal::RawKey<depth, key_part_type>;
		using Key = hypertrie::Key<key_part_type>;

		using DiagonalTestData_t = DiagonalTestData<depth, key_part_type, value_type>;

		using DiagonalPositions = typename DiagonalTestData_t::DiagonalPositions;
		using Entry = std::pair<Key, value_type>;

	public:
		DiagonalTestDataGenerator(key_part_type min = std::numeric_limits<key_part_type>::min(),
								  key_part_type max = std::numeric_limits<key_part_type>::max(),
								  value_type valueMin = std::numeric_limits<value_type>::min(),
								  value_type valueMax = std::numeric_limits<value_type>::max())
			: RawGenerator<depth, key_part_type, value_type>(min, max, valueMin, valueMax) {}

		DiagonalTestData_t diag_data(size_t size, size_t diagonal_size, const DiagonalPositions &diagonal_positions) {
			assert(diagonal_size < size);

			setValueMinMax(1, value_type(5));
			setKeyPartMinMax(key_part_type(0), key_part_type((size + diagonal_size) / depth + 1));
			DiagonalTestData_t test_data{};
			test_data.diagonal_positions = diagonal_positions;

			auto diag_key_parts = [&]() {
				std::set<key_part_type> tmp;
				while (tmp.size() < diagonal_size)
					tmp.insert(this->key_part());
				return tmp;
			}();

			for (auto diag_key_part : iter::range(diag_key_parts))
				test_data.tensor_entries[diagonal_entry(diag_key_part, diagonal_positions)] = this->value();

			while (test_data.tensor_entries.size() < size) {
				auto key_candidate = this->key();
				if (test_data.tensor_entries.count(key_candidate))
					continue;
				else if (auto diag_key_part_opt = is_diagonal(key_candidate, diagonal_positions);
						 diag_key_part_opt.has_value() and not diag_key_parts.count(diag_key_part_opt.value()))
					continue;
				test_data.tensor_entries[key_candidate] = this->value();
			}

			for (const auto &[key, value] : test_data.tensor_entries) {
				auto diag_key_part_opt = is_diagonal(key, diagonal_positions);
				if (diag_key_part_opt.has_value()) {
					test_data.diagonal_entries[diag_key_part_opt.value()][key] = value;
				}
			}
			assert(test_data.validate());
			return test_data;
		}

		auto diagonal_entry(key_part_type diagonal_key_part, const DiagonalPositions &diagonal_positions) {
			RawKey key_{};
			auto diag_pos_iter = diagonal_positions.cbegin();
			for (auto [pos, key_part] : iter::enumerate(key_)) {
				if (diag_pos_iter != diag_pos_iter.cend() and *diag_pos_iter == pos) {
					key_part = diagonal_key_part;
					diag_pos_iter++;
				} else {
					do {
						key_part = this->key_part();
					} while (key_part == diagonal_key_part);
				}
			}
			return key_;
		}

		std::optional<key_part_type> is_diagonal(const RawKey &key_, const DiagonalPositions &diagonal_positions) {
			bool first = true;
			std::optional<key_part_type> result;
			for (const auto &diag_pos : diagonal_positions) {
				if (first) {
					result = key_[diag_pos];
					first = false;
				} else {
					if (key_[diag_pos] != result.value())
						return {};
				}
			}
			return result;
		}
	};

}// namespace hypertrie::tests::utils
#endif//HYPERTRIE_DIAGONALTESTDATAGENERATOR_HPP
