//
// Created by Burhan Otour on 09.01.20.
//

#include <cassert>
#include <stdint.h>
#include <cassert>

namespace hypertrie::internal::util {
    template<typename T, typename key_part_type_t, int alignedTo>
    class KeyPartTaggedPointer {
    private:
        static_assert(
                alignedTo != 0 && ((alignedTo & (alignedTo - 1)) == 0),
                "Alignment parameter must be power of two"
        );
        static_assert(
                alignedTo > 1,
                "Pointer must be at least 2-byte aligned in order to store an int"
        );

        // for 8 byte alignment tagMask = alignedTo - 1 = 8 - 1 = 7 = 0b111
        // i.e. the lowest three bits are set, which is where the tag is stored
        static const intptr_t tagMask = alignedTo - 1;

        // pointerMask is the exact contrary: 0b...11111000
        // i.e. all bits apart from the three lowest are set, which is where the pointer is stored
        static const intptr_t pointerMask = ~tagMask;
    public:
        static constexpr int INT_TAG = 1;

        static constexpr int POINTER_TAG = 0;

    protected:
        // save us some reinterpret_casts with a union
        union {
            T *asPointer;
            uintptr_t asBits;
        };

    public:
        inline KeyPartTaggedPointer(T *pointer) {
            setPointer(pointer);
        }

        inline KeyPartTaggedPointer(key_part_type_t &number) {
            setInt(number);
        }

        ~KeyPartTaggedPointer() {
            if (getTag() == POINTER_TAG) {
                //    delete asPointer;
            }
        }

        inline void setPointer(T *pointer) {
            // make sure that the pointer really is aligned
            assert((reinterpret_cast<intptr_t>(pointer) & tagMask) == 0);
            // make sure that the tag isn't too large
            assert((POINTER_TAG & pointerMask) == 0);

            asPointer = pointer;
            asBits |= POINTER_TAG;
        }

        inline void setInt(key_part_type_t number) {
            asBits = reinterpret_cast<uintptr_t>(number);
            asBits |= INT_TAG;
        }

        inline T *getPointer() const {
            return reinterpret_cast<T *>(asBits & pointerMask);
        }

        inline int getTag() const {
            return (asBits & tagMask);
        }

        inline key_part_type_t getInt() const {
            return reinterpret_cast<key_part_type_t>(asBits & pointerMask);
        }
    };

    template<typename CompressedNodeTypePtr, typename NodeTypePtr, int alignedTo>
    class CompressedBoolHyperTrieTaggedPointer {
    private:
        static_assert(
                alignedTo != 0 && ((alignedTo & (alignedTo - 1)) == 0),
                "Alignment parameter must be power of two"
        );
        static_assert(
                alignedTo > 1,
                "Pointer must be at least 2-byte aligned in order to store an int"
        );

        // for 8 byte alignment tagMask = alignedTo - 1 = 8 - 1 = 7 = 0b111
        // i.e. the lowest three bits are set, which is where the tag is stored
        static const intptr_t tagMask = alignedTo - 1;

        // pointerMask is the exact contrary: 0b...11111000
        // i.e. all bits apart from the three lowest are set, which is where the pointer is stored
        static const intptr_t pointerMask = ~tagMask;
    public:
        static constexpr int COMPRESSED_TAG = 1;

        static constexpr int NON_COMPRESSED_TAG = 0;

    protected:
        // save us some reinterpret_casts with a union
        union {
            void *asPointer;
            uintptr_t asBits;
        };

    public:
        inline CompressedBoolHyperTrieTaggedPointer(CompressedNodeTypePtr compressedNode) {
            // Safer to clear the tag as we set a fresh tag in the setter method
            clearTag();
            setCompressedNode(compressedNode);
        }

        inline CompressedBoolHyperTrieTaggedPointer(NodeTypePtr node) {
            // Safer to clear the tag as we set a fresh tag in the setter method
            clearTag();
            setNode(node);
        }

        /**
         *
         * @param ptr it is already a tagged pointer
         */
        inline CompressedBoolHyperTrieTaggedPointer(void *ptr) {
            // DON'T CLEAR THE TAG HERE AT ALL, as we lose the compression status information
            asPointer = ptr;
        }

        inline CompressedBoolHyperTrieTaggedPointer() {
            asPointer = nullptr;
        }

    protected:
        inline void setCompressedNode(CompressedNodeTypePtr ptr) {
            // make sure that the pointer value is really aligned
            assert((reinterpret_cast<intptr_t>(ptr) & tagMask) == 0);
            // make sure that the tag isn't too large
            assert((COMPRESSED_TAG & pointerMask) == 0);

            asPointer = ptr;
            asBits |= COMPRESSED_TAG;
        }

        inline void setNode(NodeTypePtr node) {
            // make sure that the tag isn't too large
            assert((COMPRESSED_TAG & pointerMask) == 0);

            asPointer = node;
            asBits |= NON_COMPRESSED_TAG;
        }

        inline void clearTag() {
            asBits &= pointerMask;
        }

    public:
        inline int getTag() const {
            return (asBits & tagMask);
        }

        void *getPointer() const {
            return asPointer;
        }

        inline bool isEmpty() const {
            return asPointer == nullptr;
        }

        inline CompressedNodeTypePtr getCompressedNode() const {
            return reinterpret_cast<CompressedNodeTypePtr>(asBits & pointerMask);
        }

        inline NodeTypePtr getNode() const {
            return reinterpret_cast<NodeTypePtr>(asBits & pointerMask);
        }
    };
}
