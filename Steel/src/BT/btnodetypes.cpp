#include <vector>
#include <OgreString.h>

#include "BT/btnodetypes.h"

namespace Steel
{

    Ogre::String BTShapeTokenAsString__data[] =
    {
        "BTSequence",
        "BTSelector",
        "BTDecorator",
        "BTCounter",
        "BTLocalizer",
        "BTNavigator",
        "BTUnknown",
    };

    std::vector<Ogre::String> BTShapeTokenAsString = std::vector<Ogre::String> (
                BTShapeTokenAsString__data,
                BTShapeTokenAsString__data+sizeof(BTShapeTokenAsString__data)/sizeof(Ogre::String)
            );

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 



