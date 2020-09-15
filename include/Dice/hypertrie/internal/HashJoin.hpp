#ifndef HYPERTRIE_HASHJOIN_IMPL_HPP
#define HYPERTRIE_HASHJOIN_IMPL_HPP

#include <utility>

#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/util/FrontSkipIterator.hpp"
#include "Dice/hypertrie/internal/util/PermutationSort.hpp"
#include "Dice/hypertrie/internal/Hypertrie.hpp"


namespace hypertrie {

	template<HypertrieTrait tr =default_bool_Hypertrie_t>
	class HashJoin {
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;

	public:
		using poss_type = std::vector<pos_type>;

	private:
		std::vector<const_Hypertrie<tr>> hypertries;
		std::vector<poss_type> positions;

	public:

		HashJoin() = default;

		HashJoin(HashJoin &) = default;

		HashJoin(const HashJoin &) = default;


		HashJoin(std::vector<const_Hypertrie<tr>> hypertries, std::vector<poss_type> positions)
				: hypertries(std::move(hypertries)), positions(std::move(positions)) {}

		class iterator {

		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::pair<std::vector<const_Hypertrie<tr>>, key_part_type>;
			using difference_type = ptrdiff_t;
			using pointer = value_type *;
			using reference = value_type &;
		private:
			poss_type pos_in_out{};
			std::vector<pos_type> result_depths{};
			std::vector<HashDiagonal<tr>> ops{};

			bool ended = false;

			value_type value{};
		public:
			iterator() = default;

			iterator(const HashJoin &join) {
				auto max_op_count = join.hypertries.size();
				pos_in_out.reserve(max_op_count);
				result_depths.reserve(max_op_count);
				ops.reserve(max_op_count);

				pos_type out_pos = 0;
				for (const auto &[pos, join_poss, hypertrie] : iter::zip(iter::range(join.hypertries.size()), join.positions,
				                                                   join.hypertries)) {
					if (size(join_poss) > 0) {
						ops.emplace_back(HashDiagonal<tr>{hypertrie, join_poss});
						auto result_depth = result_depths.emplace_back(hypertrie.depth() - size(join_poss));
						if (result_depth) {
							pos_in_out.push_back(out_pos++);
							value.first.push_back(hypertrie); // only a place holder, is to be replaced in next()
						} else {
							pos_in_out.push_back(std::numeric_limits<pos_type>::max());
						}
					} else {
						assert(hypertrie.depth() != 0); // TODO: currently not possible
						value.first.push_back(hypertrie); // this stays unchanged during the iteration
						++out_pos;
					}
				}
				optimizeOperandOrder();
				ops.front().begin();
				next();
			}

			inline void next() {
				// check if the end was reached
				static bool found;
				// _current_key_part is increased if containsAndUpdateLower returns false
				HashDiagonal<tr> &smallest_operand = ops.front();

				while (not smallest_operand.ended()) {

					value.second = smallest_operand.currentKeyPart();

					found = true;
					// iterate all but the first Diagonal
					for (auto &operand: internal::util::skip<1>(ops)) {
						if (not operand.find(value.second)) {
							found = false;
							break;
						}
					}
					if (found) {
						for (const auto op_pos : iter::range(ops.size())) {
							if (const auto &result_depth = result_depths[op_pos]; result_depth)
								value.first[pos_in_out[op_pos]] = const_Hypertrie<tr>(ops[op_pos].currentHypertrie());
						}
						++smallest_operand;
						return;
					}
					++smallest_operand;
				}
				ended = true;
			}

			iterator &operator++() {
				if (not ended)
					next();
				return *this;
			}

			inline operator bool() const {
				return not ended;
			}

			value_type operator*() const {
				return value;
			}

		private:
			void optimizeOperandOrder() {
				using namespace internal::util;
				const auto permutation = sort_permutation::get<HashDiagonal<tr>>(ops);
				sort_permutation::apply(ops, permutation);
				sort_permutation::apply(pos_in_out, permutation);
				sort_permutation::apply(result_depths, permutation);
			}

		};

		iterator begin() const { return iterator(*this); }

		[[nodiscard]] bool end() const { return false; }


	};
}

#endif //HYPERTRIE_HASHJOIN_IMPL_HPP



