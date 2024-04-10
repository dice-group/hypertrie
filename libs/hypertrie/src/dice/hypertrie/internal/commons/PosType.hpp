#ifndef HYPERTRIE_POS_TYPE_HPP
#define HYPERTRIE_POS_TYPE_HPP

namespace dice::hypertrie::internal {
	/**
	 * pos_type is used for key_part positions of hypertrie keys. As hypertrie's should always have depth/no of dimensions <=265, 8 bit are enough to encode a hypertrie key_part position. Positions are counted starting from 0.
	 */
	using pos_type = uint8_t;
} // namespace dice::hypertrie::internal

#endif //HYPERTRIE_POS_TYPE_HPP
