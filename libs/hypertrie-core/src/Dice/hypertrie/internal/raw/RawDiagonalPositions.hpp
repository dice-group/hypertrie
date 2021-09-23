#ifndef HYPERTRIE_RAWDIAGONALPOSITIONS_HPP
#define HYPERTRIE_RAWDIAGONALPOSITIONS_HPP

namespace hypertrie::internal::raw {

	template<size_t depth>
	class RawKeyPositions : public std::bitset<depth> {

		// TODO:
		//	template<size_t depth>
		//	static inline DiagonalPositions<depth> rawDiagonalPositions(const typename tr::KeyPositions &diagonal_positions) {
		//		DiagonalPositions<depth> raw_diag_poss;
		//		for (const auto &pos : diagonal_positions)
		//			raw_diag_poss[pos] = true;
		//		return raw_diag_poss;
		//	}
		// TODO:
		//	template<size_t depth>
		//	static inline typename tr::KeyPositions diagonalPositions(const DiagonalPositions<depth> &raw_diagonal_positions) {
		//		typename tr::KeyPositions diagonal_positions;
		//		for(auto [pos, is_diag_pos] : iter::enumerate(raw_diagonal_positions))
		//			if (is_diag_pos)
		//				diagonal_positions.push_back(pos);
		//		return diagonal_positions;
		//	}
	};

}// namespace hypertrie::internal::raw

#endif//HYPERTRIE_RAWDIAGONALPOSITIONS_HPP
