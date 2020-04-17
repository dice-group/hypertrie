#ifndef HYPERTRIE_EINSUM_ITERFACE_HPP
#define HYPERTRIE_EINSUM_ITERFACE_HPP

#include "Dice/einsum/internal/Einsum.hpp"

namespace einsum::internal::interface {

    template<typename key_part_type, template<typename, typename> class map_type,
            template<typename> class set_type>
    struct einsum_interface {

        template<typename key_part_type_a, template<typename, typename> class map_type_a,
                template<typename> class set_type_a>
        using const_BoolHypertrie = hypertrie::internal::const_BoolHypertrie<key_part_type_a, map_type_a, set_type_a>;
        template<typename key_part_type_a, template<typename, typename> class map_type_a,
                template<typename> class set_type_a>
        using const_CompressedBoolHypertrie = hypertrie::internal::compressed::const_CompressedBoolHypertrie<key_part_type_a, map_type_a, set_type_a>;

        using CompressedHashDiagonal = typename hypertrie::internal::compressed::interface::compressedboolhypertrie<key_part_type, map_type, set_type>::CompressedHashDiagonal;

        using HashDiagonal = typename hypertrie::internal::interface::boolhypertrie<key_part_type, map_type, set_type>::HashDiagonal;

        template <typename value_type>
        using Einsum = einsum::internal::Einsum<value_type, key_part_type, map_type, set_type, const_BoolHypertrie, HashDiagonal>;

        template <typename value_type>
        using CompressedEinsum = einsum::internal::Einsum<value_type, key_part_type, map_type, set_type, const_CompressedBoolHypertrie, CompressedHashDiagonal>;
    };
}
#endif //HYPERTRIE_EINSUM_ITERFACE_HPP
