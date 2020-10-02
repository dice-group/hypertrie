#ifndef HYPERTRIE_DIAGONALTESTDATAGENERATOR_HPP
#define HYPERTRIE_DIAGONALTESTDATAGENERATOR_HPP

#include <set>
#include <map>
#include <cassert>

#include <itertools.hpp>

#include "AssetGenerator.hpp"

namespace hypertrie::tests::utils {

	template<size_t diag_depth, size_t depth, typename key_part_type, typename value_type>
	struct DiagonalTestData {
		static_assert(diag_depth != 0 and diag_depth <= depth);
		using Key = hypertrie::Key<key_part_type>;
		template<size_t depth_>
		using RawKey = hypertrie::internal::RawKey<depth_, key_part_type>;
		using DiagonalPositions = std::vector<uint8_t>;
		using Entry = std::pair<Key, value_type>;


		DiagonalPositions diagonal_positions;
		std::map<RawKey<depth>, value_type> tensor_entries;
		std::map<key_part_type,
				 std::map<RawKey<depth - diag_depth>, value_type>>
				diagonal_entries;

		bool validate() {
			if (diag_depth != diagonal_positions.size())
				return false;
			else if (std::set<uint8_t>{diagonal_positions.begin(), diagonal_positions.end()}.size() != diagonal_positions.size())
				return false;
			else if ([&]() { for(auto pos : diagonal_positions) if ( pos >= depth ) return true; return false; }())
				return false;
			else
				return true;
		}
	};

	template<size_t diag_depth, size_t depth, typename key_part_type, typename value_type, size_t unused_lsb_bits = 0>
	class DiagonalTestDataGenerator : public RawGenerator<depth, key_part_type, value_type, unused_lsb_bits> {
		using super = RawGenerator<depth, key_part_type, value_type, unused_lsb_bits>;

		template<size_t depth_>
		using RawKey = hypertrie::internal::RawKey<depth_, key_part_type>;
		using Key = hypertrie::Key<key_part_type>;

		using DiagonalTestData_t = DiagonalTestData<diag_depth, depth, key_part_type, value_type>;

		using DiagonalPositions = typename DiagonalTestData_t::DiagonalPositions;
		using Entry = std::pair<Key, value_type>;

	public:
		DiagonalTestDataGenerator(key_part_type min = std::numeric_limits<key_part_type>::min(),
								  key_part_type max = std::numeric_limits<key_part_type>::max(),
								  value_type valueMin = std::numeric_limits<value_type>::min(),
								  value_type valueMax = std::numeric_limits<value_type>::max())
			: RawGenerator<depth, key_part_type, value_type, unused_lsb_bits>(min, max, valueMin, valueMax) {}

		DiagonalTestData_t diag_data(size_t size, size_t diagonal_size, const DiagonalPositions &diagonal_positions) {
			assert(diagonal_size <= size);

			this->setValueMinMax(1, value_type(5));
			const size_t partMax = (size != 1) ? std::max(diagonal_size, (size / depth + 1)) : depth - 1;
			this->setKeyPartMinMax(key_part_type(0), partMax);
			DiagonalTestData_t test_data{};
			test_data.diagonal_positions = diagonal_positions;

			auto diag_key_parts = [&]() {
				std::set<key_part_type> tmp;
				while (tmp.size() < diagonal_size)
					tmp.insert(this->key_part());
				return tmp;
			}();

			for (auto diag_key_part : diag_key_parts){

				RawKey<depth> diag_key = diagonal_entry(diag_key_part, diagonal_positions);
				test_data.tensor_entries[diag_key] = this->value();
			}

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
					RawKey<depth - diag_depth> sub_key;
					{
						auto sub_key_pos = 0;
						auto diag_pos = 0;
						for (auto [pos, key_part] : iter::enumerate(key)) {
							if (diagonal_positions[diag_pos] == pos) {
								diag_pos++;
								continue;
							} else {
								sub_key[sub_key_pos++] = key_part;
							}
						}
					}
					auto &x = test_data.diagonal_entries[diag_key_part_opt.value()];
					x[sub_key] = value;
				}
			}
			assert(test_data.validate());
			return test_data;
		}

		auto diagonal_entry(key_part_type diagonal_key_part, const DiagonalPositions &diagonal_positions) {
			RawKey<depth> key_{};
			auto diag_pos_iter = diagonal_positions.cbegin();
			for (auto [pos, key_part] : iter::enumerate(key_)) {
				if (diag_pos_iter != diagonal_positions.cend() and *diag_pos_iter == pos) {
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

		std::optional<key_part_type> is_diagonal(const RawKey<depth> &key_, const DiagonalPositions &diagonal_positions) {
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
