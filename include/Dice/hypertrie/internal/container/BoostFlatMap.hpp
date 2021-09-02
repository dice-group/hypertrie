#ifndef HYPERTRIE_BOOSTFLATMAP_HPP
#define HYPERTRIE_BOOSTFLATMAP_HPP

#include <boost/container/flat_map.hpp>

namespace hypertrie::internal::container {
    template<typename key_type, typename value, typename Allocator = boost::container::new_allocator<std::pair<key_type, value>>>
    using boost_flat_map = boost::container::flat_map
            <
                    key_type,
                    value,
                    std::less<key_type>,
					typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<const key_type, value>>
            >;
}

#endif //HYPERTRIE_BOOSTFLATMAP_HPP
