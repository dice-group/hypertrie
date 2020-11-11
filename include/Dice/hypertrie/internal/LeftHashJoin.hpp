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
            using value_type = std::tuple<std::vector<std::optional<const_Hypertrie<tr>>>, key_part_type, poss_type>;
            // members
            poss_type pos_in_out{};
			poss_type join_ops_in_out{};
			std::vector<pos_type> non_optional_poss;
            std::vector<pos_type> result_depths{};
            std::vector<HashDiagonal<tr>> ops{};
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
				non_optional_poss = join.non_optional_poss;

                pos_type out_pos = 0;
                for (const auto &[pos, join_poss, hypertrie] : iter::zip(iter::range(join.hypertries.size()), join.positions,
                                                                         join.hypertries)) {
                    if (size(join_poss) > 0) {
                        ops.emplace_back(HashDiagonal<tr>{hypertrie, join_poss});
                        auto result_depth = result_depths.emplace_back(hypertrie.depth() - size(join_poss));
                        if (result_depth) {
                            pos_in_out.push_back(out_pos);
                            join_ops_in_out.push_back(out_pos++);
                            std::get<0>(value).push_back(hypertrie); // only a place holder, is to be replaced in next()
                        } else {
                            pos_in_out.push_back(std::numeric_limits<pos_type>::max());
							join_ops_in_out.push_back(std::numeric_limits<pos_type>::max());
                        }
                    } else {
                        assert(hypertrie.depth() != 0); // TODO: currently not possible
                        std::get<0>(value).push_back(hypertrie); // this stays unchanged during the iteration
                        pos_in_out.push_back(out_pos++);
                    }
                }
				// TODO: choose optimal operand
                left_operand = &ops.front();
                left_operand->begin();
                next();
            }

            inline void next() {
                while(*left_operand) {
					bool found = false;
					if (result_depths[0])
						std::get<0>(value)[pos_in_out[0]] = const_Hypertrie<tr>(left_operand->currentHypertrie());
					std::get<1>(value) = left_operand->currentKeyPart();
					// iterate all right operands
					for (typename std::vector<tr>::size_type i = 1; i < ops.size(); i++) {
						auto &right_operand = ops[i];
						// if the join was successful save the key of the right operand
						if (right_operand.find(std::get<1>(value))) {
							if (result_depths[i])
								std::get<0>(value)[join_ops_in_out[i]] = const_Hypertrie<tr>(right_operand.currentHypertrie());
						}
						// if the join was not successful do not return the operand
						else {
							// if this is a non-optional go to the next value
							if(std::find(this->non_optional_poss.begin(), this->non_optional_poss.end(), i)
								!= this->non_optional_poss.end()) {
								found = true;
								break;
							}
							else if (result_depths[i])
								std::get<0>(value)[join_ops_in_out[i]] = std::nullopt;
						}
					}
                    if(found) {
                        ++(*left_operand);
                        continue;
                    }
					// store the positions of the input operands in the results
					std::get<2>(value) = pos_in_out;
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

        };

    };

}

#endif//HYPERTRIE_LEFTHASHJOIN_HPP