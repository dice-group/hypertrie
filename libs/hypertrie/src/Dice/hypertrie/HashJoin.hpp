#ifndef HYPERTRIE_HASHJOIN_IMPL_HPP
#define HYPERTRIE_HASHJOIN_IMPL_HPP

#include <utility>

#include "Dice/hypertrie/Hypertrie.hpp"
//#include "Dice/hypertrie/internal/util/FrontSkipIterator.hpp"
#include "Dice/hypertrie/internal/util/PermutationSort.hpp"


namespace Dice::hypertrie {

	template<HypertrieTrait tr>
	class HashJoin {
	public:
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using poss_type = std::vector<internal::pos_type>;

	private:
		std::vector<const_Hypertrie<tr>> hypertries_;
		std::vector<poss_type> positions_;

	public:
		HashJoin() = default;

		HashJoin(std::vector<const_Hypertrie<tr>> hypertries, std::vector<poss_type> positions) noexcept
			: hypertries_(std::move(hypertries)), positions_(std::move(positions)) {}

		class iterator {

		public:
			using iterator_category = std::forward_iterator_tag;
			// TODO: swap positions of first and second
			using value_type = std::pair<std::vector<const_Hypertrie<tr>>, key_part_type>;
			using difference_type = ptrdiff_t;
			using pointer = value_type *;
			using reference = value_type &;

		private:
			poss_type pos_in_out_{};
			std::vector<internal::pos_type> result_depths_{};
			std::vector<HashDiagonal<tr>> ops_{};

			bool ended = false;

			value_type value{};

		public:
			iterator() = default;

			iterator(const HashJoin &join) noexcept {
				auto max_op_count = join.hypertries_.size();
				pos_in_out_.reserve(max_op_count);
				result_depths_.reserve(max_op_count);
				ops_.reserve(max_op_count);

				internal::pos_type out_pos = 0;
				for (size_t pos = 0; pos < join.hypertries_.size(); ++pos) {
					const auto &join_poss = join.positions_[pos];
					const auto &hypertrie = join.hypertries_[pos];
					if (size(join_poss) > 0) {
						ops_.emplace_back(HashDiagonal<tr>{hypertrie, internal::raw::RawKeyPositions<hypertrie_max_depth>(join_poss)});
						auto result_depth = result_depths_.emplace_back(hypertrie.depth() - size(join_poss));
						if (result_depth) {
							pos_in_out_.push_back(out_pos++);
							value.first.push_back(hypertrie);// only a place holder, is to be replaced in next()
						} else {
							pos_in_out_.push_back(std::numeric_limits<internal::pos_type>::max());
						}
					} else {
						assert(hypertrie.depth() != 0);  // TODO: currently not possible
						value.first.push_back(hypertrie);// this stays unchanged during the iteration
						++out_pos;
					}
				}
				optimizeOperandOrder();
				ops_.front().begin();
				next(true);
			}

			inline void next(bool init = false) noexcept {

				// check if the end was reached
				static bool found;
				// _current_key_part is increased if containsAndUpdateLower returns false
				HashDiagonal<tr> &smallest_operand = ops_.front();
				if (not init and not smallest_operand.ended())
					++smallest_operand;

				while (not smallest_operand.ended()) {

					value.second = smallest_operand.current_key_part();

					found = true;
					// iterate all but the first Diagonal
					for (size_t op_pos = 1; op_pos < ops_.size(); ++op_pos) {
						auto &operand = ops_[op_pos];
						if (not operand.find(value.second)) {
							found = false;
							break;
						}
					}
					if (found) {
						for (size_t op_pos = 0; op_pos < ops_.size(); ++op_pos) {
							if (const auto &result_depth = result_depths_[op_pos]; result_depth)
								value.first[pos_in_out_[op_pos]] = const_Hypertrie<tr>(ops_[op_pos].current_hypertrie());
						}
						return;
					}
					++smallest_operand;
				}
				ended = true;
			}

			iterator &operator++() noexcept {
				if (not ended)
					next();
				return *this;
			}

			iterator operator++(int) {
				auto copy = *this;
				++(*this);
				return copy;
			}

			inline operator bool() const noexcept {
				return not ended;
			}

			value_type operator*() const noexcept {
				return value;
			}

		private:
			void optimizeOperandOrder() {
				using namespace internal::util;
				const auto permutation = sort_permutation::get<HashDiagonal<tr>>(ops_);
				sort_permutation::apply(ops_, permutation);
				sort_permutation::apply(pos_in_out_, permutation);
				sort_permutation::apply(result_depths_, permutation);
			}
		};

		iterator begin() const noexcept { return iterator(*this); }

		[[nodiscard]] bool end() const noexcept { return false; }
	};
}// namespace Dice::hypertrie

#endif//HYPERTRIE_HASHJOIN_IMPL_HPP
