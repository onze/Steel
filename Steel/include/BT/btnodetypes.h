#ifndef STEEL_BTNODETYPES_H_
#define STEEL_BTNODETYPES_H_

#include <map>
#include <OgreString.h>

namespace Steel
{
    /// BTNodes state values.
    enum BTState
    {
        // Should stay first enum value
        _FIRST = -1,

        READY,
        RUNNING,
        SUCCESS,
        FAILURE,
        SKIPT_TO
    };

    /// Allows for slow lookup. Aligned on BTShapeTokenType.
    extern std::vector<Ogre::String> BTStateAsString;

    /// tells the type of processing a node does.
    enum BTShapeTokenType
    {
        // Should stay first enum value
        _BTFirst = -1,

        BTSequenceToken,
        BTSelectorToken,
//         BTDecoratorToken,
//         BTCounterToken,
        BTFinderToken,
        BTNavigatorToken,
        BTSignalListenerToken,
        BTDebugToken,

        BTUnknownToken,

        // Should stay last value
        _BTLast
    };

    /// Allows for slow lookup. Aligned on BTShapeTokenType.
    extern std::vector<Ogre::String> BTShapeTokenTypeAsString;

    /// Index within a BTStateStream.
    typedef size_t BTStateIndex;
    /// Memory offset within a BTStateStream.mData
    typedef size_t BTStateOffset;

    /// Holds shape information
    class BTShapeToken
    {
        public:
            //BTShapeToken():type(BTUnknownToken),begin(0UL),end(0UL),contentFile(""){}

            BTShapeTokenType type;
            /// node index of the token
            BTStateIndex begin;
            /// node index of the next token (last child's +1)
            BTStateIndex end;
            /// path to file with parameters
            Ogre::String contentFile;

            bool operator==(BTShapeToken const &o) const
            {
                return type == o.type && begin == o.begin && end == o.end && contentFile == o.contentFile;
            }
    };

    /** Flatten BTree (the tree shape). This is the abstract part of a tree,
     * that's shared amongst all models using this shape.*/
    typedef std::vector<BTShapeToken> BTShapeStreamData;

    class BTShapeStream
    {
        public:
            Ogre::String mName;
            BTShapeStreamData mData;
    };

    /// Shapes storage structure
    typedef std::map<Ogre::String, BTShapeStream> BTShapeStreamMap;
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

#endif
