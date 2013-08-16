#ifndef BTSTATESTREAM_H
#define BTSTATESTREAM_H

#include <vector>

#include <BT/btnodetypes.h>
#include "BTNode.h"

namespace Steel
{
    /**
     * A BTStateStream contains a vector of BTNode subclasses. 
     * It builds from s shapeStream, and grants access to its states like a regular vector,
     * despite holding instances of differents classes.
     */
    class BTStateStream
    {
        public:
            BTStateStream();
            BTStateStream(const BTStateStream& other);
            virtual ~BTStateStream();
            virtual BTStateStream& operator=(const BTStateStream& other);
            virtual bool operator==(const BTStateStream& other) const;

            bool init(BTShapeStream *shapeStream);

            /// Returns the type of token at the given index, in depth-first order.
            BTShapeTokenType tokenTypeAt(BTStateIndex index);

            /// Returns the token at the given index, in depth-first order.
            BTShapeToken tokenAt(BTStateIndex index);

            /// Returns the BTNode subclass at the given index. Returns NULL if index is invalid.
            BTNode* stateAt(BTStateIndex index);
            void *rootState()
            {
                return stateAt(0UL);
            }

            /// Returns true if the stream contains no state.
            bool empty();

            /// Removes all states from the stream, leaving it to empty()==true.
            void clear();

            Ogre::String debugName();
            BTShapeStream * const shapeStream()
            {
                return mShapeStream;
            }

        protected:
            /// Allocate states according to the given shape.
            bool buildFromShapeStream(BTShapeStream *shapeStream);
            bool placeStateAt(size_t offset, BTShapeToken &token);
            size_t sizeOfState(Steel::BTShapeTokenType token);

            // not owned
            /// The shape (as tokens).
            BTShapeStream *mShapeStream;

            // owned
            /// Tells the offset of each state
            std::vector<size_t> mStateOffsets;

            /// Raw BTState storage.
            void *mData;

            /// total size of mData
            size_t mDataSize;
    };
}
#endif // BTSTATESTREAM_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
