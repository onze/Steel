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

            /// Returns true if the stream contains no state.
            bool empty();
            
            /// Removes all states from the stream, leaving it to empty()==true.
            void clear();

        protected:
            bool placeStateAt(size_t offset,BTShapeTokenType tokenType);
            size_t stateSize(Steel::BTShapeTokenType token);
            // not owned
            // owned
            /// Tells what token type is at what index
            std::vector<BTShapeTokenType> mTokenTypes;
            
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
