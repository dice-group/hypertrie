#ifndef HYPERTRIE_HASHJOIN_IMPL_HPP
#define HYPERTRIE_HASHJOIN_IMPL_HPP

#include "dice/hypertrie/Hypertrie.hpp"
#include "dice/hypertrie/internal/util/PermutationSort.hpp"

#include <utility>

namespace dice::hypertrie {

	template<HypertrieTrait htt_t, ByteAllocator allocator_type, bool Optional = false>
	class HashJoin {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		using poss_type = std::vector<internal::pos_type>;

	private:
		std::vector<const_Hypertrie<htt_t, allocator_type>> hypertries_;
		std::vector<poss_type> positions_;

	public:
		HashJoin() = default;

		HashJoin(std::vector<const_Hypertrie<htt_t, allocator_type>> hypertries, std::vector<poss_type> positions) noexcept
			: hypertries_(std::move(hypertries)), positions_(std::move(positions)) {}

		class iterator {

		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::pair<key_part_type, std::vector<const_Hypertrie<htt_t, allocator_type>>>;
			using difference_type = ptrdiff_t;
			using pointer = value_type *;
			using reference = value_type &;

		private:
			poss_type pos_in_out_{};
			std::vector<internal::pos_type> result_depths_{};
			std::vector<HashDiagonal<htt_t, allocator_type>> ops_{};

			bool ended = false;

			/**
			 * Pair of key_part_type and a vector of const_Hypertries.
			 */
			value_type value{};

		public:
			iterator() = default;

			explicit iterator(const HashJoin &join) noexcept {
				auto max_op_count = join.hypertries_.size();
				pos_in_out_.reserve(max_op_count);
				result_depths_.reserve(max_op_count);
				ops_.reserve(max_op_count);

				internal::pos_type out_pos = 0;
				for (size_t pos = 0; pos < join.hypertries_.size(); ++pos) {
					const auto &join_poss = join.positions_[pos];
					const auto &hypertrie = join.hypertries_[pos];
					if (size(join_poss) > 0) {
						ops_.emplace_back(hypertrie, internal::raw::RawKeyPositions<hypertrie_max_depth>(join_poss));
						auto result_depth = result_depths_.emplace_back(hypertrie.depth() - size(join_poss));
						if (result_depth) {
							pos_in_out_.push_back(out_pos++);
							value.second.emplace_back();// only a placeholder, is to be replaced in next()
						} else {
							pos_in_out_.push_back(std::numeric_limits<internal::pos_type>::max());
						}
					} else {
						assert(hypertrie.depth() != 0);   // TODO: currently not possible
						value.second.push_back(hypertrie);// this stays unchanged during the iteration
						++out_pos;
					}
				}
				optimizeOperandOrder();
				ops_.front().begin();
				next(true);
			}

			inline void next(bool init = false) noexcept {
				// _current_key_part is increased if containsAndUpdateLower returns false
				HashDiagonal<htt_t, allocator_type> &smallest_operand = ops_.front();
				if (not init and not smallest_operand.ended())
					++smallest_operand;

				while (not smallest_operand.ended()) {
					// key_part
					value.first = smallest_operand.current_key_part();

					bool found = true;
					// iterate all but the first Diagonal
					for (size_t op_pos = 1; op_pos < ops_.size(); ++op_pos) {
						auto &operand = ops_[op_pos];
						if (not operand.find(value.first)) {
							found = false;
							break;
						}
					}
					if (found) {
						for (size_t op_pos = 0; op_pos < ops_.size(); ++op_pos) {
							if (const auto &result_depth = result_depths_[op_pos]; result_depth) {
								// vector of resulting const_Hypertries
								value.second[pos_in_out_[op_pos]] = const_Hypertrie<htt_t, allocator_type>(ops_[op_pos].current_hypertrie());
							}
						}
						return;
					}
					++smallest_operand;
				}
				// if no more candidates are available at the smallest_operand the join is done
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

			const value_type &operator*() const noexcept {
				return value;
			}

			value_type const *operator->() const noexcept { return &value; }

		private:
			void optimizeOperandOrder() noexcept {
				using namespace internal::util;
				const auto permutation = sort_permutation::get<HashDiagonal<htt_t, allocator_type>>(ops_);
				sort_permutation::apply(ops_, permutation);
				sort_permutation::apply(pos_in_out_, permutation);
				sort_permutation::apply(result_depths_, permutation);
			}
		};

		[[nodiscard]] iterator begin() const noexcept { return iterator(*this); }

