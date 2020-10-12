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


    public:
        // forward declaration of iterator
        class iterator;
        // methods
        LeftHashJoin() = default;

        LeftHashJoin(LeftHashJoin &) = default;

        LeftHashJoin(const LeftHashJoin &) = default;

        LeftHashJoin(std::vector<const_Hypertrie<tr>> hypertries, std::vector<poss_type> positions)
                : hypertries(std::move(hypertries)), positions(std::move(positions)) {}

        iterator begin() const { return iterator(*this); }

        [[nodiscard]] bool end() const { return false; }

        // definition of iterator class
        class iterator {

        private:
            // aliases
            using value_type = std::pair<std::vector<std::optional<const_Hypertrie<tr>>>, key_part_type>;
            // members
            poss_type pos_in_out{};
            std::vector<pos_type> result_depths{};
            std::vector<HashDiagonal<tr>> ops{};
            bool ended = false;
            value_type value{};
            HashDiagonal<tr>* left_operand = nullptr;
            static constexpr key_part_type default_key_part = []() {
              if constexpr (std::is_pointer_v<typename tr::key_part_type>) return nullptr;
              else return std::numeric_limits<typename tr::key_part_type>::max();
            }();

        public:

            iterator() = default;

            iterator(const LeftHashJoin &join) {
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
                left_operand = &ops.front();
                left_operand->begin();
                next();
            }

            inline void next() {
                if(not *left_operand) {
                    ended = true;
                    return;
                }
				if(result_depths[0])
				    value.first[pos_in_out[0]] = const_Hypertrie<tr>(left_operand->currentHypertrie());
                value.second = left_operand->currentKeyPart();
                // iterate all right operands
                for (typename std::vector<tr>::size_type i = 1; i < ops.size(); i++) {
                    auto &right_operand = ops[i];
                    // if the join was successful save the key of the right operand
                    if (right_operand.find(value.second)) {
						if (result_depths[i])
							value.first[pos_in_out[i]] = const_Hypertrie<tr>(right_operand.currentHypertrie());
					}
                    // if the join was not successful save the default key value
                    else {
						if (result_depths[i])
							value.first[pos_in_out[i]] = std::nullopt;
					}
                }
                ++(*left_operand);
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