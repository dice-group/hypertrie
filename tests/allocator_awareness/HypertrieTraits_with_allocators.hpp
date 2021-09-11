#ifndef HYPERTRIE_HYPERTRIETRAITS_WITH_ALLOCATORS_HPP
#define HYPERTRIE_HYPERTRIETRAITS_WITH_ALLOCATORS_HPP

#include "OffsetAllocator.hpp"
#include <metall/metall.hpp>
#include <Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp>
#include <tsl/boost_offset_pointer.h>


namespace tests::Dice::hypertrie::allocator_awareness {
    template<typename T>
    using m_alloc_t = metall::manager::allocator_type<T>;
    using namespace ::hypertrie::internal::raw;
    template <typename NewAllocator, bool NewLSB, typename Key, typename Value, typename Allocator,
            template<typename, typename, typename> class Map,
            template<typename, typename> class Set, bool LSB_UNUSED>
    auto inject_allocator_type(Hypertrie_internal_t<::hypertrie::Hypertrie_t<Key, Value, Allocator, Map, Set, LSB_UNUSED>>) {
        return Hypertrie_internal_t<::hypertrie::Hypertrie_t<Key, Value, NewAllocator, Map, Set, NewLSB>>{};
    }
    using tri = ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t;
    using offset_tri = decltype(inject_allocator_type<OffsetAllocator<std::byte>, true>(tri{}));
    using offset_tri_LSB_false = decltype(inject_allocator_type<OffsetAllocator<std::byte>, false>(tri{}));
    using metall_tri = decltype(inject_allocator_type<m_alloc_t<std::byte>, true>(tri{}));
    using metall_tri_LSB_false = decltype(inject_allocator_type<m_alloc_t<std::byte>, false>(tri{}));
}

namespace std {
    template<typename T>
    struct indirectly_readable_traits<boost::interprocess::offset_ptr<T>> {
        using value_type = typename boost::interprocess::offset_ptr<T>::value_type;
    };
}// namespace std

#endif //HYPERTRIE_HYPERTRIETRAITS_WITH_ALLOCATORS_HPP
