//
// Created by burhan on 29.01.20.
//

#ifndef HYPERTRIE_RAWCOMPRESSEDBOOLHYPERTRIE_HPP
#define HYPERTRIE_RAWCOMPRESSEDBOOLHYPERTRIE_HPP

#include "Dice/hypertrie/internal/compressed/RawCompressedBoolHypertrie_impl.hpp"
#include "Dice/hypertrie/internal/compressed/RawCompressedBoolHypertrie_Hash_Diagonal_impl.hpp"

namespace hypertrie::internal::compressed::interface {
    template<typename key_part_type, template<typename, typename> typename map_type,
            template<typename> typename set_type>
    struct rawcompressedboolhypertrie {
        template<pos_type depth, bool compressed>
        using RawCompressedBoolHypertrie = hypertrie::internal::compressed::RawCompressedBoolHypertrie<depth, key_part_type, map_type, set_type, compressed>;

        template<pos_type depth, pos_type diag_depth, bool compressed>
        using RawCompressedBHTHashDiagonal = hypertrie::internal::compressed::RawCompressedBHTHashDiagonal<depth, diag_depth, compressed, key_part_type, map_type, set_type>;

    };

}

#endif //HYPERTRIE_RAWCOMPRESSEDBOOLHYPERTRIE_HPP
