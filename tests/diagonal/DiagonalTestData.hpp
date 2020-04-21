#ifndef HYPERTRIE_DIAGONALTESTDATA_HPP
#define HYPERTRIE_DIAGONALTESTDATA_HPP

#include <vector>
#include <tsl/sparse_set.h>
#include <tsl/sparse_map.h>
#include <boost/container_hash/hash.hpp>


namespace hypertrie::tests {
	template<typename T, typename =std::enable_if_t<(not std::is_same_v<std::decay_t<T>, bool>)>>
	struct VectorHash {
		std::size_t operator()(const std::vector<T> &k) const {
			return boost::hash_range(k.begin(), k.end());
		}
	};

	template<typename key_part_type = size_t>
	struct DiagonalTestData {
		using pos_type = uint8_t;
		using poss_type = std::vector<pos_type>;
		using KeyHash = VectorHash<key_part_type>;
	private:
		static poss_type getRemainingPoss(const poss_type &positions, pos_type const depth) {
			std::set<uint8_t> all_positions;
			poss_type remaining_pos;
			for (auto i : iter::range(depth)) {
				all_positions.insert(i);
			}
			std::set_difference(all_positions.begin(), all_positions.end(), positions.begin(), positions.end(),
			                    std::inserter(remaining_pos, remaining_pos.begin()));
			return remaining_pos;
		}

	public:
		using Key = std::vector<key_part_type>;

		mutable pos_type depth;

		mutable poss_type join_positions;

		mutable poss_type remaining_positions;

		mutable std::vector<Key> entries;

		mutable tsl::sparse_map<key_part_type, tsl::sparse_set<Key, KeyHash>> expected_entries;


		DiagonalTestData(poss_type &join_positions, const std::size_t count,
		                 const std::size_t depth, const key_part_type max)
				: depth(depth), join_positions(join_positions),
				  remaining_positions(getRemainingPoss(join_positions, depth)) {
			entries = utils::generateNTuples(count, depth, max, 8);

			if (join_positions.empty());
			else
				for (const auto &entry : entries) {
					bool in_diagonal = true;
					auto key_part = entry[join_positions[0]];

					for (auto diagonal_pos : hypertrie::internal::util::skip<1>(join_positions)) {
						if (key_part != entry[diagonal_pos]) {
							in_diagonal = false;
							break;
						}
					}
					if (not in_diagonal)
						continue;

					Key diagonal_key;
					for (auto pos : remaining_positions) {
						diagonal_key.push_back(entry[pos]);
					}
					if (not expected_entries.count(key_part)) {
						expected_entries.insert({key_part, tsl::sparse_set<Key, KeyHash>{}});
					}
					expected_entries[key_part].insert(diagonal_key);
				}
		}
	};
}
#endif //HYPERTRIE_DIAGONALTESTDATA_HPP
