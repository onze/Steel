#ifndef STEEL_BTNODETYPES_H_
#define STEEL_BTNODETYPES_H_

#include <map>
#include <OgreString.h>

namespace Steel
{

    /// tells the type of processing a node does.
    enum BTShapeTokenType
    {
        // Should stay first enum value
        _BTFirst=-1,

        BTSequenceToken,
        BTSelectorToken,
        BTDecoratorToken,
        BTCounterToken,
        BTLocalizerToken,
        BTNavigatorToken,
        BTUnknownToken,

        // Should stay last value
        _BTLast
    };

    /// POD
    class BTShapeToken
    {
        public:
            BTShapeTokenType type;
            /// node index of the token
            unsigned begin;
            /// node index of the next token (last child's +1)
            unsigned end;
            bool operator==(BTShapeToken const &o) const
            {
                return type==o.type && begin==o.begin && end==o.end;
            }
    };

    /** Flatten BTree (the tree shape). This is the abstract part of a tree,
     * that's shared amongst all models using this shape.*/
    typedef std::vector<BTShapeToken> BTShapeStream;

    /// Shapes storage structure
    typedef std::map<Ogre::String,BTShapeStream> BTShapeStreamMap;

    /// Allows for slow lookup
    extern std::vector<Ogre::String> BTShapeTokenTypeAsString;

    /// BTNodes state values.
    enum BTState
    {
        READY=0,
        RUNNING,
        SUCCESS,
        FAILURE,
    };
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

#endif
