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
        "BTDebugPrinter",
        "BTUnknown"
    };

    std::vector<Ogre::String> BTShapeTokenTypeAsString = std::vector<Ogre::String> (
                BTShapeTokenTypeAsString__data,
                BTShapeTokenTypeAsString__data+sizeof(BTShapeTokenTypeAsString__data)/sizeof(Ogre::String)
    );
    
    Ogre::String BTStateAsString__data[] =
    {
        "READY",
        "RUNNING",
        "SUCCESS",
        "FAILURE",
        "SKIP_TO"
    };
    
    std::vector<Ogre::String> BTStateAsString = std::vector<Ogre::String> (
        BTStateAsString__data,
        BTStateAsString__data+sizeof(BTStateAsString__data)/sizeof(Ogre::String)
    );

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 



