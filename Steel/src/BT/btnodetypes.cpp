#include <vector>
#include <OgreString.h>

#include "BT/btnodetypes.h"

namespace Steel
{

    Ogre::String BTShapeTokenTypeAsString__data[] =
    {
        "BTSequence",
        "BTSelector",
//         "BTDecorator",
//         "BTCounter",
        "BTFinder",
        "BTNavigator",
        "BTSignalListener",
        "BTDebug",
        "BTUnknown"
    };

    std::vector<Ogre::String> BTShapeTokenTypeAsString = std::vector<Ogre::String> (
                BTShapeTokenTypeAsString__data,
                BTShapeTokenTypeAsString__data+sizeof(BTShapeTokenTypeAsString__data)/sizeof(Ogre::String)
    );
    
    Ogre::String BTNodeStateAsString__data[] =
    {
        "READY",
        "RUNNING",
        "SUCCESS",
        "FAILURE",
        "SKIP_TO"
    };
    
    std::vector<Ogre::String> BTNodeStateAsString = std::vector<Ogre::String> (
        BTNodeStateAsString__data,
        BTNodeStateAsString__data+sizeof(BTNodeStateAsString__data)/sizeof(Ogre::String)
    );

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 



