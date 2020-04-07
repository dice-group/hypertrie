//
// Created by root on 4/5/20.
//

#ifndef HYPERTRIE_COMPRESSEDBOOLHYPERTRIE_HPP
#define HYPERTRIE_COMPRESSEDBOOLHYPERTRIE_HPP
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"

#include "Dice/hypertrie/internal/container/AllContainer.hpp"

#include "Dice/hypertrie/internal/compressed/CompressedBoolHypertrie_impl.hpp"
#include "Dice/hypertrie/internal/compressed/CompressedBoolHypertrie_Hash_Diagonal_impl.hpp"

namespace hypertrie::internal::compressed::interface {
    template<typename key_part_type, template<typename, typename> class map_type,
            template<typename> class set_type>
    struct compressedboolhypertrie {
        using CompressedBoolHypertrie = hypertrie::internal::compressed::CompressedBoolHypertrie<key_part_type, map_type, set_type>;
        using const_CompressedBoolHypertrie = hypertrie::internal::compressed::const_CompressedBoolHypertrie<key_part_type, map_type, set_type>;
        using CompressedHashDiagonal = hypertrie::internal::compressed::CompressedHashDiagonal<key_part_type, map_type, set_type>;
        // using OrderedDiagonal = hypertrie::internal::OrderedDiagonal<key_part_type, map_type, set_type>;
    };
}
#endif //HYPERTRIE_COMPRESSEDBOOLHYPERTRIE_HPP