		[[nodiscard]] bool end() const noexcept { return false; }
	};

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	class HashJoin<htt_t, allocator_type, true> {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		using poss_type = std::vector<internal::pos_type>;

	private:
		std::vector<const_Hypertrie<htt_t, allocator_type>> hypertries_;
		std::vector<poss_type> positions_;
		poss_type non_optional_positions_;

	public:
		HashJoin() = default;

		HashJoin(std::vector<const_Hypertrie<htt_t, allocator_type>> hypertries, std::vector<poss_type> positions, poss_type non_opt) noexcept
			: hypertries_(std::move(hypertries)), positions_(std::move(positions)), non_optional_positions_(std::move(non_opt)) {}

		class iterator {

		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::pair<key_part_type, std::vector<const_Hypertrie<htt_t, allocator_type>>>;
			using difference_type = ptrdiff_t;
			using pointer = value_type *;
			using reference = value_type &;

		private:
			poss_type pos_in_out_{};
			poss_type result_depths_{};
			poss_type ops_non_optional_positions_{};
			std::vector<HashDiagonal<htt_t, allocator_type>> ops_{};

			bool ended = false;

			value_type value{};

		public:
			iterator() = default;

			explicit iterator(const HashJoin &join) noexcept {
				auto max_op_count = join.hypertries_.size();
				pos_in_out_.reserve(max_op_count);
				result_depths_.reserve(max_op_count);
				ops_.reserve(max_op_count);

				internal::pos_type out_pos = 0;
				for (size_t pos = 0; pos < join.hypertries_.size(); ++pos) {
					const auto &join_poss = join.positions_[pos];
					const auto &hypertrie = join.hypertries_[pos];
					if (size(join_poss) > 0) {
						ops_.emplace_back(hypertrie, internal::raw::RawKeyPositions<hypertrie_max_depth>(join_poss));
						if (std::find(join.non_optional_positions_.begin(), join.non_optional_positions_.end(), pos) !=
							join.non_optional_positions_.end())
							ops_non_optional_positions_.push_back(ops_.size() - 1);
						pos_in_out_.push_back(out_pos++);
						value.second.emplace_back();// only a place holder, is to be replaced in next()
						result_depths_.emplace_back(hypertrie.depth() - size(join_poss));
					} else {
						value.second.push_back(hypertrie);// this stays unchanged during the iteration
						++out_pos;
					}
				}
				optimizeOperandOrder();
				ops_.front().begin();
				next(true);
			}

			inline void next(bool init = false) noexcept {
				// _current_key_part is increased if containsAndUpdateLower returns false
				HashDiagonal<htt_t, allocator_type> &smallest_operand = ops_.front();
				if (not init and not smallest_operand.ended())
					++smallest_operand;

				while (not smallest_operand.ended()) {
					value.first = smallest_operand.current_key_part();
					bool found = true;
					// iterate all but the first Diagonal
					for (size_t op_pos = 1; op_pos < ops_.size(); ++op_pos) {
						auto &operand = ops_[op_pos];
						if (not operand.find(value.first)) {
							if (std::find(ops_non_optional_positions_.begin(), ops_non_optional_positions_.end(), op_pos) !=
								ops_non_optional_positions_.end()) {
								found = false;
								break;
							}
							value.second[pos_in_out_[op_pos]] = const_Hypertrie<htt_t, allocator_type>();
						} else {
							if (const auto &result_depth = result_depths_.at(op_pos); result_depth) {
								value.second[pos_in_out_[op_pos]] = const_Hypertrie<htt_t, allocator_type>(ops_[op_pos].current_hypertrie());
							} else {
								value.second[pos_in_out_[op_pos]] = ops_[op_pos].current_scalar_as_tensor();
							}
						}
					}
					if (found) {
						// assign hypertrie to smallest operand
						if (const auto &result_depth = result_depths_.at(0); result_depth) {
							value.second[pos_in_out_[0]] = const_Hypertrie<htt_t, allocator_type>(ops_[0].current_hypertrie());
						} else {
							value.second[pos_in_out_[0]] = ops_[0].current_scalar_as_tensor();
						}
						return;
					}
					++smallest_operand;
				}
				ended = true;
			}

			iterator &
			operator++() noexcept {
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

			const value_type &operator*() const noexcept {
				return value;
			}

			value_type const *operator->() const noexcept { return &value; }

		private:
			void optimizeOperandOrder() {
				using namespace internal::util;
				std::vector<HashDiagonal<htt_t, allocator_type> *> ops_at_positions{};// keeps the diagonals of non-optional operands
				for (auto pos : ops_non_optional_positions_) {
					ops_at_positions.push_back(&ops_[pos]);
				}
				auto permutation = sort_permutation::get<HashDiagonal<htt_t, allocator_type> *>(ops_at_positions);
				sort_permutation::apply(ops_non_optional_positions_, permutation);
				for (size_t pos = 0; pos < ops_.size(); pos++) {
					if (std::find(ops_non_optional_positions_.begin(), ops_non_optional_positions_.end(), pos) ==
						ops_non_optional_positions_.end())
						permutation.push_back(permutation.size());
				}
				assert(permutation.size() == ops_.size());// todo: remove
				sort_permutation::apply(ops_, permutation);
				sort_permutation::apply(pos_in_out_, permutation);
				sort_permutation::apply(result_depths_, permutation);
			}
		};

		iterator begin() const noexcept { return iterator(*this); }

		[[nodiscard]] bool end() const noexcept { return false; }
	};

}// namespace dice::hypertrie

#endif//HYPERTRIE_HASHJOIN_IMPL_HPP
