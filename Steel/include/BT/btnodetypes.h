#ifndef STEEL_BTNODETYPES_H_
#define STEEL_BTNODETYPES_H_

#include <map>
#include <OgreString.h>

namespace Steel
{

    /// tells the type of processing a node does.
    enum BTShapeToken
    {
        // Should stay first enum value
        _BTFirst=-1,

        BTSequence,
        BTSelector,
        BTDecorator,
        BTCounter,
        BTLocalizer,
        BTNavigator,
        BTUnknown,

        // Should stay last value
        _BTLast
    };
    /// Flatten BTree (the tree shape)
    typedef std::vector<BTShapeToken> BTShapeStream;

    /// Shapes storage structure
    typedef std::map<Ogre::String,BTShapeStream> BTShapeStreamMap;

    /// Allows for slow lookup
    extern std::vector<Ogre::String> BTShapeTokenAsString;
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

#endif
