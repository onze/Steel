#ifndef BTSTATESTREAM_H
#define BTSTATESTREAM_H

#include <vector>

#include <BT/btnodetypes.h>

namespace Steel
{
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
            BTShapeTokenType tokenTypeAt(size_t index);
            
            void *stateAt(size_t index);

            /// Returns true if the stream contains no state.
            bool empty();
            
            /// Removes all states from the stream, leaving it to empty()==true.
            void clear();
            
            Ogre::String debugName();

        protected:
            /// Allocate states according to the given shape.
            bool builFromShapeStream(BTShapeStream *shapeStream);
            bool placeStateAt(size_t offset,BTShapeToken &token);
            size_t sizeOfState(Steel::BTShapeTokenType token);
            
            // not owned
            /// The shape (contains nodes indices).
            BTShapeStream *mShapeStream;
            
            // owned
//             /// Tells what token type is at what index
//             std::vector<BTShapeTokenType> mTokenTypes;
            
            /// Tells the offset of each state
            std::vector<size_t> mStateOffsets;
            
            /// Raw states storage.
            void *mData;
            
            /// total size of mData 
            size_t mDataSize;
    };
}
#endif // BTSTATESTREAM_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
