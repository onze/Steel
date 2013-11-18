
#include "steeltypes.h"

namespace Steel
{

    Ogre::String modelTypesAsString__data[] =
    {
        "MT_OGRE",
        "MT_PHYSICS",
        "MT_LOCATION",
        "MT_BLACKBOARD",
        "MT_BT",
    };

    std::vector<Ogre::String> modelTypesAsString = std::vector<Ogre::String> (
                modelTypesAsString__data,
                modelTypesAsString__data+sizeof(modelTypesAsString__data)/sizeof(Ogre::String)
            );

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 



