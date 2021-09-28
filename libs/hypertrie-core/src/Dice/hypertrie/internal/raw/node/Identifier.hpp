#ifndef HYPERTRIE_IDENTIFIER_HPP
#define HYPERTRIE_IDENTIFIER_HPP

#include "Dice/hypertrie/internal/raw/node/Hash_or_InplaceNode.hpp"

namespace hypertrie::internal::raw {

	template<size_t depth, HypertrieCoreTrait tri>
	using Identifier = std::conditional_t<(tri::is_bool_valued and tri::taggable_key_part),
										  Hash_or_InplaceNode<depth, tri>,
										  TensorHash<depth, tri>>;

}
#endif//HYPERTRIE_IDENTIFIER_HPP
