/*
 * steeltypes.cpp
 *
 *  Created on: 2011-08-11
 *      Author: onze
 */

#include "steeltypes.h"

namespace Steel
{

    Ogre::String modelTypesAsString__data[] = { "MT_OGRE", "MT_BT" };

    std::vector<Ogre::String> modelTypesAsString = std::vector<Ogre::String> (
                modelTypesAsString__data,
                modelTypesAsString__data+sizeof(Ogre::String)/sizeof(Ogre::String)
            );

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 



