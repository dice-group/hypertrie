#ifndef HYPERTRIE_HYPERTRIEPREDECLARE_HPP
#define HYPERTRIE_HYPERTRIEPREDECLARE_HPP

#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"

namespace dice::hypertrie {


	template<HypertrieTrait tr, ByteAllocator allocator_type>
	class const_Hypertrie;

	template<HypertrieTrait tr, ByteAllocator allocator_type>
	class Hypertrie;

}// namespace dice::hypertrie


#endif//HYPERTRIE_HYPERTRIEPREDECLARE_HPP
