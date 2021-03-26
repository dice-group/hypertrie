#ifndef HYPERTRIE_NODECOMPRESSION_HPP
#define HYPERTRIE_NODECOMPRESSION_HPP

namespace hypertrie::internal::raw {
	enum class NodeCompression : bool {
		uncompressed = false,
		compressed = true
	};

}
#endif//HYPERTRIE_NODECOMPRESSION_HPP
