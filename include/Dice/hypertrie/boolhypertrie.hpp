#ifndef HYPERTRIE_BOOLHYPERTRIE_HPP
#define HYPERTRIE_BOOLHYPERTRIE_HPP

#include "Dice/hypertrie/internal/BoolHypertrie.hpp"
#include "Dice/hypertrie/internal/compressed/CompressedBoolHypertrie.hpp"
#include "Dice/hypertrie/internal/RawBoolHypertrie.hpp"
#include "Dice/hypertrie/internal/Join.hpp"
#include "Dice/hypertrie/internal/HashJoin_impl.hpp"

#include "Dice/hypertrie/internal/compressed/RawCompressedBoolHypertrie.hpp"
#include <vector>
#include <map>
#include <itertools.hpp>

#include "Dice/hypertrie/boolhypertrie.hpp"
#include "Dice/hypertrie/internal/util/FrontSkipIterator.hpp"

#include "Dice/einsum/internal/Einsum.hpp"
#include "Dice/einsum/internal/Einsum_interface.hpp"

namespace hypertrie {
    using Subscript =  ::einsum::internal::Subscript;
    using RawSubscript = ::einsum::internal::RawSubscript;

    template<typename key_part_type = unsigned long, template<typename, typename> class map_type = hypertrie::internal::container::tsl_sparse_map,
            template<typename> class set_type = hypertrie::internal::container::boost_flat_set>
    struct boolhypertrie {
        using pos_type = hypertrie::internal::pos_type;
        /**
         * Non-Compressed Data Structures
         */
        using BoolHypertrie = typename ::hypertrie::internal::interface::boolhypertrie<key_part_type, map_type, set_type>::BoolHypertrie;

        using const_BoolHypertrie = typename ::hypertrie::internal::interface::boolhypertrie<key_part_type, map_type, set_type>::const_BoolHypertrie;

        using OrderedDiagonal = typename ::hypertrie::internal::interface::boolhypertrie<key_part_type, map_type, set_type>::OrderedDiagonal;
        using HashDiagonal = typename ::hypertrie::internal::interface::boolhypertrie<key_part_type, map_type, set_type>::HashDiagonal;

        using OrderedJoin = typename ::hypertrie::internal::interface::join<key_part_type, map_type, set_type>::OrderedJoin;
        using HashJoin = typename ::hypertrie::internal::interface::join<key_part_type, map_type, set_type>::template HashJoin<const_BoolHypertrie, HashDiagonal>;

        template<typename value_type>
        using Einsum = typename ::einsum::internal::interface::einsum_interface<key_part_type, map_type, set_type>::template Einsum<value_type>;

        template<typename value_type>
        using CompressedEinsum = typename ::einsum::internal::interface::einsum_interface<key_part_type, map_type, set_type>::template CompressedEinsum<value_type>;

        template<pos_type depth>
        using RawBoolhypertrie = typename ::hypertrie::internal::interface::rawboolhypertrie<key_part_type, map_type, set_type>::template RawBoolHypertrie<depth>;

        template<typename value_type = size_t>
        using EinsumEntry = ::einsum::internal::Entry<key_part_type, value_type>;

        using KeyHash = ::einsum::internal::KeyHash<key_part_type>;
        using TimePoint = ::einsum::internal::TimePoint;

        /**
         * Compressed Data Structure
         */
        using CompressedHashDiagonal = typename ::hypertrie::internal::compressed::interface::compressedboolhypertrie<key_part_type, map_type, set_type>::CompressedHashDiagonal;

        using CompressedBoolHypertrie = typename ::hypertrie::internal::compressed::interface::compressedboolhypertrie<key_part_type, map_type, set_type>::CompressedBoolHypertrie;

        using const_CompressedBoolHypertrie = typename ::hypertrie::internal::compressed::interface::compressedboolhypertrie<key_part_type, map_type, set_type>::const_CompressedBoolHypertrie;

        template<pos_type depth, bool compressed>
        using RawCompressedBoolhypertrie = typename ::hypertrie::internal::compressed::interface::rawcompressedboolhypertrie<key_part_type, map_type, set_type>::template RawCompressedBoolHypertrie<depth, compressed>;

        using CompressedHashJoin = typename ::hypertrie::internal::interface::join<key_part_type, map_type, set_type>::template HashJoin<const_CompressedBoolHypertrie, CompressedHashDiagonal>;


        template<typename value_type = std::size_t>
        static auto
        einsum2map(const std::shared_ptr<Subscript> &subscript,
                   const std::vector<const_BoolHypertrie> &operands,
                   const TimePoint &time_point = TimePoint::max()) {
            tsl::hopscotch_map<std::vector<key_part_type>, value_type, KeyHash> results{};
            for (const auto &operand : operands)
                if (operand.size() == 0)
                    return results;

            Einsum<value_type> einsum{subscript, operands, time_point};
            for (auto &&entry : einsum) {
                results[entry.key] += entry.value;
            }
            return results;
        }
    };

}

#endif //HYPERTRIE_BOOLHYPERTRIE_HPP
