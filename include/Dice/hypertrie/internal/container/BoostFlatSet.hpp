#ifndef HYPERTRIE_BOOSTFLATSET_HPP
#define HYPERTRIE_BOOSTFLATSET_HPP

#include <boost/container/flat_set.hpp>

namespace hypertrie::internal::container {
    template<typename key_type, typename Allocator = boost::container::new_allocator<key_type>>
    using boost_flat_set = boost::container::flat_set<
            key_type,
			std::less<>,
			typename std::allocator_traits<Allocator>::template rebind_alloc<key_type>
            >;
}

#endif //HYPERTRIE_BOOSTFLATSET_HPP
