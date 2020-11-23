#ifndef HYPERTRIE_LEFTHASHJOIN_HPP
#define HYPERTRIE_LEFTHASHJOIN_HPP

#include <utility>

#include "Dice/hypertrie/internal/Hypertrie.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include "Dice/hypertrie/internal/util/FrontSkipIterator.hpp"
#include "Dice/hypertrie/internal/util/PermutationSort.hpp"

namespace hypertrie {

    template<HypertrieTrait tr = default_bool_Hypertrie_t>
    class LeftHashJoin {

    private:
        // aliases
        using key_part_type = typename tr::key_part_type;
        using poss_type = std::vector<pos_type>;
        // members
        std::vector<const_Hypertrie<tr>> hypertries;
        std::vector<poss_type> positions;
        std::vector<pos_type> non_optional_poss;


    public:
        // forward declaration of iterator
        class iterator;
        // methods
        LeftHashJoin() = default;

        LeftHashJoin(LeftHashJoin &) = default;

        LeftHashJoin(const LeftHashJoin &) = default;

        LeftHashJoin(std::vector<const_Hypertrie<tr>> hypertries,
					 std::vector<poss_type> positions,
					 std::vector<pos_type> non_optional_poss)
                : hypertries(std::move(hypertries)), positions(std::move(positions)), non_optional_poss(std::move(non_optional_poss)) {}

        iterator begin() const { return iterator(*this); }

        [[nodiscard]] bool end() const { return false; }

        // definition of iterator class
        class iterator {

        private:
            // aliases
            using value_type = std::tuple<std::vector<std::optional<const_Hypertrie<tr>>>,key_part_type,
										  poss_type, std::vector<std::size_t>>;
            // members
			pos_type shortest_op_pos;
            poss_type pos_in_out{};
			poss_type join_ops_in_out{};
			std::vector<pos_type> non_optional_poss;
            std::vector<pos_type> ops_non_optional_poss{};
            std::vector<pos_type> result_depths{};
            std::vector<HashDiagonal<tr>> ops{};
			std::vector<std::size_t> joined{};
			std::vector<pos_type> original_poss;
            bool ended = false;
            value_type value{};
            HashDiagonal<tr>* left_operand = nullptr;

        public:

            iterator() = default;

            iterator(const LeftHashJoin &join) {
                auto max_op_count = join.hypertries.size();
                pos_in_out.reserve(max_op_count);
                result_depths.reserve(max_op_count);
                ops.reserve(max_op_count);
                joined.resize(max_op_count);
				non_optional_poss = join.non_optional_poss;

                pos_type out_pos = 0;
                for (const auto &[pos, join_poss, hypertrie] : iter::zip(iter::range(join.hypertries.size()), join.positions,
                                                                         join.hypertries)) {
                    if (size(join_poss) > 0) {
                        ops.emplace_back(HashDiagonal<tr>{hypertrie, join_poss});
						// store the positions of the ops vector that can be reordered
						if(std::find(non_optional_poss.begin(), non_optional_poss.end(), pos) != non_optional_poss.end())
							ops_non_optional_poss.push_back(ops.size() - 1);
                        auto result_depth = result_depths.emplace_back(hypertrie.depth() - size(join_poss));
                        if (result_depth) {
                            pos_in_out.push_back(out_pos);
                            join_ops_in_out.push_back(out_pos++);
                            std::get<0>(value).push_back(hypertrie); // only a place holder, is to be replaced in next()
                        } else {
                            pos_in_out.push_back(std::numeric_limits<pos_type>::max());
							join_ops_in_out.push_back(std::numeric_limits<pos_type>::max());
                        }
						original_poss.push_back(pos);
                    } else {
                        assert(hypertrie.depth() != 0); // TODO: currently not possible
                        std::get<0>(value).push_back(hypertrie); // this stays unchanged during the iteration
						joined[pos] = 1;
                        pos_in_out.push_back(out_pos++);
                    }
                }
				// TODO: choose optimal operand
				shortest_op_pos = findShortestOperand(ops_non_optional_poss);
                left_operand = &ops[shortest_op_pos];
                left_operand->begin();
                next();
            }

            inline void next() {
                while(*left_operand) {
					bool found = false;
					if (result_depths[shortest_op_pos])
						std::get<0>(value)[join_ops_in_out[shortest_op_pos]] = const_Hypertrie<tr>(left_operand->currentHypertrie());
					std::get<1>(value) = left_operand->currentKeyPart();
					// iterate all right operands
					for (typename std::vector<tr>::size_type i = 0; i < ops.size(); i++) {
						// skip shortest operand
						if(i == shortest_op_pos) {
							joined[original_poss[i]] = 1;
							continue;
						}
						auto &right_operand = ops[i];
						// if the join was successful save the key of the right operand
						if (right_operand.find(std::get<1>(value))) {
							joined[original_poss[i]] = 1;
							if (result_depths[i])
								std::get<0>(value)[join_ops_in_out[i]] = const_Hypertrie<tr>(right_operand.currentHypertrie());
						}
						// if the join was not successful do not return the operand
						else {
							// if this is a non-optional go to the next value
							if(std::find(this->ops_non_optional_poss.begin(), this->ops_non_optional_poss.end(), i)
								!= this->ops_non_optional_poss.end()) {
								found = true;
								break;
							}
							else if(result_depths[i])
                                    std::get<0>(value)[join_ops_in_out[i]] = std::nullopt;
                            joined[original_poss[i]] = 0;
						}
					}
                    if(found) {
                        ++(*left_operand);
                        continue;
                    }
					// store the positions of the input operands in the results
					std::get<2>(value) = pos_in_out;
                    std::get<3>(value) = joined;
					++(*left_operand);
					return;
				}
                ended = true;
                return;
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

            pos_type findShortestOperand(const std::vector<pos_type>& poss) {
				pos_type shortest_pos = poss[0];
				auto shortest_size = ops[poss[0]].size();
                for(auto it = poss.begin(); it != poss.end(); it++) {
					if(ops[*it].size() < shortest_size) {
						shortest_pos = *it;
						shortest_size = ops[*it].size();
					}
				}
				return shortest_pos;
			}

        };

    };

}

#endif//HYPERTRIE_LEFTHASHJOIN_HPP